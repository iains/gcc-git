/* Definitions for C++ contract levels
   Copyright (C) 2020-2025 Free Software Foundation, Inc.
   C++20 edition by Jeff Chapman II (jchapman@lock3software.com).
   C++26 revisions by Nina Ranns (dinka.ranns@googlemail.com),
   Iain Sandoe (iain@sandoe.co.uk) and
   Ville Voutilainen (ville.voutilainen@gmail.com)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "cp-tree.h"
#include "stringpool.h"
#include "diagnostic.h"
#include "options.h"
#include "contracts.h"
#include "tree.h"
#include "tree-inline.h"
#include "attribs.h"
#include "tree-iterator.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "intl.h"
#include "cgraph.h"
#include "opts.h"
#include "output.h"

/* There are two implementations here, which share some support code
   and public APIs used by other parts of the front end.

   To facilitate maintenance, please keep the general ordering of content:

     shared local support functions
     cxx2a implementation:
       local support functions
       cxx2a-only public API
     cxx26 implementation:
       local support functions
       cxx26-only public API
     shared public API.  */

/* ===================== shared code ============================  */

static tree
build_comment (cp_expr condition)
{
  /* Try to get the actual source text for the condition; if that fails pretty
     print the resulting tree.  */
  char *str = get_source_text_between (global_dc->get_file_cache (),
				       condition.get_start (),
				       condition.get_finish ());
  if (!str)
    {
      /* FIXME cases where we end up here
	 #line macro usage (oof)
	 contracts10.C
	 contracts11.C  */
      const char *str = expr_to_string (condition);
      return build_string_literal (strlen (str) + 1, str);
    }

  tree t = build_string_literal (strlen (str) + 1, str);
  free (str);
  return t;
}

/* For use with the tree inliner. This preserves non-mapped local variables,
   such as postcondition result variables, during remapping.  */

static tree
retain_decl (tree decl, copy_body_data *)
{
  return decl;
}

/* Rewrite the condition of contract in place, so that references to SRC's
   parameters are updated to refer to DST's parameters. The postcondition
   result variable is left unchanged.

   When declarations are merged, we sometimes need to update contracts to
   refer to new parameters.

   If DUPLICATE_P is true, this is called by duplicate_decls to rewrite
   contracts in terms of a new set of parameters. In this case, we can retain
   local variables appearing in the contract because the contract is not being
   prepared for insertion into a new function. Importantly, this preserves the
   references to postcondition results, which are not replaced during merging.

   If false, we're preparing to emit the contract condition into the body
   of a new function, so we need to make copies of all local variables
   appearing in the contract (e.g., if it includes a lambda expression). Note
   that in this case, postcondition results are mapped to the last parameter
   of DST.

   This is also used to reuse a parent type's contracts on virtual methods.  */

static void
remap_contract (tree src, tree dst, tree contract, bool duplicate_p)
{
  copy_body_data id;
  hash_map<tree, tree> decl_map;

  memset (&id, 0, sizeof (id));
  id.src_fn = src;
  id.dst_fn = dst;
  id.src_cfun = DECL_STRUCT_FUNCTION (src);
  id.decl_map = &decl_map;

  /* If we're merging contracts, don't copy local variables.  */
  id.copy_decl = duplicate_p ? retain_decl : copy_decl_no_change;

  id.transform_call_graph_edges = CB_CGE_DUPLICATE;
  id.transform_new_cfg = false;
  id.transform_return_to_modify = false;
  id.transform_parameter = true;

  /* Make sure not to unshare trees behind the front-end's back
     since front-end specific mechanisms may rely on sharing.  */
  id.regimplify = false;
  id.do_not_unshare = true;
  id.do_not_fold = true;

  /* We're not inside any EH region.  */
  id.eh_lp_nr = 0;

  bool do_remap = false;

  /* Insert parameter remappings.  */
  gcc_assert(TREE_CODE (src) == FUNCTION_DECL);
  gcc_assert(TREE_CODE (dst) == FUNCTION_DECL);

  int src_num_artificial_args = num_artificial_parms_for (src);
  int dst_num_artificial_args = num_artificial_parms_for (dst);


  for (tree sp = DECL_ARGUMENTS (src), dp = DECL_ARGUMENTS (dst);
       sp || dp;
       sp = DECL_CHAIN (sp), dp = DECL_CHAIN (dp))
    {
      if (!sp && dp
	  && TREE_CODE (contract) == POSTCONDITION_STMT
	  && DECL_CHAIN (dp) == NULL_TREE)
	{
	  gcc_assert (!duplicate_p);
	  if (tree result = POSTCONDITION_IDENTIFIER (contract))
	    {
	      gcc_assert (DECL_P (result));
	      insert_decl_map (&id, result, dp);
	      do_remap = true;
	    }
	  break;
	}
      gcc_assert (sp && dp);

      if (sp == dp)
	continue;

      insert_decl_map (&id, sp, dp);
      do_remap = true;

      /* First artificial arg is *this. We want to remap that. However, we
	 want to skip _in_charge param and __vtt_parm. Do so now.  */
      if (src_num_artificial_args > 0)
	{
	  while (--src_num_artificial_args,src_num_artificial_args > 0)
	    sp = DECL_CHAIN (sp);
	}
      if (dst_num_artificial_args > 0)
	{
	  while (--dst_num_artificial_args,dst_num_artificial_args > 0)
	    dp = DECL_CHAIN (dp);
	}

    }

  if (!do_remap)
    return;

  walk_tree (&CONTRACT_CONDITION (contract), copy_tree_body_r, &id, NULL);
}

/* Helper to replace references to dummy this parameters with references to
   the first argument of the FUNCTION_DECL DATA.  */

static tree
remap_dummy_this_1 (tree *tp, int *, void *data)
{
  if (!is_this_parameter (*tp))
    return NULL_TREE;
  tree fn = (tree)data;
  *tp = DECL_ARGUMENTS (fn);
  return NULL_TREE;
}

/* Replace all references to dummy this parameters in EXPR with references to
   the first argument of the FUNCTION_DECL FNDECL.  */

static void
remap_dummy_this (tree fndecl, tree *expr)
{
  walk_tree (expr, remap_dummy_this_1, fndecl, NULL);
}

/* Replace uses of user's placeholder var with the actual return value.  */

struct replace_tree
{
  tree from, to;
};

static tree
remap_retval_1 (tree *here, int *do_subtree, void *d)
{
  replace_tree *data = (replace_tree *) d;

  if (*here == data->from)
    {
      *here = data->to;
      *do_subtree = 0;
    }
  else
    *do_subtree = 1;
  return NULL_TREE;
}

static void
remap_retval (tree fndecl, tree contract)
{
  struct replace_tree data;
  data.from = POSTCONDITION_IDENTIFIER (contract);
  gcc_checking_assert (DECL_RESULT (fndecl));
  data.to = DECL_RESULT (fndecl);
  walk_tree (&CONTRACT_CONDITION (contract), remap_retval_1, &data, NULL);
}

/* Contract matching.  */

/* True if the contract is valid.  */

static bool
contract_valid_p (tree contract)
{
  return CONTRACT_CONDITION (contract) != error_mark_node;
}

/* True if the contract attribute is valid.  */

static bool
contract_attribute_valid_p (tree attribute)
{
  return contract_valid_p (TREE_VALUE (TREE_VALUE (attribute)));
}

/* Compare the contract conditions of OLD_CONTRACT and NEW_CONTRACT.
   Returns false if the conditions are equivalent, and true otherwise.  */

static bool
mismatched_contracts_p (tree old_contract, tree new_contract,
			contract_matching_context ctx)
{
  /* Different kinds of contracts do not match.  */
  if (TREE_CODE (old_contract) != TREE_CODE (new_contract))
    {
      auto_diagnostic_group d;
      error_at (EXPR_LOCATION (new_contract),
		ctx == cmc_declaration
		? "mismatched contract attribute in declaration"
		: "mismatched contract attribute in override");
      inform (EXPR_LOCATION (old_contract), "previous contract here");
      return true;
    }

  /* A deferred contract tentatively matches.  */
  if (CONTRACT_CONDITION_DEFERRED_P (new_contract))
    return false;

  /* Compare the conditions of the contracts.  We fold immediately to avoid
     issues comparing contracts on overrides that use parameters -- see
     contracts-pre3.  */
  tree t1 = cp_fully_fold_init (CONTRACT_CONDITION (old_contract));
  tree t2 = cp_fully_fold_init (CONTRACT_CONDITION (new_contract));

  /* Compare the contracts. The fold doesn't eliminate conversions to members.
     Set the comparing_override_contracts flag to ensure that references
     through 'this' are equal if they designate the same member, regardless of
     the path those members.  */
  bool saved_comparing_contracts = comparing_override_contracts;
  comparing_override_contracts = (ctx == cmc_override);
  bool matching_p = cp_tree_equal (t1, t2);
  comparing_override_contracts = saved_comparing_contracts;

  if (!matching_p)
    {
      auto_diagnostic_group d;
      error_at (EXPR_LOCATION (CONTRACT_CONDITION (new_contract)),
		ctx == cmc_declaration
		? "mismatched contract condition in declaration"
		: "mismatched contract condition in override");
      inform (EXPR_LOCATION (CONTRACT_CONDITION (old_contract)),
	      "previous contract here");
      return true;
    }

  return false;
}

/* Compare the contract attributes of OLDDECL and NEWDECL. Returns true
   if the contracts match, and false if they differ.  */

static bool
match_contract_attributes (location_t oldloc, tree old_attrs,
			   location_t newloc, tree new_attrs,
			   contract_matching_context ctx)
{
  /* Contracts only match if they are both specified.  */
  if (!old_attrs || !new_attrs)
    return true;

  /* Compare each contract in turn.  */
  while (old_attrs && new_attrs)
    {
      /* If either contract is ill-formed, skip the rest of the comparison,
	 since we've already diagnosed an error.  */
      if (!contract_attribute_valid_p (new_attrs)
	  || !contract_attribute_valid_p (old_attrs))
	return false;

      if (mismatched_contracts_p (CONTRACT_STATEMENT (old_attrs),
				  CONTRACT_STATEMENT (new_attrs), ctx))
	return false;
      old_attrs = NEXT_CONTRACT_ATTR (old_attrs);
      new_attrs = NEXT_CONTRACT_ATTR (new_attrs);
    }

  /* If we didn't compare all attributes, the contracts don't match.  */
  if (old_attrs || new_attrs)
    {
      auto_diagnostic_group d;
      error_at (newloc,
		ctx == cmc_declaration
		? "declaration has a different number of contracts than "
		  "previously declared"
		: "override has a different number of contracts than "
		  "previously declared");
      inform (oldloc,
	      new_attrs
	      ? "previous declaration with fewer contracts here"
	      : "previous declaration with more contracts here");
      return false;
    }

  return true;
}

/* Map from FUNCTION_DECL to a FUNCTION_DECL for either the PRE_FN or POST_FN.
   These are used to parse contract conditions and are called inside the body
   of the guarded function.  */
static GTY(()) hash_map<tree, tree> *decl_pre_fn;
static GTY(()) hash_map<tree, tree> *decl_post_fn;


/* Given a pre or post function decl (for an outlined check function) return
   the decl for the function for which the outlined checks are being
   performed.  */
static GTY(()) hash_map<tree, tree> *orig_from_outlined;

/* Makes PRE the precondition function for FNDECL.  */

static void
set_precondition_function (tree fndecl, tree pre)
{
  gcc_assert (pre);
  hash_map_maybe_create<hm_ggc> (decl_pre_fn);
  gcc_assert (!decl_pre_fn->get (fndecl));
  decl_pre_fn->put (fndecl, pre);

  hash_map_maybe_create<hm_ggc> (orig_from_outlined);
  gcc_assert (!orig_from_outlined->get (pre));
  orig_from_outlined->put (pre, fndecl);
}

/* Makes POST the postcondition function for FNDECL.  */

static void
set_postcondition_function (tree fndecl, tree post)
{
  gcc_assert (post);
  hash_map_maybe_create<hm_ggc> (decl_post_fn);
  gcc_assert (!decl_post_fn->get (fndecl));
  decl_post_fn->put (fndecl, post);

  hash_map_maybe_create<hm_ggc> (orig_from_outlined);
  gcc_assert (!orig_from_outlined->get (post));
  orig_from_outlined->put (post, fndecl);
}

/* tree that holds the internal representation source location _impl */
static GTY(()) tree contracts_source_location_impl_type;

static tree
get_orig_for_outlined (tree fndecl)
{
  gcc_checking_assert (fndecl);
  tree *result = hash_map_safe_get (orig_from_outlined, fndecl);
  return result ? *result : NULL_TREE;
}

/* Build a declaration for the pre- or postcondition of a guarded FNDECL.  */

static tree
build_contract_condition_function (tree fndecl, bool pre)
{
  if (error_operand_p (fndecl))
    return error_mark_node;

  if (DECL_IOBJ_MEMBER_FUNCTION_P (fndecl)
      && !TYPE_METHOD_BASETYPE (TREE_TYPE (fndecl)))
    return error_mark_node;

  /* Start the copy.  */
  tree fn = copy_decl (fndecl);

  /* Don't propagate declaration attributes to the checking function,
     including the original contracts.  */
  DECL_ATTRIBUTES (fn) = NULL_TREE;

  /* DECL_ASSEMBLER_NAME will be copied from FNDECL if it's already set. Unset
     it so fn name later gets mangled according to the rules for pre and post
     functions.  */
  SET_DECL_ASSEMBLER_NAME (fn, NULL_TREE);

  /* If requested, disable optimisation of checking functions; this can, in
     some cases, prevent UB from eliding the checks themselves.  */
  if (flag_contract_disable_optimized_checks)
    DECL_ATTRIBUTES (fn)
      = tree_cons (get_identifier ("optimize"),
		   build_tree_list (NULL_TREE, build_string (3, "-O0")),
		   NULL_TREE);
  /* Now parse and add any internal representation of these attrs to the
     decl.  */
  if (DECL_ATTRIBUTES (fn))
    cplus_decl_attributes (&fn, DECL_ATTRIBUTES (fn), 0);

  /* FIXME will later optimizations delete unused args to prevent extra arg
     passing? do we care? */
  /* Handle the args list.  */
  tree arg_types = NULL_TREE;
  tree *last = &arg_types;
  tree class_type = NULL_TREE;
  for (tree arg_type = TYPE_ARG_TYPES (TREE_TYPE (fn));
      arg_type && arg_type != void_list_node;
      arg_type = TREE_CHAIN (arg_type))
    {
      if (DECL_IOBJ_MEMBER_FUNCTION_P (fndecl)
	  && TYPE_ARG_TYPES (TREE_TYPE (fn)) == arg_type)
      {
	class_type = TREE_TYPE (TREE_VALUE (arg_type));
	continue;
      }
      *last = build_tree_list (TREE_PURPOSE (arg_type), TREE_VALUE (arg_type));
      last = &TREE_CHAIN (*last);
    }

  /* Copy the function parameters, if present.  Disable warnings for them.  */
  DECL_ARGUMENTS (fn) = NULL_TREE;
  if (DECL_ARGUMENTS (fndecl))
    {
      tree *last_a = &DECL_ARGUMENTS (fn);
      for (tree p = DECL_ARGUMENTS (fndecl); p; p = TREE_CHAIN (p))
	{
	  *last_a = copy_decl (p);
	  suppress_warning (*last_a);
	  DECL_CONTEXT (*last_a) = fn;
	  last_a = &TREE_CHAIN (*last_a);
	}
    }

  tree orig_fn_value_type = TREE_TYPE (TREE_TYPE (fn));
  if (!pre && !VOID_TYPE_P (orig_fn_value_type))
    {
      /* For post contracts that deal with a non-void function, append a
	 parameter to pass the return value.  */
      tree name = get_identifier ("__r");
      tree parm = build_lang_decl (PARM_DECL, name, orig_fn_value_type);
      DECL_CONTEXT (parm) = fn;
      DECL_ARTIFICIAL (parm) = true;
      suppress_warning (parm);
      DECL_ARGUMENTS (fn) = chainon (DECL_ARGUMENTS (fn), parm);
      *last = build_tree_list (NULL_TREE, orig_fn_value_type);
      last = &TREE_CHAIN (*last);
    }

  *last = void_list_node;

  /* The handlers are void fns.  */
  tree adjusted_type = build_function_type (void_type_node, arg_types);

  /* If the original function is noexcept, build a noexcept function. */
  if (flag_exceptions && type_noexcept_p (TREE_TYPE (fndecl)))
    adjusted_type = build_exception_variant (adjusted_type, noexcept_true_spec);

  TREE_TYPE (fn) = adjusted_type;
  DECL_RESULT (fn) = NULL_TREE; /* Let the start function code fill it in.  */

  if (DECL_IOBJ_MEMBER_FUNCTION_P (fndecl))
    TREE_TYPE (fn) = build_method_type (class_type, TREE_TYPE (fn));

  if (DECL_CONSTRUCTOR_P (fndecl) || DECL_DESTRUCTOR_P (fndecl))
    {
      /* If we're dealing with a cdtor, we need to give it a "normal"
	 DECL_NAME so it doesn't trigger IDENTIFIER_CDTOR_P checks */
      DECL_NAME (fn) = DECL_ASSEMBLER_NAME(fndecl);
      DECL_CXX_DESTRUCTOR_P (fn) = DECL_CXX_CONSTRUCTOR_P (fn) = 0;
    }
  else
    DECL_NAME (fn) = copy_node (DECL_NAME (fn));
  DECL_INITIAL (fn) = NULL_TREE;
  CONTRACT_HELPER (fn) = pre ? ldf_contract_pre : ldf_contract_post;

  IDENTIFIER_VIRTUAL_P (DECL_NAME (fn)) = false;
  DECL_VIRTUAL_P (fn) = false;

  /* Make these functions internal if we can, i.e. if the guarded function is
     not vague linkage, or if we can put them in a comdat group with the
     guarded function.  */
  if (!DECL_WEAK (fndecl) || HAVE_COMDAT_GROUP)
    {
      TREE_PUBLIC (fn) = false;
      DECL_EXTERNAL (fn) = false;
      DECL_WEAK (fn) = false;
      DECL_COMDAT (fn) = false;
      DECL_INTERFACE_KNOWN (fn) = true;
    }

  DECL_ARTIFICIAL (fn) = true;

  /* Update various inline related declaration properties.  */
  //DECL_DECLARED_INLINE_P (fn) = true;
  DECL_DISREGARD_INLINE_LIMITS (fn) = true;
  suppress_warning (fn);

  return fn;
}

