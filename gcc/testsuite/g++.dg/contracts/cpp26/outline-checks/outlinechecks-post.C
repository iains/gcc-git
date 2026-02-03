// Test that there is no crash when using outline checks.
// { dg-do run }
// { dg-options "-std=c++26 -fcontracts -fcontract-checks-outlined" }

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

void f1(int &n)   post(check(n)) {}

void f2(int n, const int m = 1)   post(check(m)) {}

void f3(const int n)   post(check(n)) {}

void f4(const int &n)   post(check(n)) {}

void f5(int *n, const int m = 1)   post(check(m)) {}

void f6(const int *n, const int m = 1)   post(check(m)) {}

void f7(const int *&n)   post(check(n)) {}

void f8(int *&n)   post(check(n)) {}

void f9(int &&n) post (check(n)) {};

void f10(const int &&n) post (check(n)) {};

void f1(S &n)   post(check(n)) {}

void f2(S n, const int m = 1)   post(check(m)) {}

void f3(const S n)   post(check(n)) {}

void f4(const S &n)   post(check(n)) {}

void f5(S *n, const int m = 1)   post(check(m)) {}

void f6(const S *n, const int m = 1)   post(check(m)) {}

void f7(const S *&n)   post(check(n)) {}

void f8(S *&n)   post(check(n)) {}

void f9(S &&n)   post(check(n)) {}

void f10(const S &&n)   post(check(n)) {}


template<typename T>
void f1(T &n)   post(check(n)) {}

template<typename T>
void f2(T n, const int m = 1)   post(check(m)) {}

template<typename T>
void f3(const T n)   post(check(n)) {}

template<typename T>
void f4(const T &n)   post(check(n)) {}

template<typename T>
void f5(T *n, const int m = 1)   post(check(m)) {}

template<typename T>
void f6(const T *n, const int m = 1)   post(check(m)) {}

template<typename T>
void f7(const T *&n)   post(check(n)) {}

template<typename T>
void f8(T *&n)   post(check(n)) {}

template<typename T>
void f9(T &&n)   post(check(n)) {}

template<typename T>
void f10(const T &&n)   post(check(n)) {}

struct TestStruct{

  void f1(int &n)   post(check(n)) {}
  void f2(int n, const int m = 1)   post(check(m)) {}
  void f3(const int n)   post(check(n)) {}
  void f4(const int &n)   post(check(n)) {}
  void f5(int *n, const int m = 1)   post(check(m)) {}
  void f6(const int *n, const int m = 1)   post(check(m)) {}
  void f7(const int *&n)   post(check(n)) {}
  void f8(int *&n)   post(check(n)) {}
  void f9(int &&n)   post(check(n)) {}
  void f10(const int &&n)   post(check(n)) {}

  void f1(S &n)   post(check(n)) {}
  void f2(S n, const int m = 1)   post(check(m)) {}
  void f3(const S n)   post(check(n)) {}
  void f4(const S &n)   post(check(n)) {}
  void f5(S *n, const int m = 1)   post(check(m)) {}
  void f6(const S *n, const int m = 1)   post(check(m)) {}
  void f7(const S *&n)   post(check(n)) {}
  void f8(S *&n)   post(check(n)) {}
  void f9(S &&n)   post(check(n)) {}
  void f10(const S &&n)   post(check(n)) {}


  template<typename T>
  void f1(T &n)   post(check(n)) {}

  template<typename T>
  void f2(T n, const int m = 1)   post(check(m)) {}

  template<typename T>
  void f3(const T n)   post(check(n)) {}

  template<typename T>
  void f4(const T &n)   post(check(n)) {}

  template<typename T>
  void f5(T *n, const int m = 1)   post(check(m)) {}

  template<typename T>
  void f6(const T *n, const int m = 1)   post(check(m)) {}

  template<typename T>
  void f7(const T *&n)   post(check(n)) {}

  template<typename T>
  void f8(T *&n)   post(check(n)) {}

  template<typename T>
  void f9(T &&n)   post(check(n)) {}

  template<typename T>
  void f10(const T &&n)   post(check(n)) {}

  ~TestStruct() post(true)  {}
};


struct ConstructorTest1 {
  ConstructorTest1(int n, const int m = 1)   post(check(m)) {}
  ConstructorTest1(S n, const int m = 1)   post(check(m)) {}

  template<typename T>
  ConstructorTest1(T n, const int m = 1)   post(check(m)) {}
};

struct ConstructorTest2 {
  ConstructorTest2(const int n)   post(check(n)) {}
  ConstructorTest2(const S n)   post(check(n)) {}

  template<typename T>
  ConstructorTest2(const T n)   post(check(n)) {}
};

struct ConstructorTest3 {
  ConstructorTest3(int &n)   post(check(n)) {}
  ConstructorTest3(S &n)   post(check(n)) {}

  template<typename T>
  ConstructorTest3(T &n)   post(check(n)) {}
};

struct ConstructorTest4 {
  ConstructorTest4(const int &n)   post(check(n)) {}
  ConstructorTest4(const S &n)   post(check(n)) {}

  template<typename T>
  ConstructorTest4(const T &n)   post(check(n)) {}
};


