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

void f1(int &n)   post(mutate(n)) {}

void f4(const int &n)   post(mutate(n)) {}

void f9(int &&n) post (mutate(n)) {}

void f10(const int &&n) post (mutate(n)) {};

void f1(NonTrivial &n)   post(mutate(n)) {}

void f4(const NonTrivial &n)   post(mutate(n)) {}

void f9(NonTrivial &&n)   post(mutate(n)) {}

void f10(const NonTrivial &&n)   post(mutate(n)) {}


template<typename T>
void f1(T &n)   post(mutate(n)) {}

template<typename T>
void f2(T n)   post(mutate(n)) {}

template<typename T>
void f3(const T n)   post(mutate(n)) {}

template<typename T>
void f4(const T &n)   post(mutate(n)) {}

template<typename T>
void f9(T &&n)   post(mutate(n)) {}

template<typename T>
void f10(const T &&n)   post(mutate(n)) {}

struct TestStruct{

  void f1(int &n)   post(mutate(n)) {}
  void f4(const int &n)   post(mutate(n)) {}

  void f9(int &&n)   post(mutate(n)) {}

  void f10(const int &&n)   post(mutate(n)) {}

  void f1(NonTrivial &n)   post(mutate(n)) {}
  void f4(const NonTrivial &n)   post(mutate(n)) {}
  void f9(NonTrivial &&n)   post(mutate(n)) {}
  void f10(const NonTrivial &&n)   post(mutate(n)) {}


  template<typename T>
  void f1(T &n)   post(mutate(n)) {}

  template<typename T>
  void f2(T n)   pre(mutate(n)) {}

  template<typename T>
  void f3(const T n)   post(mutate(n)) {}

  template<typename T>
  void f4(const T &n)   post(mutate(n)) {}

  template<typename T>
  void f9(T &&n)   post(mutate(n)) {}

  template<typename T>
  void f10(const T &&n)   post(mutate(n)) {}

};

struct ConstructorTest3 {
  ConstructorTest3(int &n)   post(mutate(n)) {}
  ConstructorTest3(NonTrivial &n)   post(mutate(n)) {}

  template<typename T>
  ConstructorTest3(T &n)   post(mutate(n)) {}
};

struct ConstructorTest4 {
  ConstructorTest4(const int &n)   post(mutate(n)) {}
  ConstructorTest4(const NonTrivial &n)   post(mutate(n)) {}

  template<typename T>
  ConstructorTest4(const T &n)   post(mutate(n)) {}
};


struct ConstructorTest5 {
  ConstructorTest5(int &&n)   post(mutate(n)) {}
  ConstructorTest5(NonTrivial &&n)   post(mutate(n)) {}

  template<typename T>
  ConstructorTest5(T &&n)   post(mutate(n)) {}
};

struct ConstructorTest6 {
  ConstructorTest6(const int &&n)   post(mutate(n)) {}
  ConstructorTest6(const NonTrivial &&n)   post(mutate(n)) {}

  template<typename T>
  ConstructorTest6(const T && n)   post(mutate(n)) {}
};

template<typename T>
struct ConstructorTest7 {
  ConstructorTest7(T n)   post(mutate(n)) {}
};

template<typename T>
struct ConstructorTest8 {
  ConstructorTest8(const T n)   post(mutate(n)) {}
};


template <typename T>
void testSetFree()
{
  {
    T i = 1;
    f1(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f2<T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f2<const T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f2<T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    f2<const T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    f3<T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f3<const T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f3<T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    f3<const T&&>(std::move(i));
    assert ( i == 42);
  }

  {
    T i = 1;
    f4(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    f9(std::move(i));
    assert ( i == 42);
  }
  {
    int i = 1;
    f10(std::move(i));
    assert ( i == 42);
  }

}

template <typename T>
void testSetMemberFunc()
{
  TestStruct b;
  {
    T i = 1;
    b.f1(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f2<T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f2<const T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f2<T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f2<const T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f3<T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f3<const T&>(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f3<T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f3<const T&&>(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f4(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    b.f9(std::move(i));
    assert ( i == 42);
  }
  {
    int i = 1;
    b.f10(std::move(i));
    assert ( i == 42);
  }

}

template <typename T>
void testSetConstructor()
{
  {
    T i = 1;
    ConstructorTest3 t(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest4 t (i);
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest5 t(std::move(i));
    assert ( i == 42);
  }

  {
    T i = 1;
    ConstructorTest6 t(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest7<T&> t(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest7<const T&> t(i);
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest7<T&&> t(std::move(i));
    assert ( i == 42);
  }
  {
    T i = 1;
    ConstructorTest7<const T&&> t(std::move(i));
    assert ( i == 42);
  }
  {
     T i = 1;
     ConstructorTest8<T&> t(i);
     assert ( i == 42);
   }
   {
     T i = 1;
     ConstructorTest8<const T&> t(i);
     assert ( i == 42);
   }
   {
     T i = 1;
     ConstructorTest8<T&&> t(std::move(i));
     assert ( i == 42);
   }
   {
     T i = 1;
     ConstructorTest8<const T&&> t(std::move(i));
     assert ( i == 42);
   }
}

int main()
{
  testSetFree<int>();
  testSetFree<NonTrivial>();
  testSetFree<NonMoveable>();

  testSetMemberFunc<int>();
  testSetMemberFunc<NonTrivial>();
  testSetMemberFunc<NonMoveable>();

  testSetConstructor<int>();
  testSetConstructor<NonTrivial>();
  testSetConstructor<NonMoveable>();

}
