// Test that there is no crash when using outline checks.
// { dg-do run }
// { dg-options "-std=c++26 -fcontracts -fcontract-checks-outlined" }

#include <utility>

struct S{
  S(){};
  S(const S&){}
  ~S(){};
  int x = 0;
};

struct S2{
  S2(){};
  S2(const S2&){}
  ~S2(){};
  S x;
};

template <typename T>
struct ST{
  ST(){};
  ST(const ST&){}
  ~ST(){};
  T x;
};

template <typename T>
bool check(T t) { return true;}

int f1(int n)   post(r: check(r)) { return n;}

int& f2(int& n)   post(r: check(r)) { return n;}

const int & f3(const int &n)   post(r: check(r)) { return n;}

int&& f4(int &n)   post(r: check(r)) { return std::move(n);}

const int&& f5(int &n)   post(r: check(r)) { return std::move(n);}

int* f6(int *n)   post(r: check(r)) { return n;}

const int* f7(const int *n)   post(r: check(r)) { return n;}

int*& f8(int *&n)   post(r: check(r)) {return n;}

const int*& f9(const int *&n)   post(r: check(r)) {return n;}

S f1(S n)   post(r: check(r)) { return n;}

S& f2(S& n)   post(r: check(r)) { return n;}

const S & f3(const S &n)   post(r: check(r)) { return n;}

S&& f4(S &n)   post(r: check(r)) { return std::move(n);}

const S&& f5(S &n)   post(r: check(r)) { return std::move(n);}

S* f6(S *n)   post(r: check(r)) { return n;}

const S* f7(const S *n)   post(r: check(r)) { return n;}

S*& f8(S *&n)   post(r: check(r)) {return n;}

const S*& f9(const S *&n)   post(r: check(r)) {return n;}

template<typename T>
T f1(T n)   post(r: check(r)) { return n;}

template<typename T>
T& f2(T& n)   post(r: check(r)) { return n;}

template<typename T>
const T & f3(const T &n)   post(r: check(r)) { return n;}

template<typename T>
T&& f4(T &n)   post(r: check(r)) { return std::move(n);}

template<typename T>
const T&& f5(T &n)   post(r: check(r)) { return std::move(n);}

template<typename T>
T* f6(T *n)   post(r: check(r)) { return n;}

template<typename T>
const T* f7(const T *n)   post(r: check(r)) { return n;}

template<typename T>
T*& f8(T *&n)   post(r: check(r)) {return n;}

template<typename T>
const T*& f9(const T *&n)   post(r: check(r)) {return n;}

struct TestStruct{

  int f1(int n)   post(r: check(r)) { return n;}

  int& f2(int& n)   post(r: check(r)) { return n;}

  const int & f3(const int &n)   post(r: check(r)) { return n;}

  int&& f4(int &n)   post(r: check(r)) { return std::move(n);}

  const int&& f5(int &n)   post(r: check(r)) { return std::move(n);}

  int* f6(int *n)   post(r: check(r)) { return n;}

  const int* f7(const int *n)   post(r: check(r)) { return n;}

  int*& f8(int *&n)   post(r: check(r)) {return n;}

  const int*& f9(const int *&n)   post(r: check(r)) {return n;}

  S f1(S n)   post(r: check(r)) { return n;}

  S& f2(S& n)   post(r: check(r)) { return n;}

  const S & f3(const S &n)   post(r: check(r)) { return n;}

  S&& f4(S &n)   post(r: check(r)) { return std::move(n);}

  const S&& f5(S &n)   post(r: check(r)) { return std::move(n);}

  S* f6(S *n)   post(r: check(r)) { return n;}

  const S* f7(const S *n)   post(r: check(r)) { return n;}

  S*& f8(S *&n)   post(r: check(r)) {return n;}

  const S*& f9(const S *&n)   post(r: check(r)) {return n;}

  template<typename T>
  T f1(T n)   post(r: check(r)) { return n;}

  template<typename T>
  T& f2(T& n)   post(r: check(r)) { return n;}

  template<typename T>
  const T & f3(const T &n)   post(r: check(r)) { return n;}

  template<typename T>
  T&& f4(T &n)   post(r: check(r)) { return std::move(n);}

  template<typename T>
  const T&& f5(T &n)   post(r: check(r)) { return std::move(n);}

  template<typename T>
  T* f6(T *n)   post(r: check(r)) { return n;}

  template<typename T>
  const T* f7(const T *n)   post(r: check(r)) { return n;}

  template<typename T>
  T*& f8(T *&n)   post(r: check(r)) {return n;}

  template<typename T>
  const T*& f9(const T *&n)   post(r: check(r)) {return n;}

};


int main()
{
  int i = 1;
  int* pi;
  const int* cpi;

  S s;
  S* ps;
  const S* cps;

  S2 s2;
  S2* ps2;
  const S2* cps2;

  ST<int> st;
  ST<int>* pst;
  const ST<int>* cpst;

  f1(i);
  f2(i);
  f3(i);
  f4(i);
  f5(i);
  f6(pi);
  f7(cpi);
  f8(pi);
  f9(cpi);


  f1(s);
  f2(s);
  f3(s);
  f4(s);
  f5(s);
  f6(ps);
  f7(cps);
  f8(ps);
  f9(cps);

  f1(s2);
  f2(s2);
  f3(s2);
  f4(s2);
  f5(s2);
  f6(ps2);
  f7(cps2);
  f8(ps2);
  f9(cps2);

  f1(st);
  f2(st);
  f3(st);
  f4(st);
  f5(st);
  f6(pst);
  f7(cpst);
  f8(pst);
  f9(cpst);

  TestStruct b;
  b.f1(i);
  b.f2(i);
  b.f3(i);
  b.f4(i);
  b.f5(i);
  b.f6(pi);
  b.f7(cpi);
  b.f8(pi);
  b.f9(cpi);


  b.f1(s);
  b.f2(s);
  b.f3(s);
  b.f4(s);
  b.f5(s);
  b.f6(ps);
  b.f7(cps);
  b.f8(ps);
  b.f9(cps);

  b.f1(s2);
  b.f2(s2);
  b.f3(s2);
  b.f4(s2);
  b.f5(s2);
  b.f6(ps2);
  b.f7(cps2);
  b.f8(ps2);
  b.f9(cps2);

  b.f1(st);
  b.f2(st);
  b.f3(st);
  b.f4(st);
  b.f5(st);
  b.f6(pst);
  b.f7(cpst);
  b.f8(pst);
  b.f9(cpst);

}