struct ConstructorTest5 {
  ConstructorTest5(int &&n)   post(check(n)) {}
  ConstructorTest5(S &&n)   post(check(n)) {}

  template<typename T>
  ConstructorTest5(T &&n)   post(check(n)) {}
};

struct ConstructorTest6 {
  ConstructorTest6(const int &&n)   post(check(n)) {}
  ConstructorTest6(const S &&n)   post(check(n)) {}

  template<typename T>
  ConstructorTest6(const T && n)   post(check(n)) {}
};


struct ConstructorTest7 {
  ConstructorTest7(int *n, const int m = 1)   post(check(m)) {}
  ConstructorTest7(S *n,const int m = 1)   post(check(m)) {}

  template<typename T>
  ConstructorTest7(T *n, const int m = 1)   post(check(m)) {}
};

struct ConstructorTest8 {
  ConstructorTest8(const int* n, const int m = 1)   post(check(m)) {}
  ConstructorTest8(const S* n, const int m = 1)   post(check(m)) {}

  template<typename T>
  ConstructorTest8(const T* n, const int m = 1)   post(check(m)) {}
};

struct ConstructorTest9 {
  ConstructorTest9(int *&n)   post(check(n)) {}
  ConstructorTest9(S *&n)   post(check(n)) {}

  template<typename T>
  ConstructorTest9(T *&n)   post(check(n)) {}
};

struct ConstructorTest10 {
  ConstructorTest10(const int *&n)   post(check(n)) {}
  ConstructorTest10(const S *&n)   post(check(n)) {}

  template<typename T>
  ConstructorTest10(const T *&n)   post(check(n)) {}
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
  f5(&i);
  f6(&i);
  f7(cpi);
  f8(pi);
  f9(1);
  f10(1);


  f1(s);
  f2(s);
  f3(s);
  f4(s);
  f5(&s);
  f6(&s);
  f7(cps);
  f8(ps);
  f9(S{});
  f10(S{});

  f1(s2);
  f2(s2);
  f3(s2);
  f4(s2);
  f5(&s2);
  f6(&s2);
  f7(cps2);
  f8(ps2);
  f8(ps2);
  f9(S2{});
  f10(S2{});

  f1(st);
  f2(st);
  f3(st);
  f4(st);
  f5(&st);
  f6(&st);
  f7(cpst);
  f8(pst);
  f9(ST<int>{});
  f10(ST<int>{});

  TestStruct b;
  b.f1(i);
  b.f2(i);
  b.f3(i);
  b.f4(i);
  b.f5(&i);
  b.f6(&i);
  b.f8(pi);
  b.f9(1);
  b.f10(1);

  b.f1(s);
  b.f2(s);
  b.f3(s);
  b.f4(s);
  b.f5(&s);
  b.f6(&s);
  b.f8(ps);
  b.f9(S{});
  b.f10(S{});


  b.f1(s2);
  b.f2(s2);
  b.f3(s2);
  b.f4(s2);
  b.f5(&s2);
  b.f6(&s2);
  b.f7(cps2);
  b.f8(ps2);
  b.f9(S2{});
  b.f10(S2{});

  b.f1(st);
  b.f2(st);
  b.f3(st);
  b.f4(st);
  b.f5(&st);
  b.f6(&st);
  b.f7(cpst);
  b.f8(pst);
  b.f9(ST<int>{});
  b.f10(ST<int>{});

  {
    ConstructorTest1 const1(i);
    ConstructorTest2 const2(i);
    ConstructorTest3 const3(i);
    ConstructorTest4 const4(i);
    ConstructorTest5 const5(1);
    ConstructorTest6 const6(1);
    ConstructorTest7 const7(pi);
    ConstructorTest8 const8(cpi);
    ConstructorTest9 const9(pi);
    ConstructorTest10 const10(cpi);
  }

  {
    ConstructorTest1 const1(s);
    ConstructorTest2 const2(s);
    ConstructorTest3 const3(s);
    ConstructorTest4 const4(s);
    ConstructorTest5 const5(S{});
    ConstructorTest6 const6(S{});
    ConstructorTest7 const7(ps);
    ConstructorTest8 const8(cps);
    ConstructorTest9 const9(ps);
    ConstructorTest10 const10(cps);
  }

  {
    ConstructorTest1 const1(s2);
    ConstructorTest2 const2(s2);
    ConstructorTest3 const3(s2);
    ConstructorTest4 const4(s2);
    ConstructorTest5 const5(S2{});
    ConstructorTest6 const6(S2{});
    ConstructorTest7 const7(ps2);
    ConstructorTest8 const8(cps2);
    ConstructorTest9 const9(ps2);
    ConstructorTest10 const10(cps2);
  }

  {
    ConstructorTest1 const1(st);
    ConstructorTest2 const2(st);
    ConstructorTest3 const3(st);
    ConstructorTest4 const4(st);
    ConstructorTest5 const5(ST<int>{});
    ConstructorTest6 const6(ST<int>{});
    ConstructorTest7 const7(pst);
    ConstructorTest8 const8(cpst);
    ConstructorTest9 const9(pst);
    ConstructorTest10 const10(cpst);
  }


}
