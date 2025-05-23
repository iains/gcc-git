// { dg-do compile { target c++23 } }

// Copyright (C) 2021-2025 Free Software Foundation, Inc.
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

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// basic_string_view contains

#include <string_view>

void
test01()
{
    constexpr std::wstring_view haystack = L"no place for needles";

    static_assert(haystack.contains(std::wstring_view(L"")));
    static_assert(haystack.contains(std::wstring_view(L"no")));
    static_assert(haystack.contains(std::wstring_view(L"needles")));
    static_assert(haystack.contains(std::wstring_view(L" for ")));
    static_assert(!haystack.contains(std::wstring_view(L"places")));

    static_assert(!haystack.contains('\0'));
    static_assert(haystack.contains('n'));
    static_assert(haystack.contains('e'));
    static_assert(haystack.contains('s'));
    static_assert(!haystack.contains('x'));

    static_assert(haystack.contains(L""));
    static_assert(haystack.contains(L"no"));
    static_assert(haystack.contains(L"needles"));
    static_assert(haystack.contains(L" for "));
    static_assert(!haystack.contains(L"places"));

    constexpr std::wstring_view nothing;
    static_assert(nothing.contains(L""));
    static_assert(!nothing.contains('\0'));
}
