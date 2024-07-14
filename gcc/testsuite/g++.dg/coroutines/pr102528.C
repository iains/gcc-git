#include <coroutine>

struct simple {
    struct promise_type {
        void return_void() {}
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() { /*throw;*/ }
        simple get_return_object() { return {}; }
    };

    std::true_type await_ready() {return {}; }
    void await_suspend(std::coroutine_handle<>) {}
    void await_resume() {}
};

inline simple
test1() {
    co_return;
}

inline simple
test2() {
    co_await test1();
    co_return;
}

#if 0
void test() {
    test2();
}
#else
void test() {
    test1();
}
#endif
