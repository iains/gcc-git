// { dg-do compile { target c++23 } }
// { dg-additional-options "-fcontracts -fcontracts-disable-constification" }

void good (int x) pre (x++ > 1) {} // should not be an error.