/* Return true if CONTRACT is checked or assumed under the current build
   configuration. */

static bool
contract_active_p (tree contract)
{
  return get_contract_semantic (contract) != CCS_IGNORE;
}

/* True if FNDECL has any checked or assumed contracts whose TREE_CODE is
   C.  */

static bool
has_active_contract_condition (tree fndecl, tree_code c)
{
  tree as = flag_contracts_nonattr
	    ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
	    : DECL_CONTRACT_ATTRS (fndecl);
  for (; as != NULL_TREE; as = NEXT_CONTRACT_ATTR (as))
    {
      tree contract = TREE_VALUE (TREE_VALUE (as));
      if (TREE_CODE (contract) == c && contract_active_p (contract))
	return true;
    }
  return false;
}

/* True if FNDECL has any checked or assumed preconditions.  */

static bool
has_active_preconditions (tree fndecl)
{
  return has_active_contract_condition (fndecl, PRECONDITION_STMT);
}

/* True if FNDECL has any checked or assumed postconditions.  */

static bool
has_active_postconditions (tree fndecl)
{
  return has_active_contract_condition (fndecl, POSTCONDITION_STMT);
}

/* Return true if any contract in the CONTRACT list is checked or assumed
   under the current build configuration.  */

static bool
contract_any_active_p (tree fndecl)
{
  tree as = flag_contracts_nonattr
	    ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
	    : DECL_CONTRACT_ATTRS (fndecl);
  for (; as; as = NEXT_CONTRACT_ATTR (as))
    if (contract_active_p (TREE_VALUE (TREE_VALUE (as))))
      return true;
  return false;
}

/* Return true if any contract in CONTRACT_ATTRs is not yet parsed.  */

static bool
contract_any_deferred_p (tree contract_attr)
{
  for (; contract_attr; contract_attr = NEXT_CONTRACT_ATTR (contract_attr))
    if (CONTRACT_CONDITION_DEFERRED_P (CONTRACT_STATEMENT (contract_attr)))
      return true;
  return false;
}

/* Copy deferred contract attributes from SRC onto DEST.  This
 assumes that if one contract is deferred, all contracts are deferred.  */

static void
copy_deferred_contracts (tree srcdecl, tree destdecl)
{
  tree contracts = flag_contracts_nonattr
		    ? GET_FN_CONTRACT_SPECIFIERS (srcdecl)
		    : DECL_CONTRACT_ATTRS (srcdecl);

  if (!contracts)
    return;

  gcc_checking_assert(contract_any_deferred_p (contracts));

  tree attrs = NULL_TREE;
  for (tree c = contracts; c; c = TREE_CHAIN (c))
    {
      if (!cxx_contract_attribute_p (c))
	continue;
      attrs = tree_cons (TREE_PURPOSE (c), TREE_VALUE (c), attrs);
    }
  if (flag_contracts_nonattr)
    set_fn_contract_specifiers (destdecl, nreverse (attrs));
  else
    set_contract_attributes (destdecl, nreverse (attrs));
}


/* Returns true if function decl FNDECL has contracts and we need to
   process them for the purposes of either building caller or definition
   contract checks.
   This function does not take into account whether caller or definition
   side checking is enabled. Those checks will be done from the calling
   function which will be able to determine whether it is doing caller
   or definition contract handling.  */

static bool
handle_contracts_p (tree fndecl)
{
  return (flag_contracts
	  && !processing_template_decl
	  && (CONTRACT_HELPER (fndecl) == ldf_contract_none)
	  && contract_any_active_p (fndecl));
}

/* Returns the parameter corresponding to the return value of a guarded
   function FNDECL.  Returns NULL_TREE if FNDECL has no postconditions or
   is void.  */

static tree
get_postcondition_result_parameter (tree fndecl)
{
  if (!fndecl || fndecl == error_mark_node)
    return NULL_TREE;

  if (VOID_TYPE_P (TREE_TYPE (TREE_TYPE (fndecl))))
    return NULL_TREE;

  tree post = DECL_POST_FN (fndecl);
  if (!post || post == error_mark_node)
    return NULL_TREE;

  for (tree arg = DECL_ARGUMENTS (post); arg; arg = TREE_CHAIN (arg))
    if (!TREE_CHAIN (arg))
      return arg;

  return NULL_TREE;
}

/* Build the precondition checking function for FNDECL.  */

static tree
build_precondition_function (tree fndecl)
{
  if (!has_active_preconditions (fndecl))
    return NULL_TREE;

  return build_contract_condition_function (fndecl, /*pre=*/true);
}

/* Build the postcondition checking function for FNDECL. If the return
   type is undeduced, don't build the function yet. We do that in
   apply_deduced_return_type.  */

static tree
build_postcondition_function (tree fndecl)
{
  if (!has_active_postconditions (fndecl))
    return NULL_TREE;

  tree type = TREE_TYPE (TREE_TYPE (fndecl));
  if (is_auto (type))
    return NULL_TREE;

  return build_contract_condition_function (fndecl, /*pre=*/false);
}

/* If we're outlining the contract, build the functions to do the
   precondition and postcondition checks, and associate them with
   the function decl FNDECL.
 */

static void
build_contract_function_decls (tree fndecl)
{
  /* Build the pre/post functions (or not).  */
  if (!get_precondition_function (fndecl))
    if (tree pre = build_precondition_function (fndecl))
      set_precondition_function (fndecl, pre);

  if (!get_postcondition_function (fndecl))
    if (tree post = build_postcondition_function (fndecl))
      set_postcondition_function (fndecl, post);
}


/* Build a named TU-local constant of TYPE.  */

static tree
contracts_tu_local_named_var (location_t loc, const char *name, tree type,
			      bool is_const)
{
  tree var_ = build_decl (loc, VAR_DECL, NULL, type);
  /* Generate name.uid to ensure unique entries.  */
  char tmp_name[32];
  ASM_GENERATE_INTERNAL_LABEL (tmp_name, name, DECL_UID (var_));
  DECL_NAME (var_) = get_identifier (tmp_name);
  /* TU-local and defined.  */
  TREE_PUBLIC (var_) = false;
  DECL_EXTERNAL (var_) = false;
  TREE_STATIC (var_) = true;
  /* compiler-generated, and constant.  */
  DECL_ARTIFICIAL (var_) = true;
  DECL_IGNORED_P (var_) = true;
  TREE_CONSTANT (var_) = is_const;
  layout_decl (var_, 0);
  return var_;
}

/* Lookup a name in std::contracts/experimental, or inject it.  */
static tree
lookup_std_contracts_type (tree name_id)
{
  tree id_exp = NULL_TREE;
  if (flag_contracts_nonattr)
      id_exp = get_identifier ("contracts");
  else
      id_exp = get_identifier ("experimental");

  tree ns_exp = lookup_qualified_name (std_node, id_exp);

  tree res_type = error_mark_node;
  if (TREE_CODE (ns_exp) == NAMESPACE_DECL)
    res_type = lookup_qualified_name (ns_exp, name_id,
				       LOOK_want::TYPE
				       |LOOK_want::HIDDEN_FRIEND);

  if (TREE_CODE (res_type) == TYPE_DECL)
    res_type = TREE_TYPE (res_type);
  else
    {
      push_nested_namespace (std_node);
      push_namespace (id_exp, /*inline*/false);
      res_type = make_class_type (RECORD_TYPE);
      create_implicit_typedef (name_id, res_type);
      DECL_SOURCE_LOCATION (TYPE_NAME (res_type)) = BUILTINS_LOCATION;
      DECL_CONTEXT (TYPE_NAME (res_type)) = current_namespace;
      pushdecl_namespace_level (TYPE_NAME (res_type), /*hidden*/true);
      pop_namespace ();
      pop_nested_namespace (std_node);
    }
  return res_type;
}

/* Return handle_contract_violation (), declaring it if needed.  */

static tree
declare_handle_contract_violation ()
{
  /* We may need to declare new types, ensure they are not considered
     attached to a named module.  */
  auto module_kind_override = make_temp_override
    (module_kind, module_kind & ~(MK_PURVIEW | MK_ATTACH | MK_EXPORTING));
  tree fnname = get_identifier ("handle_contract_violation");
  tree viol_name = get_identifier ("contract_violation");
  tree l = lookup_qualified_name (global_namespace, fnname,
				  LOOK_want::HIDDEN_FRIEND);
  for (tree f: lkp_range (l))
    if (TREE_CODE (f) == FUNCTION_DECL)
	{
	  tree parms = TYPE_ARG_TYPES (TREE_TYPE (f));
	  if (remaining_arguments (parms) != 1)
	    continue;
	  tree parmtype = non_reference (TREE_VALUE (parms));
	  if (CLASS_TYPE_P (parmtype)
	      && TYPE_IDENTIFIER (parmtype) == viol_name)
	    return f;
	}

  tree violation = lookup_std_contracts_type (viol_name);
  tree fntype = NULL_TREE;
  tree v_obj_ref = cp_build_qualified_type (violation, TYPE_QUAL_CONST);
  v_obj_ref = cp_build_reference_type (v_obj_ref, /*rval*/false);
  fntype = build_function_type_list (void_type_node, v_obj_ref, NULL_TREE);

  push_nested_namespace (global_namespace);
  tree fndecl
    = build_cp_library_fn_ptr ("handle_contract_violation", fntype, ECF_COLD);
  pushdecl_namespace_level (fndecl, /*hiding*/true);
  pop_nested_namespace (global_namespace);

  /* Build the parameter(s).  */
  tree parms = cp_build_parm_decl (fndecl, NULL_TREE, v_obj_ref);
  TREE_USED (parms) = true;
  DECL_READ_P (parms) = true;
  DECL_ARGUMENTS (fndecl) = parms;
  return fndecl;
}

/* Build the call to handle_contract_violation for VIOLATION.  */

static void
build_contract_handler_call (tree violation)
{
  /* We may need to declare new types, ensure they are not considered
     attached to a named module.  */
  auto module_kind_override = make_temp_override
    (module_kind, module_kind & ~(MK_PURVIEW | MK_ATTACH | MK_EXPORTING));

  tree violation_fn = declare_handle_contract_violation ();
  tree call = build_call_n (violation_fn, 1, violation);
  finish_expr_stmt (call);
}

static tree build_contract_violation_cxx2a (tree);
static tree build_contract_violation_p2900 (tree, bool);

/* Return a VAR_DECL to pass to handle_contract_violation.  */

static tree
build_contract_violation (tree contract, bool is_const)
{
  if (flag_contracts_nonattr)
    return build_contract_violation_p2900 (contract, is_const);

  return build_contract_violation_cxx2a (contract);
}

/* Add the contract statement CONTRACT to the current block if valid.  */

static bool
emit_contract_statement (tree contract)
{
  /* Only add valid contracts.  */
  if (contract == error_mark_node
      || CONTRACT_CONDITION (contract) == error_mark_node
      || get_contract_semantic (contract) == CCS_INVALID)
    return false;

  add_stmt (contract);
  return true;
}

/* Generate the statement for the given contract attribute by adding the
   statement to the current block. Returns the next contract in the chain.  */

static void
emit_contract_attr (tree attr)
{
  gcc_checking_assert (TREE_CODE (attr) == TREE_LIST);

  emit_contract_statement (CONTRACT_STATEMENT (attr));
}

/* We're compiling the pre/postcondition function CONDFN; remap any FN
   attributes that match CODE and emit them.  */

static void
remap_and_emit_conditions (tree fn, tree condfn, tree_code code)
{
  gcc_assert (code == PRECONDITION_STMT || code == POSTCONDITION_STMT);
  tree attr = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (fn)
	      : DECL_CONTRACT_ATTRS (fn);
  for (; attr; attr = NEXT_CONTRACT_ATTR (attr))
    {
      tree contract = CONTRACT_STATEMENT (attr);
      if (TREE_CODE (contract) == code)
	{
	  contract = copy_node (contract);
	  remap_contract (fn, condfn, contract, /*duplicate_p=*/false);
	  if (!emit_contract_statement (contract))
	    continue;
	}
    }
}

/* Build and return a thunk like call to FUNCTION using the supplied
 arguments.  The call is like a thunk call in the fact that we do not
 want to create additional copies of the arguments. However, we can
 not simply reuse the thunk machinery as it does more than we want.
 More specifically, we don't want to mark the calling function as
 `DECL_THUNK_P`, we only want the special treatment for the parameters
 of the call we are about to generate.
 We reuse most of build_call_a, modulo special handling for empty
 classes which relies on `DECL_THUNK_P` to know that the call we're building
 is going to be a thunk call.
 We also mark the call as a thunk call to allow for correct gimplification
 of the arguments.
 */

static tree
build_thunk_like_call (tree function, int n, tree *argarray)
{

  function = build_call_a_1 (function, n, argarray);

  tree decl = get_callee_fndecl (function);

  /* Set TREE_USED for the benefit of -Wunused.  */
  if (decl && !TREE_USED (decl))
    TREE_USED (decl) = true;

  CALL_FROM_THUNK_P (function) = true;

  return function;
}

