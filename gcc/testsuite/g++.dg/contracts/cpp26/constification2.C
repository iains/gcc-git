// check that we do not get unused warnings for contract check function parameters
// { dg-do compile }
// { dg-options "-std=c++2b -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=enforce -O " }

struct S{
  S(){};
  S(const S&){}
  ~S(){};
  int x = 0;
};

int i = 3;
int *pi = &i;
int *pi2 = &i;


void f1() pre(i++) {} // { dg-error "increment of read-only location" }
void f2() pre((*pi)++) {};
void f3() pre(pi2++) {} // { dg-error "increment of read-only location" }

int a[2] = {1, 2};
auto [x, y] = a;
auto& [xr, yr] = a;

void f4() pre(x++) {} // { dg-error "increment of read-only location" }
void f5() pre(xr++) {}; // { dg-error "increment of read-only location" }

void f7(int x) pre([](int& i)
	      {
		return true;
	      }(x)){} // { dg-error "discards qualifiers" }
// { dg-error {no match for call to} "" { target *-*-* } .-1 }


int g = 0;
struct X { bool m(); };
struct Y {
  int z = 0;
  //, int* p, int& r, X x, X* px)

  void f(int i)  pre (++g); // { dg-error "increment of read-only location" }
  void f2(int i) pre (++i); // { dg-error "increment of read-only location" }
  void f3(int* p) pre (++(*p)); // OK
  void f4(int& r) pre (++r); // { dg-error "increment of read-only location" }
  void f5(X x) pre (x.m()); // { dg-error " argument discards qualifiers" }
  void f6(X* px) pre (px->m()) // OK

  // TODO when lambdas are fixed
 /*void f7 pre ([=,&i,*this] mutable
	 {
	    ++g;	// error: attempting to modify const lvalue
	    ++i;	// error: attempting to modify const lvalue
	    ++p;	// OK, refers to member of closure type
	    ++r;	// OK, refers to non−reference member of closure type
	    ++this->z; // OK, captured *this
	    ++z;	// OK, captured *this
	    int j = 17;
	    [&]{
	      int k = 34;
	      ++i; // error: attempting to modify const lvalue
	      ++j; // OK
	      ++k; // OK
	    }();
	    return true;
	 }())*/;

  template <int N, int& R, int* P>
  void tf1() pre(++N); 	// { dg-error "increment of read-only location" }
  // { dg-error {lvalue required as increment operand} "" { target *-*-* } .-1 }

  template <int N, int& R, int* P>
  void tf2() pre(++R);	// { dg-error "increment of read-only location" }

  template <int N, int& R, int* P>
  void tf3() pre(++(*P)); // OK

  int h1() post(r : ++r); // { dg-error "increment of read-only" }

  // TODO when lambdas are fixed
/*  int h2() post(r : [=]() mutable {
			++r;	// OK, refers to member of closure type
			return true;
	    }());
*/
  int& k() post(r : ++r); // { dg-error "increment of read-only" }
};
