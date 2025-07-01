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

// Check that a case when neither ASSERT_USES_CONTRACTS nor NDEBUG are defined
// behaves correctly (i.e. falls back to the libc assert).
// Semantic chosen is a non terminating one.
// { dg-options "-g0 -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=observe" }
// { dg-do run { target c++2a } }

#include <exception>
#include <cstdlib>
#include <cassert>
#include <csignal>

void my_term(int)
{
  std::exit(0);
}

int main()
{
  signal(SIGABRT, my_term);

  int i = 3;
  assert(i == 4);
  // We should not get here
  return 1;
}

// Since we are now seeing the output of the libc 'assert' macro, this will
// (in general) depend on the OS/libc being tested.

// { dg-output "int main.*: Assertion .*i == 4.* failed.*" { target { ! *-*-darwin* } }  }
// { dg-output "Assertion failed: .i == 4., function main," { target *-*-darwin*  }  }
