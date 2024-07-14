//  { dg-do run }

#include <coroutine>
#include <exception>
#include <utility>
#include <cassert>
#ifdef OUTPUT
#include <cstdio>
#endif

struct task {
  struct promise_type {
        task get_return_object() noexcept {
#ifdef OUTPUT
            std::puts("task::promise_type get_return_object");
#endif
            return task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept {
#ifdef OUTPUT
            std::puts("task::promise_type initial_suspend");
#endif
            return {};
        }
        
        struct yield_awaiter {
            bool await_ready() noexcept { return false; }
            auto await_suspend(std::coroutine_handle<promise_type> h) noexcept {
#ifdef OUTPUT
               std::printf("task::yield_awaiter yielding/returning value h %p cont %p\n",
                            h.address(),  h.promise().continuation.address());
#endif
                if (h.promise().continuation.address())
                  return std::exchange(h.promise().continuation, {});
                return std::coroutine_handle<>(std::noop_coroutine());
            }
            void await_resume() noexcept {
#ifdef OUTPUT
                std::puts("task::yield_awaiter::res resumed");
#endif
            }
        };

        yield_awaiter yield_value(int x) noexcept {
#ifdef OUTPUT
            std::printf("task::promise_type yield_value %d\n", x);
#endif
            value = x;
            return {};
        }

        void return_value(int x) noexcept {
#ifdef OUTPUT
            std::printf("task::promise_type return_value %d\n", x);
#endif
            value = x;
        }

        yield_awaiter final_suspend() noexcept {
#ifdef OUTPUT
            std::printf("task::promise_type final_suspend\n");
#endif
           return {};
        }

        [[noreturn]] void unhandled_exception() noexcept {
            std::terminate();
        }

        int value;
        std::coroutine_handle<> continuation;
    };

    explicit task(std::coroutine_handle<promise_type> coro) noexcept
    : coro(coro)
    {
#ifdef OUTPUT
    std::printf("task(coro) %p\n", coro.address());
 #endif
   }

    task(task&& t) noexcept
    : coro(std::exchange(t.coro, {}))
    {
#ifdef OUTPUT
     std::printf("task(task&& t) %p\n", coro.address());
#endif
     }

    ~task() {
#ifdef OUTPUT
        std::printf("~task() %p\n", coro ? coro.address() : 0);
#endif
        if (coro) coro.destroy();
    }

    __attribute__((noinline)) bool await_ready() {
#ifdef OUTPUT
        std::puts("task::await_ready");
#endif
        return false;
    }

    __attribute__((noinline)) auto await_suspend(std::coroutine_handle<> h) noexcept {
#ifdef OUTPUT
        std::printf("task::await_suspend h %p\n", h.address());
#endif
        coro.promise().continuation = h;
        return coro;
    }

    __attribute__((noinline)) int await_resume() {
#ifdef OUTPUT
        std::puts("task::await_resume");
#endif
        return coro.promise().value;
    }

    std::coroutine_handle<promise_type> coro;
};

task two_values() {
#ifdef OUTPUT
    std::puts("two_values");
#endif
    co_yield 1;
    co_return 2;
}

task example() {
#ifdef OUTPUT
    std::puts("example");
#endif
    task t = two_values();
    int result = co_await t + co_await t;
    //result += co_await t;
#ifdef OUTPUT
    std::printf("result = %i (should be 3)\n", result);
    std::fflush(stdout);
#endif
    if (result != 3)
      abort();
    co_return result;
}

int main() {
#ifdef OUTPUT
    std::printf("creating\n");
 #endif
   task t = example();
    t.coro.resume(); // Start the coro.
#ifdef OUTPUT
    std::printf("should be done\n");
#endif
    //std::printf("about to check done\n");
    //assert(t.coro.done());
}
