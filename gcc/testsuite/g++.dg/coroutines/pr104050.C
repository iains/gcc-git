// { dg-additional-options "-O1 -fcompare-debug -save-temps" }
#include <coroutine>
#include <vector>
template <typename> struct promise {
  struct final_awaitable {
    bool await_ready() noexcept;
    template <typename Promise>
    std::coroutine_handle<>
        await_suspend(std::coroutine_handle<Promise>) noexcept;
    void await_resume() noexcept;
  };
  auto get_return_object() {
    return std::coroutine_handle<promise>::from_promise(*this);
  }
  auto initial_suspend() { return std::suspend_always(); }
  auto final_suspend() noexcept { return final_awaitable(); }
  void unhandled_exception();
};
template <typename T> struct task {
  using promise_type = promise<T>;
  task(std::coroutine_handle<promise<T>>);
  bool await_ready();
  std::coroutine_handle<> await_suspend(std::coroutine_handle<>);
  T await_resume();
};
task<std::vector<int>> foo() {
  while ((co_await foo()).empty())
    ;
}
