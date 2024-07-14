// { dg-do run }

#include <iostream>
#include <coroutine>
#include <array>

using namespace std;

template <typename T>
struct generator {
  struct promise_type {

  generator get_return_object() noexcept {
  return generator{coroutine_handle<promise_type>::from_promise(*this)};
  }

  suspend_always initial_suspend() const noexcept { return {};}
  suspend_always final_suspend() const noexcept {return {};}

  suspend_always yield_value(T& v) noexcept {m_v = &v; return {};}
  suspend_always yield_value(T&& v) noexcept {m_v = &v; return {}; }
  void unhandled_exception() { }
  void return_void() {}

  T& value() const noexcept { return *m_v; }

 private:
  T* m_v;
};

 
  ~generator() {m_coroutine.destroy(); }

  void move_next() { m_coroutine.resume();}

  T& value() {return m_coroutine.promise().value();}

  generator(coroutine_handle<promise_type> coroutine) noexcept
      : m_coroutine(coroutine) {}

  std::coroutine_handle<promise_type> m_coroutine;
};


generator<std::array<std::string, 3>> arr(){
    // Compiles, but leads to segfault/ invalid free when accessed.
    co_yield {"a", "b", "c"};
}

generator<std::array<int, 3>> arrInt(){
    // Works fine
     co_yield {1, 2, 3};
}

struct F {
    std::string x;
    const std::string& operator[](size_t) const {
        return x;
    }
};

generator<F> f() {
    // leads to "munmap_chunk(): invalid pointer";
    co_yield {"abc"};
}

struct G {
    std::string s;
    G(std::string s_in) : s{std::move(s_in)} {} 
    const std::string& operator[](size_t) const {
        return s;
    }
};

generator<G> g() {
    // Works as expected, only difference to F/f() is the manually
    // specified constructor.
    co_yield {"abc"};
}


template<typename Generator>
void outputOne(Generator g) {
    g.move_next();
    const auto& el = g.value();
    std::cout << el[0] << el[1] << el[2] << std::endl;
}

int main() {
    outputOne(g());
    outputOne(f());
    outputOne(arrInt());
    outputOne(arr());
}
