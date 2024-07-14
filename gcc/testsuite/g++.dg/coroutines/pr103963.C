#include <coroutine>

class task {
public:
    class task_promise {
    public:
        task get_return_object() { return task{handle_type::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_never{}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    using handle_type = std::coroutine_handle<task_promise>;
    handle_type handle;

    auto await_ready() { return false; }
    auto await_suspend(handle_type) { return handle; }
    void await_resume() { handle.resume(); }

    using promise_type = task_promise;
    task(handle_type h) : handle(h) {}
    task(const task&) = delete;
    task& operator=(const task&) = delete;
    task(task&&) = delete;
    task& operator=(task&&) = delete;
};

int main() {
    task f = []() -> task { co_return; }();
    task g = [&f]() -> task { co_await f; }();
    g.handle.resume();
}