/* Build and return an argument list containing all the parameters of the
   (presumably guarded) function decl FNDECL.  This can be used to forward
   all of FNDECL arguments to a function taking the same list of arguments
   -- namely the unchecked form of FNDECL.

   We use CALL_FROM_THUNK_P instead of forward_parm for forwarding
   semantics.  */

static vec<tree, va_gc> *
build_arg_list (tree fndecl)
{
  vec<tree, va_gc> *args = make_tree_vector ();
  for (tree t = DECL_ARGUMENTS (fndecl); t; t = DECL_CHAIN (t))
    vec_safe_push (args, t);
  return args;
}

static tree
copy_contracts_list (tree attr, tree fndecl,
		     contract_match_kind remap_kind = cmk_all)
{
  tree last = NULL_TREE, contract_attrs = NULL_TREE;
  for (; attr; attr = NEXT_CONTRACT_ATTR (attr))
    {
      if ((remap_kind == cmk_pre
	   && (TREE_CODE (CONTRACT_STATEMENT (attr)) == POSTCONDITION_STMT))
	  || (remap_kind == cmk_post
	      && (TREE_CODE (CONTRACT_STATEMENT (attr)) == PRECONDITION_STMT)))
	continue;

      tree c = copy_node (attr);
      TREE_VALUE (c) = build_tree_list (TREE_PURPOSE (TREE_VALUE (c)),
					copy_node (CONTRACT_STATEMENT (c)));

      copy_body_data id;
      hash_map<tree, tree> decl_map;

      memset (&id, 0, sizeof (id));

      id.src_fn = fndecl;
      id.dst_fn = fndecl;
      id.src_cfun = DECL_STRUCT_FUNCTION (fndecl);
      id.decl_map = &decl_map;

      id.copy_decl = retain_decl;

      id.transform_call_graph_edges = CB_CGE_DUPLICATE;
      id.transform_new_cfg = false;
      id.transform_return_to_modify = false;
      id.transform_parameter = true;

      /* Make sure not to unshare trees behind the front-end's back
	 since front-end specific mechanisms may rely on sharing.  */
      id.regimplify = false;
      id.do_not_unshare = true;
      id.do_not_fold = true;

      /* We're not inside any EH region.  */
      id.eh_lp_nr = 0;
      walk_tree (&CONTRACT_CONDITION (CONTRACT_STATEMENT (c)),
				      copy_tree_body_r, &id, NULL);


      CONTRACT_COMMENT (CONTRACT_STATEMENT (c))
	= copy_node (CONTRACT_COMMENT (CONTRACT_STATEMENT (c)));

      chainon (last, c);
      last = c;
      if (!contract_attrs)
	contract_attrs = c;
    }
  return contract_attrs;
}

/* Returns a copy of FNDECL contracts. This is used when emiting a contract.
 If we were to emit the original contract tree, any folding of the contract
 condition would affect the original contract too. The original contract
 tree needs to be preserved in case it is used to apply to a different
 function (for inheritance or wrapping reasons). */

static tree
copy_contracts (tree fndecl, contract_match_kind remap_kind = cmk_all)
{
  tree attr = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
	      : DECL_CONTRACT_ATTRS (fndecl);
  return copy_contracts_list (attr, fndecl, remap_kind);
}


/* Add a call or a direct evaluation of the pre checks.  */

static void
apply_preconditions (tree fndecl)
{
  /* If we're starting a guarded function with valid contracts, we need to
   insert a call to the pre function.  */
  gcc_checking_assert(
      DECL_PRE_FN (fndecl) && DECL_PRE_FN (fndecl) != error_mark_node);

  releasing_vec args = build_arg_list (fndecl);
  tree call = build_thunk_like_call (DECL_PRE_FN(fndecl), args->length (),
				     args->address ());

  finish_expr_stmt (call);
}

/* Add a call or a direct evaluation of the post checks.  */

static void
apply_postconditions (tree fndecl)
{
  gcc_checking_assert(
      DECL_POST_FN (fndecl) && DECL_POST_FN (fndecl) != error_mark_node);

  releasing_vec args = build_arg_list (fndecl);
  if (get_postcondition_result_parameter (fndecl))
    vec_safe_push (args, DECL_RESULT(fndecl));
  tree call = build_thunk_like_call (DECL_POST_FN(fndecl), args->length (),
				     args->address ());
  finish_expr_stmt (call);
}


/* =========================== C++2a contracts ============================  */

/* Design Notes

   A function is called a "guarded" function if it has pre or post contract
   attributes. A contract is considered an "active" contract if runtime code is
   needed for the contract under the current contract configuration.

   pre and post contract attributes are parsed and stored in DECL_ATTRIBUTES.
   assert contracts are parsed and wrapped in statements. When genericizing, all
   active and assumed contracts are transformed into an if block. An observed
   contract:

     [[ pre: v > 0 ]]

   is transformed into:

     if (!(v > 0)) {
       handle_contract_violation(__pseudo_contract_violation{
	 5, // line_number,
	 "main.cpp", // file_name,
	 "fun", // function_name,
	 "v > 0", // comment,
	 "default", // assertion_level,
	 "default", // assertion_role,
	 maybe_continue, // continuation_mode
       });
       terminate (); // if never_continue
     }

   We use an internal type with the same layout as contract_violation rather
   than try to define the latter internally and somehow deal with its actual
   definition in a TU that includes <contract>.

   ??? is it worth factoring out the calls to handle_contract_violation and
   terminate into a local function?

   Assumed contracts use the same implementation as C++23 [[assume]].

   Parsing of pre and post contract conditions need to be deferred when the
   contracts are attached to a member function. The postcondition identifier
   cannot be used before the deduced return type of an auto function is used,
   except when used in a defining declaration in which case they conditions are
   fully parsed once the body is finished (see cpp2a/contracts-deduced{1,2}.C).

   A list of pre and post contracts can either be repeated in their entirety or
   completely absent in subsequent declarations. If contract lists appear on two
   matching declarations, their contracts have to be equivalent. In general this
   means that anything before the colon have to be token equivalent and the
   condition must be cp_tree_equal (primarily to allow for parameter renaming).

   Contracts on overrides must match those present on (all of) the overridee(s).

   Template specializations may have their own contracts. If no contracts are
   specified on the initial specialization they're assumed to be the same as
   the primary template. Specialization redeclarations must then match either
   the primary template (if they were unspecified originally), or those
   specified on the specialization.


   For non-cdtors two functions are generated for ease of implementation and to
   avoid some cases where code bloat may occurr. These are the DECL_PRE_FN and
   DECL_POST_FN. Each handles checking either the set of pre or post contracts
   of a guarded function.

     int fun(int v)
       [[ pre: v > 0 ]]
       [[ post r: r < 0 ]]
     {
       return -v;
     }

   We implement the checking as follows:

   For functions with no post-conditions we wrap the original function body as
   follows:

   {
      handle_pre_condition_checking ();
      original_function_body ();
   }

   This implements the intent that the preconditions are processed after the
   function parameters are initialised but before any other actions.

   For functions with post-conditions:

   if (preconditions_exist)
     handle_pre_condition_checking ();
   try
     {
       original_function_body ();
     }
   finally
     {
       handle_post_condition_checking ();
     }
   else [only if the function is not marked noexcept(true) ]
     {
       ;
     }

   In this, post-conditions [that might apply to the return value etc.] are
   evaluated on every non-exceptional edge out of the function.

   FIXME outlining contract checks into separate functions was motivated
   partly by wanting to call the postcondition function at each return
   statement, which we no longer do; at this point outlining doesn't seem to
   have any advantage over emitting the contracts directly in the function
   body.

   More helpful for optimization might be to make the contracts a wrapper
   function that could be inlined into the caller, the callee, or both.  */

const int max_custom_roles = 32;
static contract_role contract_build_roles[max_custom_roles] = {
};

bool valid_configs[CCS_MAYBE + 1][CCS_MAYBE + 1] = {
  { 0, 0, 0, 0, 0, },
  { 0, 1, 0, 0, 0, },
  { 0, 1, 1, 1, 1, },
  { 0, 1, 1, 1, 1, },
  { 0, 1, 0, 0, 1, },
};

static void
validate_contract_role (contract_role *role)
{
  gcc_assert (role);
  if (!unchecked_contract_p (role->axiom_semantic))
    error ("axiom contract semantic must be %<assume%> or %<ignore%>");

  if (!valid_configs[role->default_semantic][role->audit_semantic] )
    warning (0, "the %<audit%> semantic should be at least as strong as "
		"the %<default%> semantic");
}

static contract_semantic
lookup_concrete_semantic (const char *name)
{
  if (strcmp (name, "ignore") == 0)
    return CCS_IGNORE;
  if (strcmp (name, "assume") == 0)
    return flag_contracts_nonattr ? CCS_IGNORE : CCS_ASSUME;
  if (strcmp (name, "check_never_continue") == 0
      || strcmp (name, "never") == 0
      || strcmp (name, "abort") == 0)
    return CCS_NEVER;
  if (strcmp (name, "check_maybe_continue") == 0
      || strcmp (name, "maybe") == 0)
    return CCS_MAYBE;
  error ("'%s' is not a valid explicit concrete semantic", name);
  return CCS_INVALID;
}

/* Compare role and name up to either the NUL terminator or the first
   occurrence of colon.  */

static bool
role_name_equal (const char *role, const char *name)
{
  size_t role_len = strcspn (role, ":");
  size_t name_len = strcspn (name, ":");
  if (role_len != name_len)
    return false;
  return strncmp (role, name, role_len) == 0;
}

static bool
role_name_equal (contract_role *role, const char *name)
{
  if (role->name == NULL)
    return false;
  return role_name_equal (role->name, name);
}

static contract_role *
add_contract_role (const char *name,
		   contract_semantic des,
		   contract_semantic aus,
		   contract_semantic axs,
		   bool update = true)
{
  for (int i = 0; i < max_custom_roles; ++i)
    {
      contract_role *potential = contract_build_roles + i;
      if (potential->name != NULL
	  && !role_name_equal (potential, name))
	continue;
      if (potential->name != NULL && !update)
	return potential;
      potential->name = name;
      potential->default_semantic = des;
      potential->audit_semantic = aus;
      potential->axiom_semantic = axs;
      return potential;
    }
  return NULL;
}

enum contract_build_level { OFF, DEFAULT, AUDIT };
static bool flag_contract_continuation_mode = false;
static bool flag_contract_assumption_mode = true;
static int flag_contract_build_level = DEFAULT;

static bool contracts_p1332_default = false, contracts_p1332_review = false,
  contracts_std = false, contracts_p1429 = false;

static contract_semantic
get_concrete_check ()
{
  return flag_contract_continuation_mode ? CCS_MAYBE : CCS_NEVER;
}

static contract_semantic
get_concrete_axiom_semantic ()
{
  return flag_contract_assumption_mode ? CCS_ASSUME : CCS_IGNORE;
}

static void
setup_default_contract_role (bool update = true)
{
  contract_semantic check = get_concrete_check ();
  contract_semantic axiom = get_concrete_axiom_semantic ();
  switch (flag_contract_build_level)
    {
      case OFF:
	add_contract_role ("default", CCS_IGNORE, CCS_IGNORE, axiom, update);
	add_contract_role ("review", CCS_IGNORE, CCS_IGNORE, CCS_IGNORE,
			   update);
	break;
      case DEFAULT:
	add_contract_role ("default", check, CCS_IGNORE, axiom, update);
	add_contract_role ("review", check, CCS_IGNORE, CCS_IGNORE, update);
	break;
      case AUDIT:
	add_contract_role ("default", check, check, axiom, update);
	add_contract_role ("review", check, check, CCS_IGNORE, update);
	break;
    }
}

static contract_role *
get_contract_role (const char *name)
{
  for (int i = 0; i < max_custom_roles; ++i)
    {
      contract_role *potential = contract_build_roles + i;
      if (role_name_equal (potential, name))
	return potential;
    }
  if (role_name_equal (name, "default") || role_name_equal (name, "review"))
    {
      setup_default_contract_role (false);
      return get_contract_role (name);
    }
  return NULL;
}

/* Returns the default role.  */

static contract_role *
get_default_contract_role ()
{
  return get_contract_role ("default");
}

/* Convert a contract CONFIG into a contract_mode.  */

static contract_mode
contract_config_to_mode (tree config)
{
  if (config == NULL_TREE)
    return contract_mode (CONTRACT_DEFAULT, get_default_contract_role ());

  /* TREE_LIST has TREE_VALUE is a level and TREE_PURPOSE is role.  */
  if (TREE_CODE (config) == TREE_LIST)
    {
      contract_role *role = NULL;
      if (TREE_PURPOSE (config))
	role = get_contract_role (IDENTIFIER_POINTER (TREE_PURPOSE (config)));
      if (!role)
	role = get_default_contract_role ();

      contract_level level
	= map_contract_level (IDENTIFIER_POINTER (TREE_VALUE (config)));
      return contract_mode (level, role);
    }

  /* Literal semantic.  */
  gcc_assert (TREE_CODE (config) == IDENTIFIER_NODE);
  contract_semantic semantic
    = map_contract_semantic (IDENTIFIER_POINTER (config));
  return contract_mode (semantic);
}

/* Convert a contract's config into a concrete semantic using the current
   contract semantic mapping.  */

static contract_semantic
compute_concrete_semantic (tree contract)
{
  contract_mode mode = contract_config_to_mode (CONTRACT_MODE (contract));
  /* Compute the concrete semantic for the contract.  */
  if (!flag_contract_mode)
    /* If contracts are off, treat all contracts as ignore.  */
    return CCS_IGNORE;
  else if (mode.kind == contract_mode::cm_invalid)
    return CCS_INVALID;
  else if (mode.kind == contract_mode::cm_explicit)
    return mode.get_semantic ();
  else
    {
      gcc_assert (mode.get_role ());
      gcc_assert (mode.get_level () != CONTRACT_INVALID);
      contract_level level = mode.get_level ();
      contract_role *role = mode.get_role ();
      if (level == CONTRACT_DEFAULT)
	return role->default_semantic;
      else if (level == CONTRACT_AUDIT)
	return role->audit_semantic;
      else if (level == CONTRACT_AXIOM)
	return role->axiom_semantic;
    }
  gcc_assert (false);
}

/* Get name of contract_level of the specified contract. Used when building
 C++20 contract_violation object.  */

static const char *
get_contract_level_name (tree contract)
{
  if (CONTRACT_LITERAL_MODE_P (contract))
    return "";
  if (tree mode = CONTRACT_MODE (contract))
    if (tree level = TREE_VALUE (mode))
      return IDENTIFIER_POINTER (level);
  return "default";
}

/* Get name of contract_role of the specified contract. Used when building
 C++20 contract_violation object.  */

static const char *
get_contract_role_name (tree contract)
{
  if (CONTRACT_LITERAL_MODE_P (contract))
    return "";
  if (tree mode = CONTRACT_MODE (contract))
    if (tree role = TREE_PURPOSE (mode))
      return IDENTIFIER_POINTER (role);
  return "default";
}


/* Define the layout of the c++2a contract_violation object.  */

static tree
get_cxx2a_contract_violation_fields ()
{
  tree fields = NULL_TREE;
  /* Must match <contract>:
    class contract_violation {
      const char* _M_file;
      const char* _M_function;
      const char* _M_comment;
      const char* _M_level;
      const char* _M_role;
      uint_least32_t _M_line;
      signed char _M_continue;
      If this changes, also update the initializer in
      build_contract_violation.  */
  const tree types[] = { const_string_type_node,
			 const_string_type_node,
			 const_string_type_node,
			 const_string_type_node,
			 const_string_type_node,
			 uint_least32_type_node,
			 signed_char_type_node };

  for (tree type : types)
    {
      /* finish_builtin_struct wants fieldss chained in reverse.  */
      tree next = build_decl (BUILTINS_LOCATION, FIELD_DECL,
				    NULL_TREE, type);
      DECL_CHAIN (next) = fields;
      fields = next;
    }
  return fields;
}

