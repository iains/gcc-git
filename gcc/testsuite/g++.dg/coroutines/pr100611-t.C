#include <coroutine>
#include <iostream>

class Task
{
public:
	struct promise_type
	{
		Task get_return_object()
		{
			return {
				std::coroutine_handle<promise_type>::from_promise(*this)
			};
		}

		std::suspend_never initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() {}
		void return_void() {}
	};

	~Task()
	{
		if(handle_)
		{
			handle_.destroy();
		}
	}

	bool await_ready() { return false; }
	bool await_suspend(std::coroutine_handle<>) { return false; }
	bool await_resume() { return false; }

private:
	Task(std::coroutine_handle<promise_type> handle) :
		handle_(handle)
	{

	}

	std::coroutine_handle<promise_type> handle_;
};

class Foo
{
public:
	Foo(int id):
		id_(id)
	{
		std::cout << "Foo(" << id_ << ") " << this << std::endl;
	}

	Foo(const Foo& other):
		id_(other.id_)
	{
		std::cout << "Foo(const& " << id_ << ") " << this << std::endl;
	}

	Foo(Foo&& other):
		id_(other.id_)
	{
		std::cout << "Foo(&& " << id_ << ") " << this << std::endl;
	}

	~Foo()
	{
		std::cout << "~Foo(" << id_ << ") " << this << std::endl;
	}

private:
	int id_;
};

Task test()
{
	Foo foo(23);

	co_await[foo]() -> Task
	{
		co_return;
	}();
}

int main()
{
	test();
}
