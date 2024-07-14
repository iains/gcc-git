#include <coroutine>
#include <memory>
#include <vector>

struct s {
    s(std::vector<long> &&);
};
struct async_task {
    struct promise_type {
        auto initial_suspend() const { return std::suspend_never{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        auto get_return_object() { return async_task{}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    bool await_ready() const { return false; }
    void await_suspend(std::coroutine_handle<> h) {}
    auto await_resume() {}
};
auto g(auto f) {
    return async_task{};
}
async_task f() {
    // comment out co_await to make error disappear
    co_await g(std::make_unique<s>(std::vector{0L})); // error
    //auto task = g(std::make_unique<s>(std::vector{0L})); // ok
    //co_await task;
    co_return;
}
int main() {
    f();
}
