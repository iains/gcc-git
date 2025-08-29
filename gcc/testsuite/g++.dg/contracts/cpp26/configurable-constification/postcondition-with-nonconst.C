// { dg-do compile { target c++23 } }
// { dg-options "-fcontracts -fcontracts-disable-constification" }

// This should compile without error.

class X {

static int calc ()
  post (alignment: 0 == (alignment & (alignment-1)));

};

