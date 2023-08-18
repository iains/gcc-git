//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __CXXABI_H
#define __CXXABI_H

/*
 * This header provides the interface to the C++ ABI as defined at:
 *       https://itanium-cxx-abi.github.io/cxx-abi/
 */

#include <stddef.h>
#include <stdint.h>

#include <__cxxabi_config.h>
#include <bits/cxxabi_tweaks.h>

#define _LIBCPPABI_VERSION 15000
#define _LIBCXXABI_NORETURN  __attribute__((noreturn))
#define _LIBCXXABI_ALWAYS_COLD __attribute__((cold))

#ifdef __cplusplus

#ifdef NEED_PRIVATE_TYPEINFO_API
#include <typeinfo>
#else
namespace std {
class type_info; // forward declaration
}
#endif

// runtime routines use C calling conventions, but are in __cxxabiv1 namespace
namespace __cxxabiv1 {
extern "C"  {

// libsupc++ compat typedef.  NOTE: we do not DTRT for Arm EABI since the
// c++abi ctor cases always return void, but Arm EABI should return *this.

  typedef __cxa_cdtor_return_type (*__cxa_cdtor_type)(void *);

// 2.4.2 Allocating the Exception Object
extern _LIBCXXABI_FUNC_VIS void *
__cxa_allocate_exception(size_t thrown_size) throw();
extern _LIBCXXABI_FUNC_VIS void
__cxa_free_exception(void *thrown_exception) throw();

// 2.4.3 Throwing the Exception Object
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void
__cxa_throw(void *thrown_exception, std::type_info *tinfo,
            void (_LIBCXXABI_DTOR_FUNC *dest)(void *));

// 2.5.3 Exception Handlers
extern _LIBCXXABI_FUNC_VIS void *
__cxa_get_exception_ptr(void *exceptionObject) throw();
extern _LIBCXXABI_FUNC_VIS void *
__cxa_begin_catch(void *exceptionObject) throw();
extern _LIBCXXABI_FUNC_VIS void __cxa_end_catch();
#if defined(_LIBCXXABI_ARM_EHABI)
extern _LIBCXXABI_FUNC_VIS bool
__cxa_begin_cleanup(void *exceptionObject) throw();
extern _LIBCXXABI_FUNC_VIS void __cxa_end_cleanup();
#endif
extern _LIBCXXABI_FUNC_VIS std::type_info *__cxa_current_exception_type();

// 2.5.4 Rethrowing Exceptions
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_rethrow();

// 2.6 Auxiliary Runtime APIs
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_bad_cast(void);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_bad_typeid(void);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void
__cxa_throw_bad_array_new_length(void);

// 3.2.6 Pure Virtual Function API
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_pure_virtual(void);

// 3.2.7 Deleted Virtual Function API
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_NORETURN void __cxa_deleted_virtual(void);

// 3.3.2 One-time Construction API
#if defined(_LIBCXXABI_GUARD_ABI_ARM)
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD int __cxa_guard_acquire(uint32_t *);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD void __cxa_guard_release(uint32_t *);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD void __cxa_guard_abort(uint32_t *);
#else
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD int __cxa_guard_acquire(uint64_t *);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD void __cxa_guard_release(uint64_t *);
extern _LIBCXXABI_FUNC_VIS _LIBCXXABI_ALWAYS_COLD void __cxa_guard_abort(uint64_t *);
#endif

// 3.3.3 Array Construction and Destruction API
extern _LIBCXXABI_FUNC_VIS void *
__cxa_vec_new(size_t element_count, size_t element_size, size_t padding_size,
              void (*constructor)(void *), void (*destructor)(void *));

extern _LIBCXXABI_FUNC_VIS void *
__cxa_vec_new2(size_t element_count, size_t element_size, size_t padding_size,
               void (*constructor)(void *), void (*destructor)(void *),
               void *(*alloc)(size_t), void (*dealloc)(void *));

extern _LIBCXXABI_FUNC_VIS void *
__cxa_vec_new3(size_t element_count, size_t element_size, size_t padding_size,
               void (*constructor)(void *), void (*destructor)(void *),
               void *(*alloc)(size_t), void (*dealloc)(void *, size_t));

extern _LIBCXXABI_FUNC_VIS void
__cxa_vec_ctor(void *array_address, size_t element_count, size_t element_size,
               void (*constructor)(void *), void (*destructor)(void *));

extern _LIBCXXABI_FUNC_VIS void __cxa_vec_dtor(void *array_address,
                                               size_t element_count,
                                               size_t element_size,
                                               void (*destructor)(void *));

extern _LIBCXXABI_FUNC_VIS void __cxa_vec_cleanup(void *array_address,
                                                  size_t element_count,
                                                  size_t element_size,
                                                  void (*destructor)(void *));

extern _LIBCXXABI_FUNC_VIS void __cxa_vec_delete(void *array_address,
                                                 size_t element_size,
                                                 size_t padding_size,
                                                 void (*destructor)(void *));

extern _LIBCXXABI_FUNC_VIS void
__cxa_vec_delete2(void *array_address, size_t element_size, size_t padding_size,
                  void (*destructor)(void *), void (*dealloc)(void *));

extern _LIBCXXABI_FUNC_VIS void
__cxa_vec_delete3(void *__array_address, size_t element_size,
                  size_t padding_size, void (*destructor)(void *),
                  void (*dealloc)(void *, size_t));

extern _LIBCXXABI_FUNC_VIS void
__cxa_vec_cctor(void *dest_array, void *src_array, size_t element_count,
                size_t element_size, void (*constructor)(void *, void *),
                void (*destructor)(void *));

// 3.3.5.3 Runtime API
// These functions are part of the C++ ABI, but they are not defined in libc++abi:
//    int __cxa_atexit(void (*)(void *), void *, void *);
//    void __cxa_finalize(void *);

// 3.4 Demangler API
extern _LIBCXXABI_FUNC_VIS char *__cxa_demangle(const char *mangled_name,
                                                char *output_buffer,
                                                size_t *length, int *status);

// Apple additions to support C++ 0x exception_ptr class
// These are primitives to wrap a smart pointer around an exception object
extern _LIBCXXABI_FUNC_VIS void *__cxa_current_primary_exception() throw();
extern _LIBCXXABI_FUNC_VIS void
__cxa_rethrow_primary_exception(void *primary_exception);
extern _LIBCXXABI_FUNC_VIS void
__cxa_increment_exception_refcount(void *primary_exception) throw();
extern _LIBCXXABI_FUNC_VIS void
__cxa_decrement_exception_refcount(void *primary_exception) throw();

// Apple extension to support std::uncaught_exception()
extern _LIBCXXABI_FUNC_VIS bool __cxa_uncaught_exception() throw();
extern _LIBCXXABI_FUNC_VIS unsigned int __cxa_uncaught_exceptions() throw();

#if defined(__linux__) || defined(__Fuchsia__)
// Linux and Fuchsia TLS support. Not yet an official part of the Itanium ABI.
// https://sourceware.org/glibc/wiki/Destructor%20support%20for%20thread_local%20variables
extern _LIBCXXABI_FUNC_VIS int __cxa_thread_atexit(void (*)(void *), void *,
                                                   void *) throw();
#endif

} // extern "C"

/* NOTE: we can expose the API in this header, but the actual functions are
   not exported from libc++abi, so the use of this is limited (but needed for
   two of our sources to compile.  */
#ifdef NEED_PRIVATE_TYPEINFO_API

class _LIBCXXABI_TYPE_VIS __shim_type_info : public std::type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__shim_type_info();

