// check that we do not crash when capturing a constified entity in a contract assertion lambda
// { dg-do compile }
// { dg-options "-std=c++2b -fcontracts  " }
void f(int i, int j) pre( [i, &j](){ return true;} ( ))
{}