/* Build C++20 contract_violation layout compatible object.  */

static tree
build_contract_violation_cxx2a (tree contract)
{
  expanded_location loc = expand_location (EXPR_LOCATION (contract));
  const char *function = fndecl_name (DECL_ORIGIN (current_function_decl));
  const char *level = get_contract_level_name (contract);
  const char *role = get_contract_role_name (contract);

  /* Get the continuation mode.  */
  contract_continuation cmode;
  switch (get_contract_semantic (contract))
    {
    case CCS_NEVER: cmode = NEVER_CONTINUE; break;
    case CCS_MAYBE: cmode = MAYBE_CONTINUE; break;
    default: gcc_unreachable ();
    }

  /* Must match the type layout in builtin_contract_violation_type.  */
  tree ctor = build_constructor_va
    (init_list_type_node, 7,
     NULL_TREE, build_string_literal (loc.file),
     NULL_TREE, build_string_literal (function),
     NULL_TREE, CONTRACT_COMMENT (contract),
     NULL_TREE, build_string_literal (level),
     NULL_TREE, build_string_literal (role),
     NULL_TREE, build_int_cst (uint_least32_type_node, loc.line),
     NULL_TREE, build_int_cst (signed_char_type_node, cmode));

  ctor = finish_compound_literal (builtin_contract_violation_type,
				  ctor, tf_none, fcl_c99);
  protected_set_expr_location (ctor, EXPR_LOCATION (contract));
  return ctor;
}

/* Expand a cxx2a CONTRACT tree.  This is called during genericization.  */

static tree
build_contract_check_cxx2a (tree contract)
{
  contract_semantic semantic = get_contract_semantic (contract);
  if (semantic == CCS_INVALID)
    return NULL_TREE;

  /* Ignored contracts are never checked or assumed.  */
  if (semantic == CCS_IGNORE)
    return void_node;

  remap_dummy_this (current_function_decl, &CONTRACT_CONDITION (contract));
  tree condition = CONTRACT_CONDITION (contract);
  if (condition == error_mark_node)
    return NULL_TREE;

  location_t loc = EXPR_LOCATION (contract);

  if (semantic == CCS_ASSUME)
    return build_assume_call (loc, condition);

  tree if_stmt = begin_if_stmt ();
  tree cond = build_x_unary_op (loc, TRUTH_NOT_EXPR, condition, NULL_TREE,
				tf_warning_or_error);
  finish_if_stmt_cond (cond, if_stmt);
  if (semantic == CCS_NEVER || semantic == CCS_MAYBE)
    {
      tree violation = build_contract_violation (contract, /*is_const*/true);
      build_contract_handler_call (build_address (violation));
    }

  if (semantic == CCS_QUICK)
    {
      tree fn = builtin_decl_explicit (BUILT_IN_ABORT);
      releasing_vec vec;
      finish_expr_stmt (finish_call_expr (fn, &vec, false, false,
					  tf_warning_or_error));
    }
  else if (semantic == CCS_NEVER)
    /* FIXME: we should not call this when exceptions are disabled.  */
    finish_expr_stmt (build_call_a (terminate_fn, 0, nullptr));
  finish_then_clause (if_stmt);
  /* Finish the if stmt, but do not try to add it.  */
  tree scope = IF_SCOPE (if_stmt);
  IF_SCOPE (if_stmt) = NULL;
  return do_poplevel (scope);
}

/* This is called from push decl, and is used to determine if two sets of
    contract attributes match.  */

void
cxx2a_check_redecl_contract (tree newdecl, tree olddecl)
{

  tree old_contracts = DECL_CONTRACT_ATTRS(olddecl);
  tree new_contracts = DECL_CONTRACT_ATTRS(newdecl);

  if (!old_contracts && !new_contracts)
    return;

  location_t old_loc = DECL_SOURCE_LOCATION(olddecl);
  location_t new_loc = DECL_SOURCE_LOCATION(newdecl);

  /* If both declarations specify contracts, ensure they match.

   TODO: This handles a potential error a little oddly. Consider:

   struct B {
   virtual void f(int n) [[pre: n == 0]];
   };
   struct D : B {
   void f(int n) override; // inherits contracts
   };
   void D::f(int n) [[pre: n == 0]] // OK
   { }

   It's okay because we're explicitly restating the inherited contract.
   Changing the precondition on the definition D::f causes match_contracts
   to complain about the mismatch.

   This would previously have been diagnosed as adding contracts to an
   override, but this seems like it should be well-formed.  */
  if (old_contracts && new_contracts)
    {
      if (!contract_any_deferred_p (old_contracts)
	  && contract_any_deferred_p (new_contracts)
	  && DECL_UNIQUE_FRIEND_P (newdecl))
	{
	  /* The contracts of newdecl are still DEFERRED_PARSE, and we're about
	     to collapse it into olddecl, so stash away olddecl's contracts for
	     later comparison.  */
	  defer_guarded_contract_match (olddecl, olddecl, old_contracts);
	  /* Put the defered contracts on the olddecl so we parse it when
	     we can.  */
	  copy_deferred_contracts(newdecl, olddecl);
	}
      else if (contract_any_deferred_p (old_contracts)
	  || contract_any_deferred_p (new_contracts))
	  {
	  /* TODO: ignore these and figure out how to process them later.  */
	  /* Note that a friend declaration has deferred contracts, but the
	     declaration of the same function outside the class definition
	     doesn't.  */
	  }
      else
	match_contract_attributes (old_loc, old_contracts, new_loc,
				   new_contracts, cmc_declaration);

      return;
    }

  /* Handle cases where contracts are omitted in one or the other
   declaration.  */
  if (!old_contracts)
    {
      gcc_checking_assert(new_contracts);
      /* We are adding contracts to a declaration.  */
      if (DECL_INITIAL(olddecl))
	{
	  /* We can't add to a previously defined function.  */
	  auto_diagnostic_group d;
	  error_at (new_loc, "cannot add contracts after definition");
	  inform (DECL_SOURCE_LOCATION(olddecl), "original definition here");
	  return;
	}

      /* We can't add to an unguarded virtual function declaration.  */
      if (DECL_VIRTUAL_P(olddecl))
	{
	  auto_diagnostic_group d;
	  error_at (new_loc, "cannot add contracts to a virtual function");
	  inform (DECL_SOURCE_LOCATION(olddecl), "original declaration here");
	  return;
	}

      /* Depending on the "first declaration" rule, we may not be able
       to add contracts to a function after the fact; NOTE the option
       cannot be used to gate here since it takes a string which always
       compares != 0.  */
      if (flag_contract_strict_declarations)
	warning_at (new_loc, 1, "declaration adds contracts to %q#D", olddecl);

      /* The parent will copy or merge the contracts from NEWDECL to OLDDECL;
       the latter is empty so we need make no special action.  */
      if (contract_any_deferred_p (new_contracts))
	copy_deferred_contracts(newdecl, olddecl);
      else
	set_decl_contracts (
	    olddecl, copy_and_remap_contracts (olddecl, newdecl, cmk_all));

    }
}

/* ======= public API ====== */

contract_semantic
map_contract_semantic (const char *ident)
{
  if (strcmp (ident, "ignore") == 0)
    return CCS_IGNORE;
  else if (strcmp (ident, "assume") == 0)
    return CCS_ASSUME;
  else if (strcmp (ident, "check_never_continue") == 0)
    return CCS_NEVER;
  else if (strcmp (ident, "check_maybe_continue") == 0)
    return CCS_MAYBE;
  return CCS_INVALID;
}

contract_level
map_contract_level (const char *ident)
{
  if (strcmp (ident, "default") == 0)
    return CONTRACT_DEFAULT;
  else if (strcmp (ident, "audit") == 0)
    return CONTRACT_AUDIT;
  else if (strcmp (ident, "axiom") == 0)
    return CONTRACT_AXIOM;
  return CONTRACT_INVALID;
}


void
handle_OPT_fcontract_build_level_ (const char *arg)
{
  if (contracts_p1332_default || contracts_p1332_review || contracts_p1429)
    {
      error ("%<-fcontract-build-level=%> cannot be mixed with p1332/p1429");
      return;
    }
  else
    contracts_std = true;

  if (strcmp (arg, "off") == 0)
    flag_contract_build_level = OFF;
  else if (strcmp (arg, "default") == 0)
    flag_contract_build_level = DEFAULT;
  else if (strcmp (arg, "audit") == 0)
    flag_contract_build_level = AUDIT;
  else
    error ("%<-fcontract-build-level=%> must be off|default|audit");

  setup_default_contract_role ();
}

void
handle_OPT_fcontract_assumption_mode_ (const char *arg)
{
  if (contracts_p1332_default || contracts_p1332_review || contracts_p1429)
    {
      error ("%<-fcontract-assumption-mode=%> cannot be mixed with"
	  " p1332/p1429");
      return;
    }
  else
    contracts_std = true;

  if (strcmp (arg, "on") == 0)
    flag_contract_assumption_mode = true;
  else if (strcmp (arg, "off") == 0)
    flag_contract_assumption_mode = false;
  else
    error ("%<-fcontract-assumption-mode=%> must be %<on%> or %<off%>");

  setup_default_contract_role ();
}

void
handle_OPT_fcontract_continuation_mode_ (const char *arg)
{
  if (contracts_p1332_default || contracts_p1332_review || contracts_p1429)
    {
      error ("%<-fcontract-continuation-mode=%> cannot be mixed with"
	  " p1332/p1429");
      return;
    }
  else
    contracts_std = true;

  if (strcmp (arg, "on") == 0)
    flag_contract_continuation_mode = true;
  else if (strcmp (arg, "off") == 0)
    flag_contract_continuation_mode = false;
  else
    error ("%<-fcontract-continuation-mode=%> must be %<on%> or %<off%>");

  setup_default_contract_role ();
}

void
handle_OPT_fcontract_role_ (const char *arg)
{
  const char *name = arg;
  const char *vals = strchr (name, ':');
  if (vals == NULL)
    {
      error ("%<-fcontract-role=%> must be in the form role:semantics");
      return;
    }

  contract_semantic dess = CCS_INVALID, auss = CCS_INVALID, axss = CCS_INVALID;
  char *des = NULL, *aus = NULL, *axs = NULL;
  des = xstrdup (vals + 1);

  aus = strchr (des, ',');
  if (aus == NULL)
    {
      error ("%<-fcontract-role=%> semantics must include default,audit,axiom"
	  " values");
      goto validate;
    }
  *aus = '\0'; // null terminate des
  aus = aus + 1; // move past null

  axs = strchr (aus, ',');
  if (axs == NULL)
    {
      error ("%<-fcontract-role=%> semantics must include default,audit,axiom"
	  " values");
      goto validate;
    }
  *axs = '\0'; // null terminate aus
  axs = axs + 1; // move past null

  dess = lookup_concrete_semantic (des);
  auss = lookup_concrete_semantic (aus);
  axss = lookup_concrete_semantic (axs);
validate:
  free (des);
  if (dess == CCS_INVALID || auss == CCS_INVALID || axss == CCS_INVALID)
    return;

  bool is_defalult_role = role_name_equal (name, "default");
  bool is_review_role = role_name_equal (name, "review");
  bool is_std_role = is_defalult_role || is_review_role;
  if ((contracts_std && is_std_role) || (contracts_p1429 && is_defalult_role))
    {
      error ("%<-fcontract-role=%> cannot be mixed with std/p1429 contract"
	  " flags");
      return;
    }
  else if (is_std_role)
    {
      contracts_p1332_default |= is_defalult_role;
      contracts_p1332_review |= is_review_role;
    }

  contract_role *role = add_contract_role (name, dess, auss, axss);

  if (role == NULL)
    {
      // TODO: not enough space?
      error ("%<-fcontract-level=%> too many custom roles");
      return;
    }
  else
    validate_contract_role (role);
}

void
handle_OPT_fcontract_semantic_ (const char *arg)
{
  if (!strchr (arg, ':'))
    {
      error ("%<-fcontract-semantic=%> must be in the form level:semantic");
      return;
    }

  if (contracts_std || contracts_p1332_default)
    {
      error ("%<-fcontract-semantic=%> cannot be mixed with std/p1332 contract"
	  " flags");
      return;
    }
  contracts_p1429 = true;

  contract_role *role = get_contract_role ("default");
  if (!role)
    {
      error ("%<-fcontract-semantic=%> cannot find default role");
      return;
    }

  const char *semantic = strchr (arg, ':') + 1;
  contract_semantic sem = lookup_concrete_semantic (semantic);
  if (sem == CCS_INVALID)
    return;

  if (strncmp ("default:", arg, 8) == 0)
    role->default_semantic = sem;
  else if (strncmp ("audit:", arg, 6) == 0)
    role->audit_semantic = sem;
  else if (strncmp ("axiom:", arg, 6) == 0)
    role->axiom_semantic = sem;
  else
    error ("%<-fcontract-semantic=%> level must be default, audit, or axiom");
  validate_contract_role (role);
}

/* Emit the statement for an assertion attribute.  */

void
emit_assertion (tree attr)
{
  emit_contract_attr (attr);
}

/* Called on attribute lists that must not contain contracts.  If any
   contracts are present, issue an error diagnostic and return true.  */

bool
diagnose_misapplied_contracts (tree attributes)
{
  if (attributes == NULL_TREE)
    return false;

  tree contract_attr = find_contract (attributes);
  if (!contract_attr)
    return false;

  error_at (EXPR_LOCATION (CONTRACT_STATEMENT (contract_attr)),
	    "contracts must appertain to a function type");

  /* Invalidate the contract so we don't treat it as valid later on.  */
  invalidate_contract (TREE_VALUE (TREE_VALUE (contract_attr)));

  return true;
}


/* Set contracts of FNDECL to be CONTRACTS.  */

void
set_contract_attributes (tree fndecl, tree contracts)
{
  if (DECL_CONTRACT_ATTRS(fndecl))
    remove_contract_attributes (fndecl);
  tree attrs = chainon (DECL_ATTRIBUTES(fndecl), contracts);
  DECL_ATTRIBUTES (fndecl) = attrs;
}
/* Return TRUE iff ATTR has been parsed by the front-end as a c++2a contract
   attribute. */

bool
cxx_contract_attribute_p (const_tree attr)
{
  if (attr == NULL_TREE
      || TREE_CODE (attr) != TREE_LIST)
    return false;

  if (!TREE_PURPOSE (attr) || TREE_CODE (TREE_PURPOSE (attr)) != TREE_LIST)
    return false;
  if (!TREE_VALUE (attr) || TREE_CODE (TREE_VALUE (attr)) != TREE_LIST)
    return false;
  if (!TREE_VALUE (TREE_VALUE (attr)))
    return false;

  return (TREE_CODE (TREE_VALUE (TREE_VALUE (attr))) == PRECONDITION_STMT
      || TREE_CODE (TREE_VALUE (TREE_VALUE (attr))) == POSTCONDITION_STMT
      || TREE_CODE (TREE_VALUE (TREE_VALUE (attr))) == ASSERTION_STMT);
}

/* True if ATTR is an assertion.  */

bool
cp_contract_assertion_p (const_tree attr)
{
  /* This is only an assertion if it is a valid cxx contract attribute and the
     statement is an ASSERTION_STMT.  */
  return cxx_contract_attribute_p (attr)
    && TREE_CODE (CONTRACT_STATEMENT (attr)) == ASSERTION_STMT;
}


/* =========================== C++26 contracts ============================  */

