// N5008:
// dcl.contract.func/p6
// A virtual function (11.7.3),
// { dg-do compile { target c++23 } }
// { dg-additional-options "-fcontracts" }

struct Base
{
  virtual int f1() pre(true); // { dg-error "contracts cannot be added to virtual functions" }
  virtual int f2();
  int f3() pre (true) = delete ; // { dg-error "contracts cannot be added" }
  Base() pre (true) = default; // { dg-error "contracts cannot be added" }
  Base(const Base &) post (true) = default; // { dg-error "contracts cannot be added" }
  Base(Base &&) pre (true) = default; // { dg-error "contracts cannot be added" }

};
struct Child : Base
{
  int f2() pre(true); // { dg-error "contracts cannot be added to virtual functions" }

};


