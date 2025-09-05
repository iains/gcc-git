// { dg-do run }
// { dg-options "-std=c++2a -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=observe -fcontracts-disable-predicate-exception-translation" }

#include <cassert>

bool check(int i){
  if (i > 10)
    throw 3;

  return true;
}

void f() pre(check(15)){}


int main(int, char**)
{
  try {
    f();
  } catch (const int& x) {
    assert(x == 3);
  }
}
