// check that we do not ICE with an empty nontrivial parameter
// { dg-do compile }
// { dg-options "-std=c++23 -fcontracts " }
struct NTClass {
  NTClass(){};
  ~NTClass(){};
};

struct Empty {};

struct PostCond {
  void f (const NTClass i)
    post ( true);

  void g (const Empty i)
    post ( true);

};

void
PostCond::f (NTClass i){}

void
PostCond::g (Empty i){}