  _LIBCXXABI_HIDDEN virtual void noop1() const;
  _LIBCXXABI_HIDDEN virtual void noop2() const;
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *thrown_type,
                                           void *&adjustedPtr) const = 0;
};

class _LIBCXXABI_TYPE_VIS __fundamental_type_info : public __shim_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__fundamental_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
};

class _LIBCXXABI_TYPE_VIS __array_type_info : public __shim_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__array_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
};

class _LIBCXXABI_TYPE_VIS __function_type_info : public __shim_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__function_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
};

class _LIBCXXABI_TYPE_VIS __enum_type_info : public __shim_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__enum_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
};

enum
{
    unknown = 0,
    public_path,
    not_public_path,
    yes,
    no
};

class _LIBCXXABI_TYPE_VIS __class_type_info;

struct _LIBCXXABI_HIDDEN __dynamic_cast_info
{
// const data supplied to the search:

    const __class_type_info* dst_type;
    const void* static_ptr;
    const __class_type_info* static_type;
    ptrdiff_t src2dst_offset;

// Data that represents the answer:

    // pointer to a dst_type which has (static_ptr, static_type) above it
    const void* dst_ptr_leading_to_static_ptr;
    // pointer to a dst_type which does not have (static_ptr, static_type) above it
    const void* dst_ptr_not_leading_to_static_ptr;

