// Copyright (C) 2013-2025 Free Software Foundation, Inc.
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

// { dg-do run { target c++11 } }

#include <unordered_set>
#include <testsuite_hooks.h>
#include <testsuite_allocator.h>
#include <testsuite_counter_type.h>

using __gnu_test::propagating_allocator;
using __gnu_test::counter_type;

void test01()
{
  typedef propagating_allocator<counter_type, false> alloc_type;
  typedef __gnu_test::counter_type_hasher hash;
  typedef std::unordered_multiset<counter_type, hash,
				  std::equal_to<counter_type>,
				  alloc_type> test_type;

  test_type v1(alloc_type(1));
  v1.emplace(0);

  test_type v2(alloc_type(2));
  v2.emplace(1);

  counter_type::reset();

  v2 = std::move(v1);

  VERIFY( 1 == v1.get_allocator().get_personality() );
  VERIFY( 2 == v2.get_allocator().get_personality() );

  VERIFY( counter_type::move_count == 0  );
  // 1 element in v1 destroyed.
  VERIFY( counter_type::destructor_count == 1 );
}

void test02()
{
  typedef propagating_allocator<counter_type, true> alloc_type;
  typedef __gnu_test::counter_type_hasher hash;
  typedef std::unordered_multiset<counter_type, hash,
				  std::equal_to<counter_type>,
				  alloc_type> test_type;

  test_type v1(alloc_type(1));
  v1.emplace(0);

  auto it = v1.begin();

  test_type v2(alloc_type(2));
  v2.emplace(0);

  counter_type::reset();

  v2 = std::move(v1);

  VERIFY(1 == v1.get_allocator().get_personality());
  VERIFY(1 == v2.get_allocator().get_personality());

  VERIFY( counter_type::move_count == 0 );
  VERIFY( counter_type::copy_count == 0 );
  VERIFY( counter_type::destructor_count == 1 );

  VERIFY( it == v2.begin() );
}

int main()
{
  test01();
  test02();
  return 0;
}