/* FIXME: add a design description.... */
static tree
contracts_fixup_cdtorname (tree idin)
{
  const char *n = IDENTIFIER_POINTER (idin);
  size_t l = strlen (n);
  char *nn = xasprintf ("%.*s_", (int)l-1, n);
  tree nid = get_identifier (nn);
  free (nn);
  return nid;
}

/* Lookup a name in std::, or inject it.  */

static tree
lookup_std_type (tree name_id)
{
  tree res_type = lookup_qualified_name (std_node, name_id,
					 LOOK_want::TYPE
					 | LOOK_want::HIDDEN_FRIEND);

  if (TREE_CODE (res_type) == TYPE_DECL)
    res_type = TREE_TYPE (res_type);
  else
    {
      push_nested_namespace (std_node);
      res_type = make_class_type (RECORD_TYPE);
      create_implicit_typedef (name_id, res_type);
      DECL_SOURCE_LOCATION (TYPE_NAME (res_type)) = BUILTINS_LOCATION;
      DECL_CONTEXT (TYPE_NAME (res_type)) = current_namespace;
      pushdecl_namespace_level (TYPE_NAME (res_type), /*hidden*/true);
      pop_nested_namespace (std_node);
    }
  return res_type;
}

/* Build a layout-compatible internal version of source location __impl
   type.  */

static tree
get_contracts_source_location_impl_type (tree context = NULL_TREE)
{
  if (contracts_source_location_impl_type)
     return contracts_source_location_impl_type;

  /* First see if we have a declaration that we can use.  */
  tree contracts_source_location_type
    = lookup_std_type (get_identifier ("source_location"));

  if (contracts_source_location_type
      && contracts_source_location_type != error_mark_node
      && TYPE_FIELDS (contracts_source_location_type))
    {
      contracts_source_location_impl_type = get_source_location_impl_type ();
      return contracts_source_location_impl_type;
    }

  /* We do not, so build the __impl layout equivalent type, which must
     match <source_location>:
     struct __impl
      {
	  const char* _M_file_name;
	  const char* _M_function_name;
	  unsigned _M_line;
	  unsigned _M_column;
      }; */
  const tree types[] = { const_string_type_node,
			const_string_type_node,
			uint_least32_type_node,
			uint_least32_type_node };

 const char *names[] = { "_M_file_name",
			 "_M_function_name",
			 "_M_line",
			 "_M_column",
			};
  tree fields = NULL_TREE;
  unsigned n = 0;
  for (tree type : types)
  {
    /* finish_builtin_struct wants fields chained in reverse.  */
    tree next = build_decl (BUILTINS_LOCATION, FIELD_DECL,
			    get_identifier (names[n++]), type);
    DECL_CHAIN (next) = fields;
    fields = next;
  }

  iloc_sentinel ils (input_location);
  input_location = BUILTINS_LOCATION;
  contracts_source_location_impl_type = make_class_type (RECORD_TYPE);
  finish_builtin_struct (contracts_source_location_impl_type,
			 "__impl", fields, NULL_TREE);
  CLASSTYPE_AS_BASE (contracts_source_location_impl_type)
    = contracts_source_location_impl_type;
  xref_basetypes (contracts_source_location_impl_type, /*bases=*/NULL_TREE);
  DECL_CONTEXT (TYPE_NAME (contracts_source_location_impl_type)) = context;
  contracts_source_location_impl_type
    = cp_build_qualified_type (contracts_source_location_impl_type,
			       TYPE_QUAL_CONST);

  return contracts_source_location_impl_type;
}

static tree
get_src_loc_impl_ptr (location_t loc)
{
  if (!contracts_source_location_impl_type)
    get_contracts_source_location_impl_type ();

  tree fndecl = current_function_decl;
  /* We might be an outlined function.  */
  if (DECL_IS_PRE_FN_P (fndecl) || DECL_IS_POST_FN_P (fndecl))
    fndecl = get_orig_for_outlined (fndecl);

  gcc_checking_assert (fndecl);
  tree impl__
    = build_source_location_impl (loc, fndecl,
				  contracts_source_location_impl_type);
  tree p = build_pointer_type (contracts_source_location_impl_type);
  return build_fold_addr_expr_with_type_loc (loc, impl__, p);
}

/* Build a layout-compatible internal version of contract_violation type.  */

static tree
get_p9600_contract_violation_fields ()
{
  tree fields = NULL_TREE;
  /* Must match <contracts>:
  class contract_violation {
    uint16_t _M_version;
    assertion_kind _M_assertion_kind;
    evaluation_semantic _M_evaluation_semantic;
    detection_mode _M_detection_mode;
    const char* _M_comment;
    void *_M_src_loc_ptr;
    __vendor_ext* _M_ext;
  };
    If this changes, also update the initializer in
    build_contract_violation.  */
  const tree types[] = { uint16_type_node,
			 uint16_type_node,
			 uint16_type_node,
			 uint16_type_node,
			 const_string_type_node,
			 ptr_type_node,
			 ptr_type_node
			};
 const char *names[] = { "_M_version",
			 "_M_assertion_kind",
			 "_M_evaluation_semantic",
			 "_M_detection_mode",
			 "_M_comment",
			 "_M_src_loc_ptr",
			 "_M_ext",
			};
  unsigned n = 0;
  for (tree type : types)
    {
      /* finish_builtin_struct wants fields chained in reverse.  */
      tree next = build_decl (BUILTINS_LOCATION, FIELD_DECL,
				  get_identifier(names[n++]), type);
      DECL_CHAIN (next) = fields;
      fields = next;
    }
 return fields;
}

/* Get constract_assertion_kind of the specified contract. Used when building
 P2900R7 contract_violation object.  */

static int
get_contract_assertion_kind (tree contract)
{
  gcc_checking_assert (flag_contracts_nonattr);
  if (CONTRACT_ASSERTION_KIND (contract))
    {
      tree s = CONTRACT_ASSERTION_KIND (contract);
      tree i = (TREE_CODE (s) == INTEGER_CST) ? s
					      : DECL_INITIAL (STRIP_NOPS (s));
      gcc_checking_assert (!type_dependent_expression_p (s) && i);
      return (uint16_t) tree_to_uhwi (i);
    }

  switch (TREE_CODE (contract))
  {
    case ASSERTION_STMT:	return CAK_ASSERT;
    case PRECONDITION_STMT:	return CAK_PRE;
    case POSTCONDITION_STMT:	return CAK_POST;
    default:
       break;
  }

  gcc_unreachable ();
}

/* Get contract_evaluation_semantic of the specified contract.  */

static uint16_t
get_evaluation_semantic (tree contract)
{
  gcc_checking_assert (flag_contracts_nonattr);
  if (CONTRACT_EVALUATION_SEMANTIC (contract))
    {
      tree s = CONTRACT_EVALUATION_SEMANTIC (contract);
      tree i = (TREE_CODE (s) == INTEGER_CST) ? s
					      : DECL_INITIAL (STRIP_NOPS (s));
      gcc_checking_assert (!type_dependent_expression_p (s) && i);
      return (uint16_t) tree_to_uhwi (i);
    }

  /* Temporary; pick up the semantic from the bitfield and translate to the
     P2900 version.  */
  contract_semantic semantic = get_contract_semantic (contract);
  switch (semantic)
    {
      default:
	gcc_unreachable ();
      case CCS_IGNORE:
	return CES_IGNORE;
      case CCS_OBSERVE:
	return CES_OBSERVE;
      case CCS_ENFORCE:
	return CES_ENFORCE;
      case CCS_QUICK:
	return CES_QUICK;
    }
}

/* Insert a BUILT_IN_OBSERVABLE epoch marker.  */

static void
emit_builtin_observable ()
{
  tree fn = builtin_decl_explicit (BUILT_IN_OBSERVABLE);
  releasing_vec vec;
  fn = finish_call_expr (fn, &vec, false, false, tf_warning_or_error);
  finish_expr_stmt (fn);

}

/* Build a p2900 contract_violation layout compatible object. */

/* Constructor - possibly not constant. */

static tree
build_contract_violation_p2900_ctor (tree contract)
{
  bool can_be_const = true;
  uint16_t version = 1;
  /* Default CDM_PREDICATE_FALSE. */
  uint16_t detection_mode = CDM_PREDICATE_FALSE;

  tree assertion_kind = CONTRACT_ASSERTION_KIND (contract);
  if (!assertion_kind || really_constant_p (assertion_kind))
    {
      uint16_t kind = get_contract_assertion_kind (contract);
      assertion_kind = build_int_cst (uint16_type_node, kind);
    }
  else
    can_be_const = false;

  tree eval_semantic = CONTRACT_EVALUATION_SEMANTIC (contract);
  if (!eval_semantic || really_constant_p (eval_semantic))
    {
      uint16_t semantic = get_evaluation_semantic (contract);
      eval_semantic = build_int_cst (uint16_type_node, semantic);
    }
  else
    can_be_const = false;

  tree comment = CONTRACT_COMMENT (contract);
  if (comment && !really_constant_p (comment))
    can_be_const = false;

  tree std_src_loc_impl_ptr = CONTRACT_STD_SOURCE_LOC (contract);
  if (std_src_loc_impl_ptr)
    {
      std_src_loc_impl_ptr = convert_from_reference (std_src_loc_impl_ptr);
      if (!really_constant_p (std_src_loc_impl_ptr))
	can_be_const = false;
    }
  else
    std_src_loc_impl_ptr = get_src_loc_impl_ptr (EXPR_LOCATION (contract));

  /* Must match the type layout in builtin_contract_violation_type.  */
  tree f0 = next_aggregate_field (TYPE_FIELDS (builtin_contract_violation_type));
  tree f1 = next_aggregate_field (DECL_CHAIN (f0));
  tree f2 = next_aggregate_field (DECL_CHAIN (f1));
  tree f3 = next_aggregate_field (DECL_CHAIN (f2));
  tree f4 = next_aggregate_field (DECL_CHAIN (f3));
  tree f5 = next_aggregate_field (DECL_CHAIN (f4));
  tree f6 = next_aggregate_field (DECL_CHAIN (f5));
  tree ctor = build_constructor_va
    (builtin_contract_violation_type, 7,
     f0, build_int_cst (uint16_type_node, version),
     f1, assertion_kind,
     f2, eval_semantic,
     f3, build_int_cst (uint16_type_node, detection_mode),
     f4, comment,
     f5, std_src_loc_impl_ptr,
     f6, build_zero_cst (nullptr_type_node)); // __vendor_ext

  TREE_READONLY (ctor) = true;
  if (can_be_const)
    TREE_CONSTANT (ctor) = true;

  return ctor;
}

static tree
build_contract_violation_p2900_constant (tree ctor, tree contract, bool is_const)
{
  tree viol_ = contracts_tu_local_named_var
    (EXPR_LOCATION (contract), "Lcontract_violation",
     builtin_contract_violation_type, /*is_const*/true);

  TREE_CONSTANT (viol_) = is_const;
  DECL_INITIAL (viol_) = ctor;
  varpool_node::finalize_decl (viol_);

  return viol_;
}

static tree
build_contract_violation_p2900_var (tree ctor, tree contract)
{
  location_t loc = EXPR_LOCATION (contract);
  tree a_type
    = strip_top_quals (non_reference (builtin_contract_violation_type));
  tree viol_ = build_decl (loc, VAR_DECL, NULL_TREE, a_type);
  DECL_SOURCE_LOCATION (viol_) = loc;
  DECL_CONTEXT (viol_) = current_function_decl;
  DECL_ARTIFICIAL (viol_) = true;
  layout_decl (viol_, 0);
  DECL_INITIAL (viol_) = ctor;
  return viol_;
}

static tree
build_contract_violation_p2900 (tree contract, bool is_const)
{
  tree ctor = build_contract_violation_p2900_ctor (contract);
  return build_contract_violation_p2900_constant (ctor, contract, is_const);
}

/* Shared code between TU-local wrappers for the violation handler.  */

static tree
declare_one_violation_handler_wrapper (tree fn_name, tree fn_type,
				       tree p1_type, tree p2_type)
{
  location_t loc = BUILTINS_LOCATION;
  tree fn_decl = build_lang_decl_loc (loc, FUNCTION_DECL, fn_name, fn_type);
  DECL_CONTEXT (fn_decl) = FROB_CONTEXT (global_namespace);
  DECL_ARTIFICIAL (fn_decl) = true;
  DECL_INITIAL (fn_decl) = error_mark_node;
  /* Let the start function code fill in the result decl.  */
  DECL_RESULT (fn_decl) = NULL_TREE;
  /* Two args violation ref, dynamic info.  */
  tree parms = cp_build_parm_decl (fn_decl, NULL_TREE, p1_type);
  TREE_USED (parms) = true;
  DECL_READ_P (parms) = true;
  tree p2 = cp_build_parm_decl (fn_decl, NULL_TREE, p2_type);
  TREE_USED (p2) = true;
  DECL_READ_P (p2) = true;
  DECL_CHAIN (parms) = p2;
  DECL_ARGUMENTS (fn_decl) = parms;
  /* Make this function internal.  */
  TREE_PUBLIC (fn_decl) = false;
  DECL_EXTERNAL (fn_decl) = false;
  DECL_WEAK (fn_decl) = false;
  return fn_decl;
}

static GTY(()) tree __tu_has_violation = NULL_TREE;
static GTY(()) tree __tu_has_violation_exception = NULL_TREE;

static void
declare_violation_handler_wrappers ()
{
  if (__tu_has_violation && __tu_has_violation_exception)
    return;

  iloc_sentinel ils (input_location);
  input_location = BUILTINS_LOCATION;
  tree v_obj_type = builtin_contract_violation_type;
  v_obj_type = cp_build_qualified_type (v_obj_type, TYPE_QUAL_CONST);
  v_obj_type = cp_build_reference_type (v_obj_type, /*rval*/false);
  tree fn_type = build_function_type_list (void_type_node, v_obj_type,
					   uint16_type_node, NULL_TREE);
  tree fn_name = get_identifier ("__tu_has_violation_exception");
  __tu_has_violation_exception
    = declare_one_violation_handler_wrapper (fn_name, fn_type, v_obj_type,
					     uint16_type_node);
  fn_name = get_identifier ("__tu_has_violation");
  __tu_has_violation
    = declare_one_violation_handler_wrapper (fn_name, fn_type, v_obj_type,
					     uint16_type_node);
}

/* Expand a p2900 CONTRACT tree.  This is called during genericization.  */