    // The following three paths are either unknown, public_path or not_public_path.
    // access of path from dst_ptr_leading_to_static_ptr to (static_ptr, static_type)
    int path_dst_ptr_to_static_ptr;
    // access of path from (dynamic_ptr, dynamic_type) to (static_ptr, static_type)
    //    when there is no dst_type along the path
    int path_dynamic_ptr_to_static_ptr;
    // access of path from (dynamic_ptr, dynamic_type) to dst_type
    //    (not used if there is a (static_ptr, static_type) above a dst_type).
    int path_dynamic_ptr_to_dst_ptr;

    // Number of dst_types below (static_ptr, static_type)
    int number_to_static_ptr;
    // Number of dst_types not below (static_ptr, static_type)
    int number_to_dst_ptr;

// Data that helps stop the search before the entire tree is searched:

    // is_dst_type_derived_from_static_type is either unknown, yes or no.
    int is_dst_type_derived_from_static_type;
    // Number of dst_type in tree.  If 0, then that means unknown.
    int number_of_dst_type;
    // communicates to a dst_type node that (static_ptr, static_type) was found
    //    above it.
    bool found_our_static_ptr;
    // communicates to a dst_type node that a static_type was found
    //    above it, but it wasn't (static_ptr, static_type)
    bool found_any_static_type;
    // Set whenever a search can be stopped
    bool search_done;
};

// Has no base class
class _LIBCXXABI_TYPE_VIS __class_type_info : public __shim_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__class_type_info();

  _LIBCXXABI_HIDDEN void process_static_type_above_dst(__dynamic_cast_info *,
                                                       const void *,
                                                       const void *, int) const;
  _LIBCXXABI_HIDDEN void process_static_type_below_dst(__dynamic_cast_info *,
                                                       const void *, int) const;
  _LIBCXXABI_HIDDEN void process_found_base_class(__dynamic_cast_info *, void *,
                                                  int) const;
  _LIBCXXABI_HIDDEN virtual void search_above_dst(__dynamic_cast_info *,
                                                  const void *, const void *,
                                                  int, bool) const;
  _LIBCXXABI_HIDDEN virtual void
  search_below_dst(__dynamic_cast_info *, const void *, int, bool) const;
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
  _LIBCXXABI_HIDDEN virtual void
  has_unambiguous_public_base(__dynamic_cast_info *, void *, int) const;
};

// Has one non-virtual public base class at offset zero
class _LIBCXXABI_TYPE_VIS __si_class_type_info : public __class_type_info {
public:
  const __class_type_info *__base_type;

  _LIBCXXABI_HIDDEN virtual ~__si_class_type_info();

  _LIBCXXABI_HIDDEN virtual void search_above_dst(__dynamic_cast_info *,
                                                  const void *, const void *,
                                                  int, bool) const;
  _LIBCXXABI_HIDDEN virtual void
  search_below_dst(__dynamic_cast_info *, const void *, int, bool) const;
  _LIBCXXABI_HIDDEN virtual void
  has_unambiguous_public_base(__dynamic_cast_info *, void *, int) const;
};

