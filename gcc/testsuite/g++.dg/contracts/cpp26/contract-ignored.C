// { dg-do run }
// { dg-options "-std=c++26 -fcontracts -fcontract-evaluation-semantic=ignore" }

#include <contracts>

int f(int i, int j = 1)
  pre (i > 0)
  post (r: r < 5)
{
  contract_assert ( j > 0);
  return i;
}

int main(int, char**)
{
  f (0,0);
}

void
handle_contract_violation (const std::contracts::contract_violation &)
{
  __builtin_abort ();
}
