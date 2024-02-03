// Components for manipulating sequences of characters -*- C++ -*-

// Copyright (C) 1997-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

//
// ISO C++ 14882: 21  Strings library
//

// Written by Jason Merrill based upon the specification by Takanori Adachi
// in ANSI X3J16/94-0013R2.  Rewritten by Nathan Myers.

#ifndef _GLIBCXX_USE_CXX11_ABI
// Instantiations in this file use the new SSO std::string ABI unless included
// by another file which defines _GLIBCXX_USE_CXX11_ABI=0.
# define _GLIBCXX_USE_CXX11_ABI 1
#endif

#if _GLIBCXX_USE_CXX11_ABI
# define _GLIBCXX_DEFINING_CXX11_ABI_INSTANTIATIONS 1
#else
# define _GLIBCXX_DEFINING_CXX11_ABI_INSTANTIATIONS 0
#endif

// Prevent the basic_string(const _CharT*, const _Alloc&) and
// basic_string(size_type, _CharT, const _Alloc&) constructors from being
// replaced by constrained function templates, so that we instantiate the
// pre-C++17 definitions.
// This also causes the instantiation of the non-standard C++0x-era
// insert(iterator, initializer_list<C>) overload, see PR libstdc++/83328
#define _GLIBCXX_DEFINING_STRING_INSTANTIATIONS 1

#include <string>

#if _GLIBCXX_USE_DUAL_ABI || _GLIBCXX_USE_CXX11_ABI
#include <stdexcept>

#if _GLIBCXX_USE_CXX11_ABI
# include <bits/cow_string.h>
typedef std::__std_cow_string<char, std::char_traits<char>,
			      std::allocator<char>> cowstr;
#else
typedef std::string cowstr;
#endif

static_assert(sizeof(std::__cow_string) == sizeof(cowstr),
	      "sizeof(std::string) has changed");
static_assert(alignof(std::__cow_string) == alignof(cowstr),
	      "alignof(std::string) has changed");
#endif // _GLIBCXX_USE_DUAL_ABI || _GLIBCXX_USE_CXX11_ABI

#if _GLIBCXX_USE_CXX11_ABI && ! _GLIBCXX_DEFINING_CXX11_ABI_INSTANTIATIONS
# define _GLIBCXX_DEFINING_COW_STRING_INSTANTIATIONS 1
# include <bits/cow_string.h>
# define basic_string __std_cow_string
#endif

// Instantiation configuration.
#ifndef C
# define C char
#endif

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

typedef basic_string<C, std::char_traits<C>, std::allocator<C>> S;

  template class basic_string<C, std::char_traits<C>, std::allocator<C>>;
#if ! _GLIBCXX_DEFINING_COW_STRING_INSTANTIATIONS
  template S operator+(const C*, const S&);
  template S operator+(C, const S&);
  template S operator+(const S&, const S&);
#endif

  // Only one template keyword allowed here.
  // See core issue #46 (NAD)
  // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_closed.html#46
  template
    S::basic_string(C*, C*, const allocator<C>&);

  template
    S::basic_string(const C*, const C*, const allocator<C>&);

  template
    S::basic_string(S::iterator, S::iterator, const allocator<C>&);

#if _GLIBCXX_USE_CXX11_ABI && ! _GLIBCXX_DEFINING_COW_STRING_INSTANTIATIONS
  template
    void
    S::_M_construct(S::iterator, S::iterator, forward_iterator_tag);

  template
    void
    S::_M_construct(S::const_iterator, S::const_iterator,
		    forward_iterator_tag);

  template
    void
    S::_M_construct(C*, C*, forward_iterator_tag);

  template
    void
    S::_M_construct(const C*, const C*, forward_iterator_tag);

#else // ! _GLIBCXX_USE_CXX11_ABI || _GLIBCXX_DEFINING_COW_STRING_INSTANTIATIONS

  template
    C*
    S::_S_construct(S::iterator, S::iterator,
		    const allocator<C>&, forward_iterator_tag);

  template
    C*
    S::_S_construct(C*, C*, const allocator<C>&, forward_iterator_tag);

  template
    C*
    S::_S_construct(const C*, const C*, const allocator<C>&,
		    forward_iterator_tag);
#endif

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#if ! _GLIBCXX_DEFINING_COW_STRING_INSTANTIATIONS
namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  using std::S;
  template bool operator==(const S::iterator&, const S::iterator&);
  template bool operator==(const S::const_iterator&, const S::const_iterator&);

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
#endif
