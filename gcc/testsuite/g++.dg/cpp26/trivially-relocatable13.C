// { dg-do compile { target c++26 } }

template <typename T>
constexpr bool relocatable = __builtin_is_trivially_relocatable(T);

static_assert(relocatable<void()>);  // { dg-error "assert" }
// { dg-message "is not trivially relocatable, because" "" { target *-*-* } .-1 }
// { dg-message "not a class or scalar type" "" { target *-*-* } .-2 }


template <typename T>
constexpr bool nothrow_relocatable = __builtin_is_nothrow_relocatable(T);

static_assert(nothrow_relocatable<void()>);  // { dg-line nrv }
// { dg-error "assert" "" { target *-*-* } nrv }
// { dg-message "is not nothrow relocatable, because" "" { target *-*-* } nrv }
// { dg-message "not trivially relocatable" "" { target *-*-* } nrv }
// { dg-message "not a class or scalar type" "" { target *-*-* } nrv }
// { dg-message "not nothrow move constructible" "" { target *-*-* } nrv }
// { dg-error "could not convert" "" { target *-*-* } nrv }


template <typename T>
constexpr bool replaceable = __builtin_is_replaceable(T);

static_assert(replaceable<void()>);  // { dg-error "assert" }
// { dg-message "is not replaceable, because" "" { target *-*-* } .-1 }
// { dg-message "not a class or scalar type" "" { target *-*-* } .-2 }

static_assert(replaceable<const int>);  // { dg-error "assert" }
// { dg-message "const" "" { target *-*-* } .-1 }


struct A {};
struct B : virtual A {};  // { dg-message "virtual base" }
static_assert(relocatable<B>);  // { dg-error "assert" }

struct C {
  ~C() = delete;  // { dg-message "deleted|declared here" }
};
static_assert(relocatable<C>);  // { dg-error "assert" }

struct D {  // { dg-error "deleted" }
  C c;
};
static_assert(relocatable<D>);  // { dg-error "assert" }

struct E trivially_relocatable_if_eligible {   // { dg-bogus "trivially_relocatable_if_eligible" }
  B b;  // { dg-message "field" }
};
static_assert(relocatable<E>);  // { dg-error "assert" }

struct F : E {};  // { dg-message "base class" }
static_assert(relocatable<F>);  // { dg-error "assert" }

struct G {  // { dg-message "default-movable" }
  G(G&&);  // { dg-message "user-provided" }
};
static_assert(relocatable<G>);  // { dg-error "assert" }

union H {  // { dg-message "user-declared special member functions" }
  H(H&&) = default;
  H& operator=(H&&);  // { dg-message "user-provided" }
};
static_assert(relocatable<H[2]>);  // { dg-error "assert" }

struct I {  // { dg-message "default-movable" }
  ~I();  // { dg-message "user-provided" }
};
static_assert(relocatable<I>);  // { dg-error "assert" }

struct J {  // { dg-message "default-movable" }
  ~J() noexcept(false);  // { dg-message "noexcept" }
};
static_assert(nothrow_relocatable<J[5]>);  // { dg-error "assert" }

struct K {  // { dg-message "default-movable" }
  ~K();
};
static_assert(replaceable<K>);  // { dg-error "assert" }

struct L {
  ~L() = delete;  // { dg-message "deleted" }
};
static_assert(replaceable<L>);  // { dg-error "assert" }

struct M {
  M(M&&, int = 0);
  M(M&&);
};
static_assert(replaceable<M>);  // { dg-error "assert" }
// { dg-error "ambiguous" "" { target *-*-* } .-1 }

struct N {
  N(N&&);  // { dg-message "user-provided" }
  N& operator=(N&&);
};
static_assert(replaceable<N>);  // { dg-error "assert" }

struct O replaceable_if_eligible {  // { dg-bogus "replaceable_if_eligible" }
  N n;  // { dg-message "field" }
};
static_assert(replaceable<O>);  // { dg-error "assert" }

struct P : O {};  // { dg-message "base" }
static_assert(replaceable<P[2]>);  // { dg-error "assert" }
