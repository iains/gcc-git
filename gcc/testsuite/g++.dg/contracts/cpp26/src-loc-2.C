// { dg-do run }
// { dg-options "-std=c++26 -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=observe " }

// Test source location with a late inclusion of <source_location>

int
foo (int x)
  pre ( x > 10 )
{
  return x - 9;
}

#include <source_location>

int main ()
{
  foo (9);
}

// { dg-output "contract violation in function int foo.int. at .*:8: x > 10.*(\n|\r\n|\r)" }