static tree
build_contract_check_p2900 (tree contract)
{
  uint16_t semantic = get_evaluation_semantic (contract);
  if (semantic == CES_INVALID)
    return NULL_TREE;

  /* Ignored contracts are never checked or assumed.  */
  if (semantic == CES_IGNORE)
    return void_node;

  location_t loc = EXPR_LOCATION (contract);

  remap_dummy_this (current_function_decl, &CONTRACT_CONDITION (contract));
  tree condition = CONTRACT_CONDITION (contract);
  if (condition == error_mark_node)
    return NULL_TREE;

  bool check_might_throw = flag_exceptions
			   && !expr_noexcept_p (condition, tf_none);

  /* Build a read-only violation object, with the contract settings.  */
  /* Build a statement expression to hold a contract check, with the check
     potentially wrapped in a try-catch expr.  */
  tree cc_bind = build3 (BIND_EXPR, void_type_node, NULL, NULL, NULL);
  BIND_EXPR_BODY (cc_bind) = push_stmt_list ();

  if (!flag_contract_disable_check_epochs
      && TREE_CODE (contract) == ASSERTION_STMT)
    emit_builtin_observable ();
  tree cond = build_x_unary_op (loc, TRUTH_NOT_EXPR, condition, NULL_TREE,
				tf_warning_or_error);
  tree ctor = build_contract_violation_p2900_ctor (contract);
  tree violation = NULL_TREE;
  bool viol_is_var = false;
  if (TREE_CONSTANT (ctor))
    violation = build_contract_violation_p2900_constant (ctor, contract, /*is_const*/true);
  else
    {
      violation = build_contract_violation_p2900_var (ctor, contract);
      add_decl_expr (violation);
      BIND_EXPR_VARS (cc_bind) = violation;
      viol_is_var = true;
    }
  /* So now do we need a try-catch?  */
  if (check_might_throw)
    {
      /* This will hold the computed condition.  */
      tree check_failed = build_decl (loc, VAR_DECL, NULL, boolean_type_node);
      DECL_ARTIFICIAL (check_failed) = true;
      DECL_IGNORED_P (check_failed) = true;
      DECL_CONTEXT (check_failed) = current_function_decl;
      layout_decl (check_failed, 0);
      add_decl_expr (check_failed);
      DECL_CHAIN (check_failed) = BIND_EXPR_VARS (cc_bind);
      BIND_EXPR_VARS (cc_bind) = check_failed;
      /* This will let us check if we had an exception.  */
      tree no_excp_ = build_decl (loc, VAR_DECL, NULL, boolean_type_node);
      DECL_ARTIFICIAL (no_excp_) = true;
      DECL_IGNORED_P (no_excp_) = true;
      DECL_CONTEXT (no_excp_) = current_function_decl;
      DECL_INITIAL (no_excp_) = boolean_true_node;
      layout_decl (no_excp_, 0);
      add_decl_expr (no_excp_);
      DECL_CHAIN (no_excp_) = BIND_EXPR_VARS (cc_bind);
      BIND_EXPR_VARS (cc_bind) = no_excp_;

      tree check_try = begin_try_block ();
      finish_expr_stmt (cp_build_init_expr (check_failed, cond));
      finish_try_block (check_try);
      tree handler = begin_handler ();
      finish_handler_parms (NULL_TREE, handler); /* catch (...) */
      tree e = cp_build_modify_expr (loc, no_excp_, NOP_EXPR, boolean_false_node,
				     tf_warning_or_error);
      finish_expr_stmt (e);
      tree s_const = build_int_cst (uint16_type_node, semantic);
      if (viol_is_var)
	{
	  /* We can update the detection mode here.  */
	  tree memb
	    = lookup_member (builtin_contract_violation_type,
			     get_identifier ("_M_detection_mode"),
			     1, 0, tf_warning_or_error);
	  tree r
	    = build_class_member_access_expr (violation, memb, NULL_TREE, false,
					      tf_warning_or_error);
	  r = cp_build_modify_expr (loc, r, NOP_EXPR,
				build_int_cst (uint16_type_node, (uint16_t)CDM_EVAL_EXCEPTION),
				tf_warning_or_error);
	  finish_expr_stmt (r);
	  finish_expr_stmt (build_call_n (__tu_has_violation, 2,
					  build_address (violation),
					  s_const));
	}
      else
	/* We need to make a copy of the violation object to update.  */
	finish_expr_stmt (build_call_n (__tu_has_violation_exception, 2,
					build_address (violation),
					s_const));
      finish_handler (handler);
      finish_handler_sequence (check_try);
      cond = build2 (TRUTH_ANDIF_EXPR, boolean_type_node, check_failed, no_excp_);
      BIND_EXPR_VARS (cc_bind) = nreverse (BIND_EXPR_VARS (cc_bind));
    }

  tree do_check = begin_if_stmt ();
  finish_if_stmt_cond (cond, do_check);
  finish_expr_stmt (build_call_n (__tu_has_violation, 2,
				  build_address (violation),
				  build_int_cst (uint16_type_node, semantic)));
  finish_then_clause (do_check);
  finish_if_stmt (do_check);

  BIND_EXPR_BODY (cc_bind) = pop_stmt_list (BIND_EXPR_BODY (cc_bind));
  return cc_bind;
}

static location_t
get_contract_end_loc (tree contracts)
{
  tree last = NULL_TREE;
  for (tree a = contracts; a; a = TREE_CHAIN (a))
    last = a;
  gcc_checking_assert (last);
  last = CONTRACT_STATEMENT (last);
  /* FIXME: We should have a location including the condition, but maybe it is
     not available here.  */
#if 0
  if ((TREE_CODE (last) == POSTCONDITION_STMT)
      || (TREE_CODE (last) == PRECONDITION_STMT))
    last = CONTRACT_CONDITION (last);
#endif
  return EXPR_LOCATION (last);
}

struct contract_redecl
{
  tree original_contracts;
  tree contract_specifiers;
  tree pending_list;
  location_t note_loc;
};

static GTY(()) hash_map<tree, contract_redecl> *redeclared_contracts;

/* This is called via duplicate_decls, from pushdecl, and is used to determine
   if two sets of contract attributes match.  */

static void
p2900_check_redecl_contract (tree newdecl, tree olddecl)
{
  tree new_contracts = get_fn_contract_specifiers (newdecl);
  tree old_contracts = get_fn_contract_specifiers (olddecl);

  if (!old_contracts && !new_contracts)
    return;

  /* We should always be comparing with the 'first' declaration which should
     have been recorded already (if it has contract specifiers).  However
     if the new decl is trying to add contracts, that is an error and we do
     not want to create a map entry yet.  */
  contract_redecl *rdp = hash_map_safe_get (redeclared_contracts, olddecl);
  gcc_checking_assert (rdp || !old_contracts);

  location_t new_loc = DECL_SOURCE_LOCATION (newdecl);
  if (new_contracts && !old_contracts)
    {
      auto_diagnostic_group d;
      /* If a re-declaration has contracts, they must be the same as those that
	 appear on the first declaration seen (they cannot be added).  */
      location_t cont_end = get_contract_end_loc (new_contracts);
      cont_end = make_location (new_loc, new_loc, cont_end);
      error_at (cont_end, "declaration adds contracts to %q#D", olddecl);
      location_t old_loc;
      if (rdp && rdp->note_loc)
	old_loc = rdp->note_loc;
      else
	old_loc = DECL_SOURCE_LOCATION (olddecl);
      inform (old_loc , "first declared here");
      return;
    }

  if (old_contracts && !new_contracts)
    {
      /* We allow re-declarations to omit contracts declared on the initial decl.
       In fact, this is required if the conditions contain lambdas.  Check if
       all the parameters are correctly const qualified. */
      check_postconditions_in_redecl (olddecl, newdecl);
    }
  else if (old_contracts && new_contracts &&
      !contract_any_deferred_p (old_contracts)
      && contract_any_deferred_p (new_contracts)
      && DECL_UNIQUE_FRIEND_P (newdecl))
    {
	/* Newdecl's contracts are still DEFERRED_PARSE, and we're about to
	   collapse it into olddecl, so stash away olddecl's contracts for
	   later comparison.  */
	defer_guarded_contract_match (olddecl, olddecl, old_contracts);
	/* put the defered contracts on the olddecl so we parse it when
	  we can.  */
	copy_deferred_contracts (newdecl, olddecl);
    }
  else if (contract_any_deferred_p (old_contracts)
	   || contract_any_deferred_p (new_contracts))
    {
      /* TODO: ignore these and figure out how to process them later.  */
      /* Note that a friend declaration has deferred contracts, but the
	 declaration of the same function outside the class definition
	 doesn't.  */
    }
  else
    {
      gcc_checking_assert (old_contracts);
      location_t cont_end = get_contract_end_loc (new_contracts);
      cont_end = make_location (new_loc, new_loc, cont_end);
      /* We have two sets - they should match or we issue a diagnostic.  */
      match_contract_attributes (rdp->note_loc, rdp->original_contracts,
				 cont_end, new_contracts, cmc_declaration);
    }

  return;
}

/* ======== public API ======= */

tree
init_builtin_contract_violation_type ()
{
  if (builtin_contract_violation_type)
    return builtin_contract_violation_type;

  tree fields;
  if (flag_contracts_nonattr)
    fields = get_p9600_contract_violation_fields ();
  else
    fields = get_cxx2a_contract_violation_fields ();

  iloc_sentinel ils (input_location);
  input_location = BUILTINS_LOCATION;
  builtin_contract_violation_type = make_class_type (RECORD_TYPE);
  finish_builtin_struct (builtin_contract_violation_type,
			 "__builtin_contract_violation_type", fields, NULL_TREE);
  CLASSTYPE_AS_BASE (builtin_contract_violation_type)
    = builtin_contract_violation_type;
  DECL_CONTEXT (TYPE_NAME (builtin_contract_violation_type))
    = FROB_CONTEXT (global_namespace);
  TREE_PUBLIC (TYPE_NAME (builtin_contract_violation_type)) = true;
  CLASSTYPE_LITERAL_P (builtin_contract_violation_type) = true;
  CLASSTYPE_LAZY_COPY_CTOR (builtin_contract_violation_type) = true;
  xref_basetypes (builtin_contract_violation_type, /*bases=*/NULL_TREE);
  builtin_contract_violation_type
    = cp_build_qualified_type (builtin_contract_violation_type,
			       TYPE_QUAL_CONST);
  return builtin_contract_violation_type;
}

/* Genericize a CONTRACT tree, but do not attach it to the current context,
   the caller is responsible for that.
   This is called during genericization.  */

tree
build_contract_check (tree contract)
{
  if (flag_contracts_nonattr)
    {
      declare_violation_handler_wrappers ();
      return build_contract_check_p2900 (contract);
    }
  return build_contract_check_cxx2a (contract);
}


/* Converts a contract condition to bool and ensures it has a location.  */

tree
finish_contract_condition (cp_expr condition)
{
  if (!condition || error_operand_p (condition))
    return condition;

  /* Ensure we have the condition location saved in case we later need to
     emit a conversion error during template instantiation and wouldn't
     otherwise have it.  This differs from maybe_wrap_with_location in that
     it allows wrappers on EXCEPTIONAL_CLASS_P which includes CONSTRUCTORs.  */
  if (!CAN_HAVE_LOCATION_P (condition)
      && condition.get_location () != UNKNOWN_LOCATION)
    {
      tree_code code
	= (((CONSTANT_CLASS_P (condition) && TREE_CODE (condition) != STRING_CST)
	    || (TREE_CODE (condition) == CONST_DECL && !TREE_STATIC (condition)))
	  ? NON_LVALUE_EXPR : VIEW_CONVERT_EXPR);
      condition = build1_loc (condition.get_location (), code,
			      TREE_TYPE (condition), condition);
      EXPR_LOCATION_WRAPPER_P (condition) = true;
    }

  if (type_dependent_expression_p (condition))
    return condition;

  return condition_conversion (condition);
}

/* Wrap the DECL into VIEW_CONVERT_EXPR representing const qualified version
   of the declaration.  */

tree
view_as_const (tree decl)
{
  if (!contract_const_wrapper_p (decl))
    {
      tree ctype = TREE_TYPE (decl);
      ctype = cp_build_qualified_type (ctype, (cp_type_quals (ctype)
					       | TYPE_QUAL_CONST));
      decl = build1 (VIEW_CONVERT_EXPR, ctype, decl);
      /* Mark the VCE as contract const wrapper.  */
      decl->base.private_flag = true;
    }
  return decl;
}

/* Constify access to DECL from within the contract condition.  */

tree
constify_contract_access (tree decl)
{
  /* We check if we have a variable, a parameter, a variable of reference type,
   * or a parameter of reference type
   */
  if (!TREE_READONLY (decl)
      && (VAR_P (decl)
	  || (TREE_CODE (decl) == PARM_DECL)
	  || (REFERENCE_REF_P (decl)
	      && (VAR_P (TREE_OPERAND (decl, 0))
		  || (TREE_CODE (TREE_OPERAND (decl, 0)) == PARM_DECL)
		  || (TREE_CODE (TREE_OPERAND (decl, 0))
		      == TEMPLATE_PARM_INDEX)))))
    decl = view_as_const (decl);

  return decl;
}

/* If declaration DECL is a PARM_DECL and it appears in a postcondition, then
   check that it is not a non-const by-value param. LOCATION is where the
   expression was found and is used for diagnostic purposes.  */

void
check_param_in_postcondition (tree decl, location_t location)
{
  if (flag_contracts_nonattr
      && TREE_CODE (decl) == PARM_DECL
      && processing_postcondition
      && !cp_unevaluated_operand
      && !(REFERENCE_REF_P (decl)
	   && TREE_CODE (TREE_OPERAND (decl, 0)) == PARM_DECL)
	   /* Return value parameter has DECL_ARTIFICIAL flag set. The flag
	    * presence of the flag should be sufficient to distinguish the
	    * return value parameter in this context. */
	   && !(DECL_ARTIFICIAL (decl)))
    {
      set_parm_used_in_post (decl);

      if (!dependent_type_p (TREE_TYPE (decl))
	  && !CP_TYPE_CONST_P (TREE_TYPE (decl))
	  && !TREE_READONLY (decl))
	{
	  error_at (location,
		    "a value parameter used in a postcondition must be const");
	  inform (DECL_SOURCE_LOCATION (decl), "parameter declared here");
	}
    }
}

/* Check if parameters used in postconditions are const qualified on
   a redeclaration that does not specify contracts or on an instantiation
   of a function template.  */

void
check_postconditions_in_redecl (tree olddecl, tree newdecl)
{
  tree attr = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (olddecl)
	      : DECL_CONTRACT_ATTRS (olddecl);
  if (!attr)
    return;

  tree t1 = FUNCTION_FIRST_USER_PARM (olddecl);
  tree t2 = FUNCTION_FIRST_USER_PARM (newdecl);

  for (; t1 && t1 != void_list_node;
  t1 = TREE_CHAIN (t1), t2 = TREE_CHAIN (t2))
    {
      if (parm_used_in_post_p (t1))
	{
	  set_parm_used_in_post (t2);
	  if (!dependent_type_p (TREE_TYPE (t2))
	      && !CP_TYPE_CONST_P (TREE_TYPE (t2))
	      && !TREE_READONLY (t2))
	    {
	      error_at (DECL_SOURCE_LOCATION (t2),
	      "value parameter %qE used in a postcondition must be const", t2);
	      inform (DECL_SOURCE_LOCATION (olddecl),
	      "previous declaration here");
	    }
	}
    }
}

void
maybe_update_postconditions (tree fndecl)
{
  /* Update any postconditions and the postcondition checking function
     as needed.  If there are postconditions, we'll use those to rewrite
     return statements to check postconditions.  */
  if (has_active_postconditions (fndecl))
    {
      rebuild_postconditions (fndecl);
      tree post = build_postcondition_function (fndecl);
      set_postcondition_function (fndecl, post);
    }
}

void
start_function_contracts (tree fndecl)
{
  if (error_operand_p (fndecl))
    return;

  if (!handle_contracts_p (fndecl))
    return;

  /* Even if we will use an outlined function for the check (which will be the
     same one we might use on the callee-side) we still need to check the re-
     mapped contracts for shadowing.  */

  /* Check that the user did not try to shadow a function parameter with the
     specified postcondition result name.  */
  if (flag_contracts_nonattr)
    for (tree ca = GET_FN_CONTRACT_SPECIFIERS (fndecl); ca; ca = TREE_CHAIN (ca))
      if (POSTCONDITION_P (CONTRACT_STATEMENT (ca)))
	if (tree id = POSTCONDITION_IDENTIFIER (CONTRACT_STATEMENT (ca)))
	  {
	    tree r_name = tree_strip_any_location_wrapper (id);
	    if (TREE_CODE (id) == PARM_DECL)
	      r_name = DECL_NAME (id);
	    gcc_checking_assert (r_name && TREE_CODE (r_name) == IDENTIFIER_NODE);
	    tree seen = lookup_name (r_name);
	    if (seen
		&& TREE_CODE (seen) == PARM_DECL
		&& DECL_CONTEXT (seen)
		&& DECL_CONTEXT (seen) == fndecl)
	      {
		auto_diagnostic_group d;
		location_t id_l = location_wrapper_p (id)
				  ? EXPR_LOCATION (id)
				  : DECL_SOURCE_LOCATION (id);
		location_t co_l = EXPR_LOCATION (CONTRACT_STATEMENT (ca));
		if (id_l != UNKNOWN_LOCATION)
		  co_l = make_location (id_l, get_start (co_l), get_finish (co_l));
		error_at (co_l,
			  "contract postcondition result names must not shadow"
			  " function parameters");
		inform (DECL_SOURCE_LOCATION (seen),
			"parameter declared here");
		POSTCONDITION_IDENTIFIER (CONTRACT_STATEMENT (ca))
		  = error_mark_node;
		CONTRACT_CONDITION (CONTRACT_STATEMENT (ca)) = error_mark_node;
	      }
	  }

  /* Contracts may have just been added without a chance to parse them, though
     we still need the PRE_FN available to generate a call to it.  */
  /* Do we already have declarations generated ? */
  if (!DECL_PRE_FN (fndecl) && !DECL_POST_FN (fndecl))
    build_contract_function_decls (fndecl);
}

