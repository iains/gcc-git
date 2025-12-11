// { dg-do compile { target c++26 } }
// Make sure we properly do overload resolution when checking default-movable.

template <typename T>
constexpr bool trivially_relocatable = __builtin_is_trivially_relocatable(T);

template <typename T>
constexpr bool replaceable = __builtin_is_replaceable(T);

struct A {
  A(A&&, int = 0) = delete;
  A(A&&) = default;
};
static_assert(!trivially_relocatable<A>);
static_assert(!replaceable<A>);

struct B {
  B& operator=(B&&) = default;
  template <typename = void> B& operator=(B&&);
};
static_assert(trivially_relocatable<B>);
static_assert(!replaceable<B>);

struct C {
  C(C&&, int = 0) = delete;
  C(C&&);
};
static_assert(!trivially_relocatable<C>);
static_assert(!replaceable<C>);

struct D {
  D(D&&) = default;
  // lvalue operator= is fine
  D& operator=(D&&) & = default;
};
static_assert(trivially_relocatable<D>);
static_assert(replaceable<D>);

struct E {
  E(E&&) = default;
  // rvalue operator= is not fine
  E& operator=(E&&) && = default;
};
static_assert(trivially_relocatable<E>);
static_assert(!replaceable<E>);

struct F {
  F(F&&) = default;
  // having both is fine as long as we select a defaulted one.
  F& operator=(const F&) & = default;
  F& operator=(F&) &&;
};
static_assert(trivially_relocatable<F>);
static_assert(replaceable<F>);
