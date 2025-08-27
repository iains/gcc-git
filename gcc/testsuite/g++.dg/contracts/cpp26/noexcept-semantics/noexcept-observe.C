// Test that there is no crash with default contract violation handler and noexcept_observe
// { dg-do compile { target c++26 } }
// { dg-additional-options "-fcontracts -fcontract-evaluation-semantic=noexcept_observe " }


void f(int i) pre(i > 0){}

int main()
{
  f(0);
}

