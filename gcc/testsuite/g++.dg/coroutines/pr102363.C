#include <coroutine>
#include <source_location>
#include <cstdio>
struct example {
    struct promise_type {
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() { throw; }
        example get_return_object() { return {}; }
        void return_void() {}

        auto await_transform(const char* dummy, std::source_location l = std::source_location::current()) {
            printf("bad: %s:%u:%u: %s\n", l.file_name(), l.line(), l.column(), l.function_name());
            return std::suspend_never();
        }
    };
};

example func() {
    auto l = std::source_location::current();
    printf("good: %s:%u:%u: %s\n", l.file_name(), l.line(), l.column(), l.function_name());

    // bad location points here:
    //       v
    co_await "pretend this is an awaitable";
}

int main() {
    func();
}
