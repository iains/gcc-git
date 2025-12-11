// CWG3049 - implicitly deleted copy assignment operator does not affect trivial relocatibility
// { dg-do compile { target c++11 } }
// { dg-options "" }
// { dg-additional-options "-pedantic" { target c++17 } }

#if __cpp_trivial_relocatability < 202502L
#define trivially_relocatable_if_eligible __trivially_relocatable_if_eligible
#define replaceable_if_eligible __replaceable_if_eligible
#endif

namespace std
{
template <typename T, T v>
struct integral_constant
{
  static constexpr T value = v;
};

template <typename>
struct is_trivially_relocatable;

template <typename>
struct is_replaceable;

template<typename T>
struct is_trivially_relocatable
  : public integral_constant <bool, __builtin_is_trivially_relocatable (T)>
{
};

template<typename T>
struct is_replaceable
  : public integral_constant <bool, __builtin_is_replaceable (T)>
{
};

template <typename T>
inline constexpr bool is_trivially_relocatable_v	// { dg-warning "inline variables are only available with" "" { target c++14_down } }
  = __builtin_is_trivially_relocatable (T);		// { dg-warning "variable templates only available with" "" { target c++11_down } .-1 }

template <typename T>
inline constexpr bool is_replaceable_v			// { dg-warning "inline variables are only available with" "" { target c++14_down } }
  = __builtin_is_replaceable (T);			// { dg-warning "variable templates only available with" "" { target c++11_down } .-1 }
}

struct A {
  const int i; };
struct B { int& r; };
struct C trivially_relocatable_if_eligible { C(); C operator=(const C&) = delete; };
struct D { C m; };


static_assert (std::is_trivially_relocatable_v <A>, "");
static_assert (std::is_trivially_relocatable_v <B>, "");
static_assert (std::is_trivially_relocatable_v <C>, "");
static_assert (std::is_trivially_relocatable_v <D>, "");
static_assert (!std::is_replaceable_v <A>, "");
static_assert (!std::is_replaceable_v <B>, "");
static_assert (!std::is_replaceable_v <C>, "");
static_assert (!std::is_replaceable_v <D>, "");



struct E1{
private:
   ~E1() = default;
};
struct F1{
  E1 e;
};

struct E2{
   ~E2() = delete;
};
struct F2{
  E2 e;
};

static_assert (std::is_trivially_relocatable_v <E1>, "");
static_assert (std::is_replaceable_v <E1>, "");
static_assert (std::is_trivially_relocatable_v <F1>, "");
static_assert (!std::is_replaceable_v <F1>, "");

static_assert (!std::is_trivially_relocatable_v <E2>, "");
static_assert (!std::is_replaceable_v <E2>, "");
static_assert (!std::is_trivially_relocatable_v <F2>, "");
static_assert (!std::is_replaceable_v <F2>, "");