struct _LIBCXXABI_HIDDEN __base_class_type_info
{
public:
    const __class_type_info* __base_type;
    long __offset_flags;

    enum __offset_flags_masks
    {
        __virtual_mask = 0x1,
        __public_mask  = 0x2, // base is public
        __offset_shift = 8
    };

    void search_above_dst(__dynamic_cast_info*, const void*, const void*, int, bool) const;
    void search_below_dst(__dynamic_cast_info*, const void*, int, bool) const;
    void has_unambiguous_public_base(__dynamic_cast_info*, void*, int) const;
};

// Has one or more base classes
class _LIBCXXABI_TYPE_VIS __vmi_class_type_info : public __class_type_info {
public:
  unsigned int __flags;
  unsigned int __base_count;
  __base_class_type_info __base_info[1];

  enum __flags_masks {
    __non_diamond_repeat_mask = 0x1, // has two or more distinct base class
                                     //    objects of the same type
    __diamond_shaped_mask = 0x2      // has base class object with two or
                                     //    more derived objects
  };

  _LIBCXXABI_HIDDEN virtual ~__vmi_class_type_info();

  _LIBCXXABI_HIDDEN virtual void search_above_dst(__dynamic_cast_info *,
                                                  const void *, const void *,
                                                  int, bool) const;
  _LIBCXXABI_HIDDEN virtual void
  search_below_dst(__dynamic_cast_info *, const void *, int, bool) const;
  _LIBCXXABI_HIDDEN virtual void
  has_unambiguous_public_base(__dynamic_cast_info *, void *, int) const;
};

class _LIBCXXABI_TYPE_VIS __pbase_type_info : public __shim_type_info {
public:
  unsigned int __flags;
  const __shim_type_info *__pointee;

  enum __masks {
    __const_mask = 0x1,
    __volatile_mask = 0x2,
    __restrict_mask = 0x4,
    __incomplete_mask = 0x8,
    __incomplete_class_mask = 0x10,
    __transaction_safe_mask = 0x20,
    // This implements the following proposal from cxx-abi-dev (not yet part of
    // the ABI document):
    //
    //   http://sourcerytools.com/pipermail/cxx-abi-dev/2016-October/002986.html
    //
    // This is necessary for support of http://wg21.link/p0012, which permits
    // throwing noexcept function and member function pointers and catching
    // them as non-noexcept pointers.
    __noexcept_mask = 0x40,

    // Flags that cannot be removed by a standard conversion.
    __no_remove_flags_mask = __const_mask | __volatile_mask | __restrict_mask,
    // Flags that cannot be added by a standard conversion.
    __no_add_flags_mask = __transaction_safe_mask | __noexcept_mask
  };

  _LIBCXXABI_HIDDEN virtual ~__pbase_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
};

class _LIBCXXABI_TYPE_VIS __pointer_type_info : public __pbase_type_info {
public:
  _LIBCXXABI_HIDDEN virtual ~__pointer_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
  _LIBCXXABI_HIDDEN bool can_catch_nested(const __shim_type_info *) const;
};

class _LIBCXXABI_TYPE_VIS __pointer_to_member_type_info
    : public __pbase_type_info {
public:
  const __class_type_info *__context;

  _LIBCXXABI_HIDDEN virtual ~__pointer_to_member_type_info();
  _LIBCXXABI_HIDDEN virtual bool can_catch(const __shim_type_info *,
                                           void *&) const;
  _LIBCXXABI_HIDDEN bool can_catch_nested(const __shim_type_info *) const;
};

#endif // NEED_PRIVATE_TYPEINFO_API

  // A magic placeholder class that can be caught by reference
  // to recognize foreign exceptions.
  class __foreign_exception
  {
    virtual ~__foreign_exception() throw();
    virtual void __pure_dummy() = 0; // prevent catch by value
  };

} // namespace __cxxabiv1

namespace abi = __cxxabiv1;

#endif // __cplusplus

#endif // __CXXABI_H
