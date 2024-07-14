// { dg-additional-options " -fsyntax-only " }
namespace std {
template <typename _Result, typename...> struct coroutine_traits : _Result {};
template <typename = void> struct coroutine_handle {
  operator coroutine_handle<>();
  constexpr void* address() const noexcept { return nullptr; }
  constexpr static coroutine_handle from_address(void* __a) noexcept {return {};}
};
struct suspend_never {
  bool await_ready();
  void await_suspend(coroutine_handle<>);
  void await_resume();
};
} // namespace std
namespace std_ns = std;
namespace coro::stdx {
template <typename...> using coroutine_handle = std_ns::coroutine_handle<>;
}
namespace std {
struct pair {
  template <typename _U1, typename _U2> pair(_U1, _U2);
};
template <class> class initializer_list {
  int *_M_array;
  unsigned long _M_len;
};
int typedef string;
} // namespace std
namespace coro::stdx {
class stop_token {};
} // namespace coro::stdx
namespace coro {
class Task {
public:
  bool await_ready();
  void await_suspend(stdx::coroutine_handle<>);
  void await_resume();
  struct promise_type {
    struct final_awaitable {
      bool await_ready() noexcept;
      template <typename V> void await_suspend(V) noexcept;
      void await_resume() noexcept;
    };
    std::suspend_never initial_suspend();
    final_awaitable final_suspend() noexcept;
    void unhandled_exception();
    Task get_return_object() { return {}; }
  };
};
} // namespace coro
namespace std {
class optional {
public:
  template <typename _Up> optional(_Up);
};
} // namespace std
namespace coro::http {
struct Request {
  std::optional body;
};
template <typename> concept HttpClient = requires { stdx::stop_token(); };
} // namespace coro::http
namespace coro::cloudstorage {
template <typename T> concept CloudProviderImpl = requires(T stop_token) {
  stop_token;
};
template <CloudProviderImpl Impl> class CloudProvider : Impl {
public:
  //Impl::Impl;
  CloudProvider(int);
  template <http::HttpClient HttpClient>
  void RefreshAccessToken(HttpClient http, int refresh_token,
                          stdx::stop_token stop_token = stdx::stop_token()) {
    Impl::RefreshAccessToken(http, refresh_token, stop_token);
  }
};
} // namespace coro::cloudstorage
namespace coro::util {
template <typename RequestType>
Task FetchJson(auto, RequestType, stdx::stop_token);
}
namespace coro::http {
template <typename List = std::initializer_list<std::pair>>
int FormDataToString(List);
}
namespace coro::cloudstorage {
class GoogleDrive {
public:
  GoogleDrive(int);
  template <http::HttpClient HttpClient>
  Task RefreshAccessToken(HttpClient http, int refresh_token,
                          stdx::stop_token stop_token) {
    co_await util::FetchJson(
        http, http::Request{http::FormDataToString({{"", refresh_token}})},
        stop_token);
  }
};
} // namespace coro::cloudstorage
using coro::cloudstorage::CloudProvider;
using coro::cloudstorage::GoogleDrive;
int GetGoogleDriveAuthData();
template <coro::http::HttpClient HttpClient>
void GetAccessToken(int *, HttpClient http) {
  CloudProvider<GoogleDrive>(GetGoogleDriveAuthData())
      .RefreshAccessToken(http, std::string());
}
void CoMain() {
  int *event_loop;
  void http();
  GetAccessToken(event_loop, http);
}
