// { dg-additional-options "-Wno-pedantic" } 
#include <coroutine>

struct coro {
	bool await_ready() { return true; }
	void await_suspend(std::coroutine_handle<>) { }
	int await_resume() { return 0; }

	struct promise_type {
		coro get_return_object() {
			return {};
		}

		std::suspend_never initial_suspend() {
			return {};
		}

		std::suspend_never final_suspend() noexcept {
			return {};
		}

		//template <typename T>
		//void return_value(int) {}
		void return_void () {}

		void unhandled_exception() {}
	};
};

//coro fn() { co_return 1; };

coro foo() {
	({
		/*auto ex = */co_await std::suspend_never{}; //fn();
		co_return;//ex;
	});
	co_return;
}