/* Add contract handling to the function in FNDECL.

   When we have only pre-conditions, this simply prepends a call (or a direct
   evaluation, for cdtors) to the existing function body.

   When we have post conditions we build a try-finally block.
   If the function might throw then the handler in the try-finally is an
   EH_ELSE expression, where the post condition check is applied to the
   non-exceptional path, and an empty statement is added to the EH path.  If
   the function has a non-throwing eh spec, then the handler is simply the
   post-condition checker.  */

void
maybe_apply_function_contracts (tree fndecl)
{
  if (!handle_contracts_p (fndecl))
    /* We did nothing and the original function body statement list will be
       popped by our caller.  */
    return;

  bool do_pre = has_active_preconditions (fndecl);
  bool do_post = has_active_postconditions (fndecl);
  /* We should not have reached here with nothing to do... */
  gcc_checking_assert (do_pre || do_post);

  /* If the function is noexcept, the user's written body will be wrapped in a
     MUST_NOT_THROW expression.  In that case we want to extract the body from
     that and build the replacement (including the pre and post-conditions as
     needed) in its place.  */
  tree fnbody;
  if (TYPE_NOEXCEPT_P (TREE_TYPE (fndecl)))
    {
      tree m_n_t_expr = expr_first (DECL_SAVED_TREE (fndecl));
      gcc_checking_assert (TREE_CODE (m_n_t_expr) == MUST_NOT_THROW_EXPR);
      fnbody = TREE_OPERAND (m_n_t_expr, 0);
      TREE_OPERAND (m_n_t_expr, 0) = push_stmt_list ();
    }
  else
    {
      fnbody = DECL_SAVED_TREE (fndecl);
      DECL_SAVED_TREE (fndecl) = push_stmt_list ();
    }

  /* Now add the pre and post conditions to the existing function body.
     This copies the approach used for function try blocks.  */
  tree compound_stmt = begin_compound_stmt (0);
  current_binding_level->artificial = true;

  /* Do not add locations for the synthesised code.  */
  location_t loc = UNKNOWN_LOCATION;

  /* For other cases, we call a function to process the check.  */

  /* If we have a pre, but not a post, then just emit that and we are done.  */
  if (!do_post)
    {
      apply_preconditions (fndecl);
      add_stmt (fnbody);
      finish_compound_stmt (compound_stmt);
      return;
    }

  if (do_pre)
    /* Add a precondition call, if we have one. */
    apply_preconditions (fndecl);
  tree try_fin = build_stmt (loc, TRY_FINALLY_EXPR, fnbody, NULL_TREE);
  add_stmt (try_fin);
  TREE_OPERAND (try_fin, 1) = push_stmt_list ();
  /* If we have exceptions, and a function that might throw, then add
     an EH_ELSE clause that allows the exception to propagate upwards
     without encountering the post-condition checks.  */
  if (flag_exceptions && !type_noexcept_p (TREE_TYPE (fndecl)))
    {
      tree eh_else = build_stmt (loc, EH_ELSE_EXPR, NULL_TREE, NULL_TREE);
      add_stmt (eh_else);
      TREE_OPERAND (eh_else, 0) = push_stmt_list ();
      apply_postconditions (fndecl);
      TREE_OPERAND (eh_else, 0) = pop_stmt_list (TREE_OPERAND (eh_else, 0));
      TREE_OPERAND (eh_else, 1) = build_empty_stmt (loc);
    }
  else
    apply_postconditions (fndecl);
  TREE_OPERAND (try_fin, 1) = pop_stmt_list (TREE_OPERAND (try_fin, 1));
  finish_compound_stmt (compound_stmt);
  /* The DECL_SAVED_TREE stmt list will be popped by our caller.  */
}

/* Returns a copy of SOURCE contracts where any references to SOURCE's
   PARM_DECLs have been rewritten to the corresponding PARM_DECL in DEST.  */

tree
copy_and_remap_contracts (tree dest, tree source,
			  contract_match_kind remap_kind)
{
  tree last = NULL_TREE, contract_attrs = NULL_TREE;
  tree attr = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (source)
	      : DECL_CONTRACT_ATTRS (source);
  for (; attr; attr = NEXT_CONTRACT_ATTR (attr))
    {
      if ((remap_kind == cmk_pre
	   && (TREE_CODE (CONTRACT_STATEMENT (attr)) == POSTCONDITION_STMT))
	  || (remap_kind == cmk_post
	      && (TREE_CODE (CONTRACT_STATEMENT (attr)) == PRECONDITION_STMT)))
	continue;

      tree c = copy_node (attr);
      TREE_VALUE (c) = build_tree_list (TREE_PURPOSE (TREE_VALUE (c)),
					copy_node (CONTRACT_STATEMENT (c)));

      remap_contract (source, dest, CONTRACT_STATEMENT (c),
		      /*duplicate_p=*/true);

      if (TREE_CODE (CONTRACT_STATEMENT (c)) == POSTCONDITION_STMT)
	{
	  tree oldvar = POSTCONDITION_IDENTIFIER (CONTRACT_STATEMENT (c));
	  if (oldvar && oldvar != error_mark_node)
	    {
	      //tree type = copy_node (oldvar);
	      DECL_CONTEXT (oldvar) = dest;
	    }
	}

      CONTRACT_COMMENT (CONTRACT_STATEMENT (c))
	= copy_node (CONTRACT_COMMENT (CONTRACT_STATEMENT (c)));

      chainon (last, c);
      last = c;
      if (!contract_attrs)
	contract_attrs = c;
    }

  return contract_attrs;
}

/* Finish up the pre & post function definitions for a guarded FNDECL,
   and compile those functions all the way to assembler language output.  */

void
finish_function_contracts (tree fndecl)
{
  /* If the guarded func is either already decided to be ill-formed or is
     not yet complete return early.  */
  if (error_operand_p (fndecl)
      || !DECL_INITIAL (fndecl)
      || DECL_INITIAL (fndecl) == error_mark_node)
    return;

  /* If there are no contracts here do not need to build the outlined functions.  */
  if (!handle_contracts_p (fndecl))
    return;

  tree attr = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
	      : DECL_CONTRACT_ATTRS (fndecl);
  for (; attr; attr = NEXT_CONTRACT_ATTR (attr))
    {
      tree contract = CONTRACT_STATEMENT (attr);
      if (!CONTRACT_CONDITION (contract)
	  || CONTRACT_CONDITION (contract) == error_mark_node)
	return;
      /* We are generating code, deferred parses should be complete.  */
      gcc_checking_assert (!CONTRACT_CONDITION_DEFERRED_P (contract));
    }

  int flags = SF_DEFAULT | SF_PRE_PARSED;

  /* If either the pre or post functions are bad, don't bother emitting
     any contracts.  The program is already ill-formed.  */
  tree pre = DECL_PRE_FN (fndecl);
  tree post = DECL_POST_FN (fndecl);
  if (pre == error_mark_node || post == error_mark_node)
    return;

  if (pre && !DECL_INITIAL (pre))
    {
      DECL_PENDING_INLINE_P (pre) = false;
      start_preparsed_function (pre, DECL_ATTRIBUTES (pre), flags);
      remap_and_emit_conditions (fndecl, pre, PRECONDITION_STMT);
      finish_return_stmt (NULL_TREE);
      pre = finish_function (false);
      expand_or_defer_fn (pre);
    }

  if (post && !DECL_INITIAL (post))
    {
      DECL_PENDING_INLINE_P (post) = false;
      start_preparsed_function (post, DECL_ATTRIBUTES (post), flags);
      remap_and_emit_conditions (fndecl, post, POSTCONDITION_STMT);
      gcc_checking_assert (VOID_TYPE_P (TREE_TYPE (TREE_TYPE (post))));
      finish_return_stmt (NULL_TREE);
      post = finish_function (false);
      expand_or_defer_fn (post);
    }
}

/* Set the (maybe) parsed contract specifier LIST for DECL.  */

void
set_fn_contract_specifiers (tree decl, tree list)
{
  if (!decl || error_operand_p (decl))
    return;

  bool existed = false;
  contract_redecl& rd
    = hash_map_safe_get_or_insert<hm_ggc> (redeclared_contracts, decl, &existed);
  if (!existed)
    {
      /* This is the first time we encountered this decl, save the contracts
	 and supporting data; to compare redeclarations with.  */
      rd.original_contracts = list;
      location_t decl_loc = DECL_SOURCE_LOCATION (decl);
      location_t cont_end = decl_loc;
      if (list)
	cont_end = get_contract_end_loc (list);
      rd.note_loc = make_location (decl_loc, decl_loc, cont_end);
    }
  rd.contract_specifiers = list;
}

void
update_fn_contract_specifiers (tree decl, tree list)
{
  if (!decl || error_operand_p (decl))
    return;

  bool existed = false;
  contract_redecl& rd
    = hash_map_safe_get_or_insert<hm_ggc> (redeclared_contracts, decl, &existed);
  gcc_checking_assert (existed);
  /* We might come here multiple times as we iterate through deferred contracts
     on a decl, wait until they are all done and then copy.  */
  if (!contract_any_deferred_p (list))
    {
      /* See above.  */
      rd.original_contracts = copy_contracts_list (list, decl);
      location_t decl_loc = DECL_SOURCE_LOCATION (decl);
      location_t cont_end = decl_loc;
      if (list)
	cont_end = get_contract_end_loc (list);
      rd.note_loc = make_location (decl_loc, decl_loc, cont_end);
    }
  rd.contract_specifiers = list;
}

/* When a decl is about to be removed, then we need to release its content and
   then take it out of the map.  */
void
remove_decl_with_fn_contracts_specifiers (tree decl)
{
  if (contract_redecl *p = hash_map_safe_get (redeclared_contracts, decl))
    {
      p->contract_specifiers = NULL_TREE;
      p->original_contracts = NULL_TREE;
      p->pending_list = NULL_TREE;
      redeclared_contracts->remove (decl);
    }
}

/* If this function has contract specifiers, then remove them, but leave the
   function registered.  */
void
remove_fn_contract_specifiers (tree decl)
{
  if (contract_redecl *p = hash_map_safe_get (redeclared_contracts, decl))
    {
      p->contract_specifiers = NULL_TREE;
      p->original_contracts = NULL_TREE;
      p->pending_list = NULL_TREE;
    }
}

/* Get the contract specifier list for this DECL if there is one.  */

tree
get_fn_contract_specifiers (tree decl)
{
  if (contract_redecl *p = hash_map_safe_get (redeclared_contracts, decl))
    return p->contract_specifiers;
  return NULL_TREE;
}

/* A subroutine of duplicate_decls. Diagnose issues in the redeclaration of
   guarded functions.  */
void
check_redecl_contract (tree newdecl, tree olddecl)
{
  if (TREE_CODE (newdecl) == TEMPLATE_DECL)
    newdecl = DECL_TEMPLATE_RESULT (newdecl);
  if (TREE_CODE (olddecl) == TEMPLATE_DECL)
    olddecl = DECL_TEMPLATE_RESULT (olddecl);

  if (flag_contracts_nonattr)
    p2900_check_redecl_contract (newdecl, olddecl);
  else
    cxx2a_check_redecl_contract (newdecl, olddecl);

}

/* Update the contracts of DEST to match the argument names from contracts
  of SRC. When we merge two declarations in duplicate_decls, we preserve the
  arguments from the new declaration, if the new declaration is a
  definition. We need to update the contracts accordingly.  */
void update_contract_arguments(tree srcdecl, tree destdecl)
{
  tree src_contracts = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (srcdecl)
	      : DECL_CONTRACT_ATTRS (srcdecl);
  tree dest_contracts = flag_contracts_nonattr
	      ? GET_FN_CONTRACT_SPECIFIERS (destdecl)
	      : DECL_CONTRACT_ATTRS (destdecl);

  if (!src_contracts && !dest_contracts)
    return;

  /* C++20 contracts allowed first declaration to omit contracts.
    Handle this first so it's easily stripped out later.  */
  if (!flag_contracts_nonattr && !dest_contracts)
    {
      if (contract_any_deferred_p (src_contracts))
	copy_deferred_contracts(srcdecl, destdecl);
      else
	{
	  /* temporarily rename the arguments to get the right mapping */
	  tree tmp_arguments = DECL_ARGUMENTS (destdecl);
	  DECL_ARGUMENTS (destdecl) = DECL_ARGUMENTS (srcdecl);
	  set_decl_contracts (destdecl,
			      copy_and_remap_contracts (destdecl, srcdecl,
							cmk_all));
	  DECL_ARGUMENTS (destdecl) = tmp_arguments;
	}
      return;
    }

  /* Check if src even has contracts. It is possible that a redeclaration
    does not have contracts. Is this is the case, first apply contracts
    to src.
   */
  if (!src_contracts)
    {
      if (contract_any_deferred_p (dest_contracts))
	{
	  copy_deferred_contracts (destdecl, srcdecl);
	  /* Nothing more to do here.  */
	  return;
	}
      else
	set_decl_contracts (srcdecl,
			    copy_and_remap_contracts (srcdecl, destdecl,
						      cmk_all));
    }

  /* For deferred contracts, we currently copy the tokens from the redeclaration
    onto the decl that will be preserved. This is not ideal because the
    redeclaration may have erroneous contracts.
    For non deferred contracts we currently do copy and remap, which is doing
    more than we need.  */
  if (contract_any_deferred_p (src_contracts))
    {
      copy_deferred_contracts(srcdecl, destdecl);
    }
  else
    {
      /* temporarily rename the arguments to get the right mapping */
      tree tmp_arguments = DECL_ARGUMENTS (destdecl);
      DECL_ARGUMENTS (destdecl) = DECL_ARGUMENTS (srcdecl);
      set_decl_contracts (destdecl,
			  copy_and_remap_contracts (destdecl, srcdecl,
						    cmk_all));
      DECL_ARGUMENTS (destdecl) = tmp_arguments;
    }

}

