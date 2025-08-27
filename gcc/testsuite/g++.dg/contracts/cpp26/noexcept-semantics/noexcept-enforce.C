// Test that there is no crash with default contract violation handler and noexcept_enforce
// { dg-do compile { target c++26 } }
// { dg-additional-options "-fcontracts -fcontract-evaluation-semantic=noexcept_enforce " }


void f(int i) pre(i > 0){}

int main()
{
  f(0);
}
