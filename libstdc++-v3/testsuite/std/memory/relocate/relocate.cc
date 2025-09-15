// { dg-do run { target c++26 } }

#include <memory>

#include <testsuite_hooks.h>
#include <testsuite_allocator.h>

struct S trivially_relocatable_if_eligible
{
  constexpr S() : s(0) {}
  constexpr S(int x) : s(x) {}
  constexpr S(S&& x) : s(x.s) {}
  constexpr S& operator=(S&& x) { s = x.s; return *this; }
  unsigned char s;
};

struct T
{
  constexpr T() : s(0) {}
  constexpr T(int x) : s(x) {}
  constexpr T(T&& x) noexcept : s(x.s) {}
  constexpr T& operator=(T&& x) { s = x.s; return *this; }
  unsigned char s;
};

static_assert(std::is_trivially_relocatable_v<S>);
static_assert(std::is_nothrow_relocatable_v<S>);
static_assert(std::is_nothrow_relocatable_v<T>);

template <typename _Tp>
void
test_relocate()
{
  unsigned char a[20], c[20];
  _Tp *sf, *sl, *sr;
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = ::new(&c[5]) _Tp(42);
  sr->~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = ::new(&a[1]) _Tp(42);
  sr->~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = sf + 1;
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
}

template <typename _Tp>
constexpr void
test_relocate_constexpr()
{
  _Tp a[20], c[20];
  for (int i = 0; i < 20; ++i)
    {
      a[i].~_Tp();
      c[i].~_Tp();
    }
  _Tp *sf, *sl, *sr;
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = ::new(&c[5]) _Tp(42);
  sr->~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = ::new(&a[1]) _Tp(42);
  sr->~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
  sf = ::new(&a[2]) _Tp(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) _Tp(i + 9);
  ++sl;
  sr = sf + 1;
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~_Tp();
      ++sr;
    }
  for (int i = 0; i < 20; ++i)
    {
      ::new(&a[i]) _Tp(0);
      ::new(&c[i]) _Tp(0);
    }
}

template <typename _Tp>
void
test_relocate_array()
{
  unsigned char a[20], c[20];
  using A = _Tp[2];
  A *sf, *sl, *sr;
  sf = (A *)(::new(&a[2]) A{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (A *)(::new(&a[4 + 2 * i]) A{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = (A *)(::new(&c[6]) A{42, 42});
  (*sr)[0].~_Tp();
  (*sr)[1].~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~_Tp();
      (*sr)[1].~_Tp();
      ++sr;
    }
  sf = (A *)(::new(&a[2]) A{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (A *)(::new(&a[4 + 2 * i]) A{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = (A *)(::new(&a[0]) A{42, 42});
  (*sr)[0].~_Tp();
  (*sr)[1].~_Tp();
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~_Tp();
      (*sr)[1].~_Tp();
      ++sr;
    }
  sf = (A *)(::new(&a[2]) A{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (A *)(::new(&a[4 + 2 * i]) A{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = sf + 1;
  VERIFY( std::relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~_Tp();
      (*sr)[1].~_Tp();
      ++sr;
    }
}

void
test_trivially_relocate()
{
  unsigned char a[20], c[20];
  S *sf, *sl, *sr;
  sf = ::new(&a[2]) S(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) S(i + 9);
  ++sl;
  sr = ::new(&c[5]) S(42);
  sr->~S();
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~S();
      ++sr;
    }
  sf = ::new(&a[2]) S(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) S(i + 9);
  ++sl;
  sr = ::new(&a[1]) S(42);
  sr->~S();
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~S();
      ++sr;
    }
  sf = ::new(&a[2]) S(11);
  for (int i = 3; i < 8; ++i)
    sl = ::new(&a[i]) S(i + 9);
  ++sl;
  sr = sf + 1;
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( sr->s == i + 11 );
      sr->~S();
      ++sr;
    }
}

void
test_trivially_relocate_array()
{
  unsigned char a[20], c[20];
  using SA = S[2];
  SA *sf, *sl, *sr;
  sf = (SA *)(::new(&a[2]) SA{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (SA *)(::new(&a[4 + 2 * i]) SA{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = (SA *)(::new(&c[6]) SA{42, 42});
  (*sr)[0].~S();
  (*sr)[1].~S();
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~S();
      (*sr)[1].~S();
      ++sr;
    }
  sf = (SA *)(::new(&a[2]) SA{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (SA *)(::new(&a[4 + 2 * i]) SA{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = (SA *)(::new(&a[0]) SA{42, 42});
  (*sr)[0].~S();
  (*sr)[1].~S();
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~S();
      (*sr)[1].~S();
      ++sr;
    }
  sf = (SA *)(::new(&a[2]) SA{11, 12});
  for (int i = 0; i < 5; ++i)
    sl = (SA *)(::new(&a[4 + 2 * i]) SA{13 + 2 * i, 14 + 2 * i});
  ++sl;
  sr = sf + 1;
  VERIFY( std::trivially_relocate(sf, sl, sr) == sr + 6 );
  for (int i = 0; i < 6; ++i)
    {
      VERIFY( (*sr)[0].s == 2 * i + 11 && (*sr)[1].s == 2 * i + 12 );
      (*sr)[0].~S();
      (*sr)[1].~S();
      ++sr;
    }
}

int main()
{
  test_relocate<S>();
  test_relocate<T>();
  test_relocate_constexpr<S>();
  test_relocate_constexpr<T>();
  test_relocate_array<S>();
  test_relocate_array<T>();
  test_trivially_relocate();
  test_trivially_relocate_array();
  static_assert([] {
    test_relocate_constexpr<S>();
    test_relocate_constexpr<T>();
    return true;
  }());
}