void
maybe_emit_violation_handler_wrappers ()
{
  if (!__tu_has_violation && !__tu_has_violation_exception)
    return;

  /* __tu_has_violation */
  start_preparsed_function (__tu_has_violation, NULL_TREE,
			    SF_DEFAULT | SF_PRE_PARSED);
  tree body = begin_function_body ();
  tree compound_stmt = begin_compound_stmt (BCS_FN_BODY);
  tree v = DECL_ARGUMENTS (__tu_has_violation);
  tree semantic = DECL_CHAIN (v);

  /* We might be done.  */
  tree cond = build2 (EQ_EXPR, uint16_type_node, semantic,
		      build_int_cst (uint16_type_node, (uint16_t)CES_QUICK));
  tree if_quick = begin_if_stmt ();
  finish_if_stmt_cond (cond, if_quick);
  finish_expr_stmt (build_call_a (terminate_fn, 0, nullptr));
  finish_then_clause (if_quick);
  finish_if_stmt (if_quick);

  /* We are going to call the handler.  */
  build_contract_handler_call (v);

  tree if_observe = begin_if_stmt ();
  /* if (observe) return; */
  cond = build2 (EQ_EXPR, uint16_type_node, semantic,
		 build_int_cst (uint16_type_node, (uint16_t)CES_OBSERVE));
  finish_if_stmt_cond (cond, if_observe);
  if (!flag_contract_disable_check_epochs)
    emit_builtin_observable ();
  finish_return_stmt (NULL_TREE);
  finish_then_clause (if_observe);
  finish_if_stmt (if_observe);

  /* else terminate.  */
  finish_expr_stmt (build_call_a (terminate_fn, 0, nullptr));
  finish_compound_stmt (compound_stmt);
  finish_function_body (body);
  __tu_has_violation = finish_function (false);
  expand_or_defer_fn (__tu_has_violation);

  /* __tu_has_violation_exception */
  start_preparsed_function (__tu_has_violation_exception, NULL_TREE,
			    SF_DEFAULT | SF_PRE_PARSED);
  body = begin_function_body ();
  compound_stmt = begin_compound_stmt (BCS_FN_BODY);
  v = DECL_ARGUMENTS (__tu_has_violation_exception);
  semantic = DECL_CHAIN (v);
  location_t loc = DECL_SOURCE_LOCATION (__tu_has_violation_exception);
  tree a_type = strip_top_quals (non_reference (TREE_TYPE (v)));
  tree v2 = build_decl (loc, VAR_DECL, NULL_TREE, a_type);
  DECL_SOURCE_LOCATION (v2) = loc;
  DECL_CONTEXT (v2) = current_function_decl;
  DECL_ARTIFICIAL (v2) = true;
  layout_decl (v2, 0);
  v2 = pushdecl (v2);
  add_decl_expr (v2);
  tree r = cp_build_init_expr (v2, convert_from_reference (v));
  finish_expr_stmt (r);
  tree memb = lookup_member (a_type, get_identifier ("_M_detection_mode"),
		     /*protect=*/1, /*want_type=*/0, tf_warning_or_error);
  r = build_class_member_access_expr (v2, memb, NULL_TREE, false,
				      tf_warning_or_error);
  r = cp_build_modify_expr
   (loc, r, NOP_EXPR,
    build_int_cst (uint16_type_node, (uint16_t)CDM_EVAL_EXCEPTION),
    tf_warning_or_error);
  finish_expr_stmt (r);
  finish_expr_stmt (build_call_n (__tu_has_violation, 2, build_address (v2), semantic));
  finish_return_stmt (NULL_TREE);
  finish_compound_stmt (compound_stmt);
  finish_function_body (body);
  __tu_has_violation_exception = finish_function (false);
  expand_or_defer_fn (__tu_has_violation_exception);
}

/* ============ external API ============ */

/* Remove contract attributes from decl FNDECL. Returns the
   removed contracts. */

tree
remove_contract_attributes (tree fndecl)
{
  if (flag_contracts_nonattr)
    return NULL_TREE;

  tree contracts = NULL_TREE;
  tree other = NULL_TREE;
  for (tree a = DECL_ATTRIBUTES (fndecl); a; a = TREE_CHAIN (a))
    {
      tree list = tree_cons (TREE_PURPOSE (a), TREE_VALUE (a), NULL_TREE);
      if (cxx_contract_attribute_p (a))
	contracts = chainon (contracts, list);
      else
	other = chainon (other, list);
    }
  DECL_ATTRIBUTES (fndecl) = other;
  return contracts;
}

/* Returns true if all attributes are contracts.  */

bool
all_attributes_are_contracts_p (tree attributes)
{
  for (; attributes; attributes = TREE_CHAIN (attributes))
    if (!cxx_contract_attribute_p (attributes))
      return false;
  return true;
}

/* Mark most of a contract as being invalid.  */

tree
invalidate_contract (tree contract)
{
  if (TREE_CODE (contract) == POSTCONDITION_STMT
      && POSTCONDITION_IDENTIFIER (contract))
    POSTCONDITION_IDENTIFIER (contract) = error_mark_node;
  CONTRACT_CONDITION (contract) = error_mark_node;
  CONTRACT_COMMENT (contract) = error_mark_node;
  return contract;
}

/* Returns an invented parameter declaration of the form 'TYPE ID' for the
   purpose of parsing the postcondition.

   We use a PARM_DECL instead of a VAR_DECL so that tsubst forces a lookup
   in local specializations when we instantiate these things later.  */

tree
make_postcondition_variable (cp_expr id, tree type)
{
  if (id == error_mark_node)
    return id;
  gcc_checking_assert (scope_chain && scope_chain->bindings
		       && scope_chain->bindings->kind == sk_contract);

  tree decl = build_lang_decl (PARM_DECL, id, type);
  DECL_ARTIFICIAL (decl) = true;
  DECL_SOURCE_LOCATION (decl) = id.get_location ();

  pushdecl (decl);
  return decl;
}

/* As above, except that the type is unknown.  */

tree
make_postcondition_variable (cp_expr id)
{
  return make_postcondition_variable (id, make_auto ());
}

/* Check that the TYPE is valid for a named postcondition variable on
   function decl FNDECL. Emit a diagnostic if it is not.  Returns TRUE if
   the result is OK and false otherwise.  */

bool
check_postcondition_result (tree fndecl, tree type, location_t loc)
{
  /* Do not be confused by targetm.cxx.cdtor_return_this ();
     conceptually, cdtors have no return value.  */
  if (VOID_TYPE_P (type)
      || DECL_CONSTRUCTOR_P (fndecl)
      || DECL_DESTRUCTOR_P (fndecl))
    {
      error_at (loc,
		DECL_CONSTRUCTOR_P (fndecl)
		? G_("constructor does not return a value to test")
		: DECL_DESTRUCTOR_P (fndecl)
		? G_("destructor does not return a value to test")
		: G_("function does not return a value to test"));
      return false;
    }

  return true;
}

/* Instantiate each postcondition with the return type to finalize the
   attribute on a function decl FNDECL.  */

void
rebuild_postconditions (tree fndecl)
{
  if (!fndecl || fndecl == error_mark_node)
    return;

  tree type = TREE_TYPE (TREE_TYPE (fndecl));
  tree attributes = flag_contracts_nonattr
		    ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
		    : DECL_CONTRACT_ATTRS (fndecl);

  for (; attributes ; attributes = NEXT_CONTRACT_ATTR (attributes))
    {
      tree contract = TREE_VALUE (TREE_VALUE (attributes));
      if (TREE_CODE (contract) != POSTCONDITION_STMT)
	continue;
      tree condition = CONTRACT_CONDITION (contract);
      if (!condition || condition == error_mark_node)
	continue;

      /* If any conditions are deferred, they're all deferred.  Note that
	 we don't have to instantiate postconditions in that case because
	 the type is available through the declaration.  */
      if (TREE_CODE (condition) == DEFERRED_PARSE)
	return;

      tree oldvar = POSTCONDITION_IDENTIFIER (contract);
      if (!oldvar)
	continue;

      /* Always update the context of the result variable so that it can
	 be remapped by remap_contracts.  */
      DECL_CONTEXT (oldvar) = fndecl;

      /* If the return type is undeduced, defer until later.  */
      if (TREE_CODE (type) == TEMPLATE_TYPE_PARM)
	return;

      /* Check the postcondition variable.  */
      location_t loc = DECL_SOURCE_LOCATION (oldvar);
      if (!check_postcondition_result (fndecl, type, loc))
	{
	  invalidate_contract (contract);
	  continue;
	}

      /* "Instantiate" the result variable using the known type.  Also update
	  the context so the inliner will actually remap this the parameter
	  when generating contract checks.  */
      tree newvar = copy_node (oldvar);
      TREE_TYPE (newvar) = type;

      /* Make parameters and result available for substitution.  */
      local_specialization_stack stack (lss_copy);
      for (tree t = DECL_ARGUMENTS (fndecl); t != NULL_TREE;
	  t = TREE_CHAIN (t))
	register_local_identity (t);

      begin_scope (sk_contract, fndecl);
      bool old_pc = processing_postcondition;
      processing_postcondition = true;
      register_local_specialization (newvar, oldvar);

      condition = tsubst_expr (condition, make_tree_vec (0),
			       tf_warning_or_error, fndecl);

      /* Update the contract condition and result.  */
      POSTCONDITION_IDENTIFIER (contract) = newvar;
      CONTRACT_CONDITION (contract) = finish_contract_condition (condition);
      processing_postcondition = old_pc;
      gcc_checking_assert (scope_chain && scope_chain->bindings
			   && scope_chain->bindings->kind == sk_contract);
      pop_bindings_and_leave_scope ();
    }
}
/* Build a contract statement.  */

tree
grok_contract (tree attribute, tree mode, tree result, cp_expr condition,
	       location_t loc)
{
  if (condition == error_mark_node)
    return error_mark_node;

  tree_code code;
  if (is_attribute_p ("assert", attribute)
      || is_attribute_p ("contract_assert", attribute))
    code = ASSERTION_STMT;
  else if (is_attribute_p ("pre", attribute))
    code = PRECONDITION_STMT;
  else if (is_attribute_p ("post", attribute))
    code = POSTCONDITION_STMT;
  else
    gcc_unreachable ();

  /* Build the contract. The condition is added later.  In the case that
     the contract is deferred, result an plain identifier, not a result
     variable.  */
  tree contract;
  if (code != POSTCONDITION_STMT)
    contract = build5_loc (loc, code, void_type_node, mode,
			   NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);
  else
    {
      contract = build_nt (code, mode, NULL_TREE, NULL_TREE,
			   NULL_TREE, NULL_TREE, result);
      TREE_TYPE (contract) = void_type_node;
      SET_EXPR_LOCATION (contract, loc);
    }

  /* Determine the evaluation semantic:
     First, apply the c++2a rules
     FIXME: this is a convenience to avoid updating many tests.  */
  contract_semantic semantic = compute_concrete_semantic (contract);
  if (flag_contracts_nonattr
      && OPTION_SET_P (flag_contract_evaluation_semantic))
    semantic
      = static_cast<contract_semantic>(flag_contract_evaluation_semantic);
  set_contract_semantic (contract, semantic);

  /* If the contract is deferred, don't do anything with the condition.  */
  if (TREE_CODE (condition) == DEFERRED_PARSE)
    {
      CONTRACT_CONDITION (contract) = condition;
      return contract;
    }

  /* Generate the comment from the original condition.  */
  CONTRACT_COMMENT (contract) = build_comment (condition);

  /* The condition is converted to bool.  */
  condition = finish_contract_condition (condition);

  if (condition == error_mark_node)
    return error_mark_node;

  CONTRACT_CONDITION (contract) = condition;

  return contract;
}

/* Build the contract attribute specifier where IDENTIFIER is one of 'pre',
   'post' or 'assert' and CONTRACT is the underlying statement.  */

tree
finish_contract_attribute (tree identifier, tree contract)
{
  if (contract == error_mark_node)
    return error_mark_node;

  tree attribute = build_tree_list (build_tree_list (NULL_TREE, identifier),
				    build_tree_list (NULL_TREE, contract));

  /* Mark the attribute as dependent if the condition is dependent.  */
  tree condition = CONTRACT_CONDITION (contract);
  if (TREE_CODE (condition) != DEFERRED_PARSE
      && value_dependent_expression_p (condition))
    ATTR_IS_DEPENDENT (attribute) = true;

  return attribute;
}

/* Update condition of a late-parsed contract and postcondition variable,
   if any.  */

void
update_late_contract (tree contract, tree result, cp_expr condition)
{
  if (TREE_CODE (contract) == POSTCONDITION_STMT)
    POSTCONDITION_IDENTIFIER (contract) = result;

  /* Generate the comment from the original condition.  */
  CONTRACT_COMMENT (contract) = build_comment (condition);

  /* The condition is converted to bool.  */
  condition = finish_contract_condition (condition);
  CONTRACT_CONDITION (contract) = condition;
}

/* Deferred contract mapping.

   This is used to compare late-parsed contracts on overrides with their
   base class functions.

   TODO: It seems like this could be replaced by a simple list that maps from
   overrides to their base functions. It's not clear that we really need
   a map to a function + a list of contracts.   */

/* Map from FNDECL to a tree list of contracts that have not been matched or
   diagnosed yet.  The TREE_PURPOSE is the basefn we're overriding, and the
   TREE_VALUE is the list of contract attrs for BASEFN.  */

static hash_map<tree_decl_hash, tree> pending_guarded_decls;

void
defer_guarded_contract_match (tree fndecl, tree fn, tree contracts)
{
  if (!pending_guarded_decls.get (fndecl))
    {
      pending_guarded_decls.put (fndecl, build_tree_list (fn, contracts));
      return;
    }
  for (tree pending = *pending_guarded_decls.get (fndecl);
      pending;
      pending = TREE_CHAIN (pending))
    {
      if (TREE_VALUE (pending) == contracts)
	return;
      if (TREE_CHAIN (pending) == NULL_TREE)
	TREE_CHAIN (pending) = build_tree_list (fn, contracts);
    }
}

/* If the function decl FNDECL has any contracts that had their matching
   deferred earlier, do that checking now.  */

void
match_deferred_contracts (tree fndecl)
{
  tree *tp = pending_guarded_decls.get (fndecl);
  if (!tp)
    return;

  tree attributes = flag_contracts_nonattr
		    ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
		    : DECL_CONTRACT_ATTRS (fndecl);

  gcc_assert(!contract_any_deferred_p (attributes));

  processing_template_decl_sentinel ptds;
  processing_template_decl = uses_template_parms (fndecl);

  /* Do late contract matching.  */
  for (tree pending = *tp; pending; pending = TREE_CHAIN (pending))
    {
      tree new_contracts = TREE_VALUE (pending);
      location_t new_loc = CONTRACT_SOURCE_LOCATION (new_contracts);
      tree old_contracts = flag_contracts_nonattr
		    ? GET_FN_CONTRACT_SPECIFIERS (fndecl)
		    : DECL_CONTRACT_ATTRS (fndecl);
      location_t old_loc = CONTRACT_SOURCE_LOCATION (old_contracts);
      /* todo : this is suspicious : with P2900 we have a TREE_PURPOSE set
	 despite the fact we no longer inherit the contracts from the base.
	 Where is TREE_PURPOSE being set ?
	 It looks like it's set for declarations of friend functions.
	 This means the diagnostic may claim override even in case oF
	 re declarations.  */
      tree base = TREE_PURPOSE (pending);
      match_contract_attributes (new_loc, new_contracts,
				 old_loc, old_contracts,
				 base ? cmc_override : cmc_declaration);
    }

  /* Clear out deferred match list so we don't check it twice.  */
  pending_guarded_decls.remove (fndecl);
}

/* Returns the precondition funtion for FNDECL, or null if not set.  */

tree
get_precondition_function (tree fndecl)
{
  gcc_checking_assert (fndecl);
  tree *result = hash_map_safe_get (decl_pre_fn, fndecl);
  return result ? *result : NULL_TREE;
}

/* Returns the postcondition funtion for FNDECL, or null if not set.  */

tree
get_postcondition_function (tree fndecl)
{
  gcc_checking_assert (fndecl);
  tree *result = hash_map_safe_get (decl_post_fn, fndecl);
  return result ? *result : NULL_TREE;
}

/* Set the PRE and POST functions for FNDECL.  Note that PRE and POST can
   be null in this case. If so the functions are not recorded.  Used by the
   modules code.  */

void
set_contract_functions (tree fndecl, tree pre, tree post)
{
  if (pre)
    set_precondition_function (fndecl, pre);

  if (post)
    set_postcondition_function (fndecl, post);
}


#include "gt-cp-contracts.h"
