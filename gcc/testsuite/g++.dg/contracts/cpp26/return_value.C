// Various tests with non trivial return value identifier
// in either derived class or base class.
// { dg-do compile }
// { dg-options "-std=c++2a -fcontracts -fcontracts-nonattr" }


struct NonTrivial{
  NonTrivial(){};
  NonTrivial(const NonTrivial&){}
  ~NonTrivial(){};
  int x = 0;
};

template<typename T>
bool check(T t){
  return true;
}
struct S{
 NonTrivial f1(const NonTrivial i) post(r: i.x > 0 ) { return NonTrivial{};}

 template <typename T>
 NonTrivial f2(const T i) post(r: i.x > 0 ) { return NonTrivial{};}

 auto f3(const NonTrivial i) post(r: i.x > 0 ) { return NonTrivial{};}

 template <typename T>
 T f4(const T i) post(r: i.x > 0 ) { return NonTrivial{};}

 template <typename T>
 auto f5(const T i) post(r: i.x > 0 ) { return i;}

 template <typename T>
 auto f6(const T i) post(r: check(i) ) { return i;}

 auto f7(const NonTrivial i) post(r: check(r) ) { return i;}

};

template <typename U>
struct S1
{

  struct S2
  {
    NonTrivial
    f1 (const NonTrivial i) post(r: i.x > 0 )
      { return NonTrivial
	  {};
      }

      template <typename T>
      NonTrivial
      f2 (const T i) post(r: i.x > 0 )
	{ return NonTrivial
	    {};
	}

	auto
	f3 (const NonTrivial i)
	post(r: i.x > 0 )
	  { return NonTrivial
	      {};}

	template <typename T>
	T
	f4 (const T i)
	post(r: i.x > 0 )
	  { return NonTrivial
	      {};}

	template <typename T>
	auto
	f5 (const T i)
	post(r: i.x > 0 )
	  { return i;}

      };

    NonTrivial
    f1 (const NonTrivial i)
    post(r: i.x > 0 )
      { S2 s;
	return s.f1(i);
      }

    template <typename T>
    NonTrivial
    f2 (const T i)
    post(r: i.x > 0 )
      { S2 s;
	return s.f2(i);
      }

    auto
    f3 (const NonTrivial i)
    post(r: i.x > 0 )
      { S2 s;
	return s.f3(i);
      }

    template <typename T>
    T
    f4 (const T i)
    post(r: i.x > 0 )
      { S2 s;
	return s.f4(i);
      }

    template <typename T>
    auto
    f5 (const T i)
    post(r: i.x > 0 )
      { S2 s;
	return s.f5(i);
      }

  };


int main()
{
  S s;
  NonTrivial n;
  s.f1(NonTrivial{});

  s.f2(n);
  s.f3(n);
  s.f4(n);
  s.f5(n);
  s.f6(n);
  s.f7(n);


  S1<NonTrivial> s1;
  s1.f1(n);
  s1.f2(n);
  s1.f3(n);
  s1.f4(n);
  s1.f5(n);
}
