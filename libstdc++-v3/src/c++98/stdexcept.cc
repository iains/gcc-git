// Methods for Exception Support for -*- C++ -*-

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
// ISO C++ 14882: 19.1  Exception classes
//

// All exception classes still use the classic COW std::string.
#define _GLIBCXX_USE_CXX11_ABI 0
#include <string>
#include <stdexcept>

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

#if ! _GLIBCXX_USE_CXX11_ABI
  logic_error::logic_error(const string& __arg)
  : exception(), _M_msg(__arg) { }
#endif

  logic_error::~logic_error() _GLIBCXX_USE_NOEXCEPT { }

  const char*
  logic_error::what() const _GLIBCXX_USE_NOEXCEPT
  { return _M_msg.c_str(); }

#if ! _GLIBCXX_USE_CXX11_ABI
  domain_error::domain_error(const string& __arg)
  : logic_error(__arg) { }
#endif

  domain_error::~domain_error() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  invalid_argument::invalid_argument(const string& __arg)
  : logic_error(__arg) { }
#endif

  invalid_argument::~invalid_argument() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  length_error::length_error(const string& __arg)
  : logic_error(__arg) { }
#endif

  length_error::~length_error() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  out_of_range::out_of_range(const string& __arg)
  : logic_error(__arg) { }
#endif

  out_of_range::~out_of_range() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  runtime_error::runtime_error(const string& __arg)
  : exception(), _M_msg(__arg) { }
#endif

  runtime_error::~runtime_error() _GLIBCXX_USE_NOEXCEPT { }

  const char*
  runtime_error::what() const _GLIBCXX_USE_NOEXCEPT
  { return _M_msg.c_str(); }

#if ! _GLIBCXX_USE_CXX11_ABI
  range_error::range_error(const string& __arg)
  : runtime_error(__arg) { }
#endif

  range_error::~range_error() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  overflow_error::overflow_error(const string& __arg)
  : runtime_error(__arg) { }
#endif

  overflow_error::~overflow_error() _GLIBCXX_USE_NOEXCEPT { }

#if ! _GLIBCXX_USE_CXX11_ABI
  underflow_error::underflow_error(const string& __arg)
  : runtime_error(__arg) { }
#endif

  underflow_error::~underflow_error() _GLIBCXX_USE_NOEXCEPT { }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace
