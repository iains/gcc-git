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
  friend bool operator==(Base, Base) pre (true) = default; // { dg-error "contracts cannot be added" }

};
struct Child : Base
{
  int f2() pre(true); // { dg-error "contracts cannot be added to virtual functions" }
  friend bool operator==(Child, Child) post (true) = default; // { dg-error "contracts cannot be added" }

};

template<typename T>
struct ST
{
  virtual int f1() pre(true); // { dg-error "contracts cannot be added to virtual functions" }
  virtual int f1(T) pre(true); // { dg-error "contracts cannot be added to virtual functions" }
  virtual int f2();
  int f3() pre (true) = delete ; // { dg-error "contracts cannot be added" }
  ST() pre (true) = default; // { dg-error "contracts cannot be added" }
  ST(const ST &) post (true) = default; // { dg-error "contracts cannot be added" }
  ST(ST &&) pre (true) = default; // { dg-error "contracts cannot be added" }
  ST(T) pre (true) = delete; // { dg-error "contracts cannot be added" }

  friend bool operator==(ST, ST) pre (true) = default; // { dg-error "contracts cannot be added" }
};

ST<int> st;

int freeFunc1()  pre(true) = delete; // { dg-error "contracts cannot be added" }
int freeFunc2()  post(true) = delete; // { dg-error "contracts cannot be added" }
template<typename T>
int freeFunc3(T)  post(true) = delete; // { dg-error "contracts cannot be added" }


