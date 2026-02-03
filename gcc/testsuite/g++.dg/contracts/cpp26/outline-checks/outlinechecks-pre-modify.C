// Test that an argument used in a postcondition can be modified in the
// presence of outline checks.
// { dg-do run }
// { dg-options "-std=c++26 -fcontracts -fcontract-checks-outlined" }

#include <cassert>
#include <utility>

struct NonTrivial{
  NonTrivial(int i): x(i) {};
  NonTrivial(const NonTrivial&){}
  ~NonTrivial(){};
  bool operator==(const int& n) const { return x==n;}
  int x = 0;
};

struct NonMoveable{
  NonMoveable(int i): x(i){};
  NonMoveable(NonMoveable&&) = delete;
  bool operator==(const int& n) const { return x==n;}
  int x = 0;
};

bool mutate(const int & t) {
  const_cast<int &>(t) = 42;
  return true;
}

bool mutate(const NonTrivial & t) {
  const_cast<NonTrivial &>(t).x = 42;
  return true;
}

bool mutate(const NonMoveable & t) {
  const_cast<NonMoveable &>(t).x = 42;
  return true;
}


void f1(int &n)   pre(mutate(n)) { assert (n ==42);}

void f2(int n)   pre(mutate(n)) { assert (n ==42);}

void f4(const int &n)   pre(mutate(n)) { assert (n ==42);}

void f9(int &&n) pre (mutate(n)) { assert (n ==42);};

void f10(const int &&n) pre (mutate(n)) { assert (n ==42);};

void f1(NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}

void f4(const NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}

void f9(NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}

void f10(const NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}


template<typename T>
void f1(T &n)   pre(mutate(n)) { assert (n ==42);}

template<typename T>
void f2(T n)   pre(mutate(n)) { assert (n ==42);}

template<typename T>
void f3(const T n)   pre(mutate(n)) { assert (n ==42);}

template<typename T>
void f4(const T &n)   pre(mutate(n)) { assert (n ==42);}

template<typename T>
void f9(T &&n)   pre(mutate(n)) { assert (n ==42);}

template<typename T>
void f10(const T &&n)   pre(mutate(n)) { assert (n ==42);}

struct TestStruct{

  void f1(int &n)   pre(mutate(n)) { assert (n ==42);}
  void f2(int n)   pre(mutate(n)) { assert (n ==42);}
  void f4(const int &n)   pre(mutate(n)) { assert (n ==42);}
  void f9(int &&n)   pre(mutate(n)) { assert (n ==42);}
  void f10(const int &&n)   pre(mutate(n)) { assert (n ==42);}

  void f1(NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}
  void f3(const NonTrivial n)   pre(mutate(n)) { assert (n ==42);}
  void f4(const NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}
  void f9(NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}
  void f10(const NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}


  template<typename T>
  void f1(T &n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  void f2(T n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  void f3(const T n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  void f4(const T &n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  void f9(T &&n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  void f10(const T &&n)   pre(mutate(n)) { assert (n ==42);}

};


struct ConstructorTest1 {
  ConstructorTest1(int n)   pre(mutate(n)) { assert (n ==42);}
  ConstructorTest1(NonTrivial n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  ConstructorTest1(T n)   pre(mutate(n)) { assert (n ==42);}
};

struct ConstructorTest3 {
  ConstructorTest3(int &n)   pre(mutate(n)) { assert (n ==42);}
  ConstructorTest3(NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  ConstructorTest3(T &n)   pre(mutate(n)) { assert (n ==42);}
};

struct ConstructorTest4 {
  ConstructorTest4(const int &n)   pre(mutate(n)) { assert (n ==42);}
  ConstructorTest4(const NonTrivial &n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  ConstructorTest4(const T &n)   pre(mutate(n)) { assert (n ==42);}
};


struct ConstructorTest5 {
  ConstructorTest5(int &&n)   pre(mutate(n)) { assert (n ==42);}
  ConstructorTest5(NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  ConstructorTest5(T &&n)   pre(mutate(n)) { assert (n ==42);}
};

struct ConstructorTest6 {
  ConstructorTest6(const int &&n)   pre(mutate(n)) { assert (n ==42);}
  ConstructorTest6(const NonTrivial &&n)   pre(mutate(n)) { assert (n ==42);}

  template<typename T>
  ConstructorTest6(const T && n)   pre(mutate(n)) { assert (n ==42);}
};

template<typename T>
struct ConstructorTest7 {
  ConstructorTest7(T n)   pre(mutate(n)) { assert (n ==42);}
};

template<typename T>
struct ConstructorTest8 {
  ConstructorTest8(const T n)   pre(mutate(n)) { assert (n ==42); }
};

template<typename T>
void testSet1(){
  T i = 1;

  f1(i);
  f2<T&>(i);
  f2<const T&>(i);
  f2<T&&>(std::move(i));
  f2<const T&&>(std::move(i));
  f3<T&>(i);
  f3<const T&>(i);
  f3<T&&>(std::move(i));
  f3<const T&&>(std::move(i));
  f4(i);
  f9(std::move(i));
  f10(std::move(i));

  TestStruct b;
  b.f1(i);
  b.f2<T&>(i);
  b.f2<const T&>(i);
  b.f2<T&&>(std::move(i));
  b.f2<const T&&>(std::move(i));
  b.f3<T&>(i);
  b.f3<const T&>(i);
  b.f3<T&&>(std::move(i));
  b.f3<const T&&>(std::move(i));
  b.f4(i);
  b.f9(std::move(i));
  b.f10(std::move(i));

  ConstructorTest3 ct3(i);
  ConstructorTest4 ct4(i);
  ConstructorTest5 ct5(std::move(i));
  ConstructorTest6 ct6(std::move(i));

  ConstructorTest7<T &> ct71(i);
  ConstructorTest7<const T &> ct72(i);
  ConstructorTest7<T &&> ct73(std::move(i));
  ConstructorTest7<const T &&> ct74(std::move(i));

  ConstructorTest8<T &> ct81(i);
  ConstructorTest8<const T &> ct82(i);
  ConstructorTest8<T &&> ct83(std::move(i));
  ConstructorTest8<const T &&> ct84(std::move(i));

}
template<typename T>
void testSet2(){
  T i = 1;

  f2(i);

  TestStruct b;
  b.f2(i);

  ConstructorTest1 ct1(1);
}
int main()
{
  testSet1<int>();
  testSet1<NonTrivial>();
  testSet1<NonMoveable>();

  testSet2<int>();
  testSet2<NonTrivial>();

}
