// Test that there is no ICE with outlined contracts on a virtual function with Nontrivial types
// in precondition checks
// { dg-do compile { target c++23 } }
// { dg-additional-options "-fcontracts -fcontracts-on-virtual-functions=P2900R13" }
struct NonTrivial{
  NonTrivial(){};
  NonTrivial(const NonTrivial&){}
  ~NonTrivial(){};
  int x = 0;
};

struct S
{

  virtual void f(const NonTrivial s) pre(s.x >0 );

};

void S::f(const NonTrivial g) pre(g.x >0 ){};

int main()
{
  NonTrivial nt;
  S s;
  s.f(nt);
}
