// { dg-do compile { target c++17 } }
// { dg-options "-fconcepts" }

template <class T>
concept Isint = __is_same_as(T,int);

template <class T>
struct A
{
  int i;
  A(...);
};

template <Isint I>
A(I) -> A<I>;

A a(1);
A a2(1.0);			// { dg-error "" }
