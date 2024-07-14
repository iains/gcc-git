namespace std {
template <int __v> struct integral_constant {
  static constexpr int value = __v;
};
template <typename _Tp, typename _Up = _Tp> _Up __declval(int);
template <typename _Tp> auto declval() -> decltype(__declval<_Tp>(0));
template <typename> struct __success_type;
template <bool, bool, typename...> struct __result_of_impl;
template <typename _Fn, typename... _Args>
__success_type<decltype(declval<_Fn>()(_Args()...))> _S_test;
template <typename _Functor, typename... _ArgTypes>
struct __result_of_impl<false, false, _Functor, _ArgTypes...> {
  typedef decltype(_S_test<_Functor, _ArgTypes...>) type;
};
template <typename _Functor, typename... _ArgTypes>
struct __invoke_result : __result_of_impl<integral_constant<false>::value,
                                          integral_constant<false>::value,
                                          _Functor, _ArgTypes...> {};
template <typename _Fn, typename... _Args>
using invoke_result_t = __invoke_result<_Fn, _Args...>::type;
template <typename> void forward();
template <typename> struct iterator_traits;
template <typename _Tp> struct iterator_traits<_Tp *> {
  using reference = _Tp;
};
} // namespace std
template <typename _Iterator> struct __normal_iterator {
  std::iterator_traits<_Iterator>::reference operator*();
};
namespace std {
template <class> struct initializer_list {
  int *_M_array;
  unsigned long _M_len;
};
template <typename, typename _Fn> void __invoke_impl(_Fn);
template <typename _Callable, typename... _Args>
void __invoke(_Callable __fn, _Args... __args) {
  __invoke_impl<typename __invoke_result<_Callable, _Args...>::type>(
      __fn(__args)...);
}
template <typename> struct allocator;
template <typename> struct allocator_traits;
template <typename _Tp> struct allocator_traits<allocator<_Tp>> {
  using pointer = _Tp *;
  template <typename _Up> using rebind_alloc = allocator<_Up>;
};
} // namespace std
template <typename _Alloc>
struct __alloc_traits : std::allocator_traits<_Alloc> {
  template <typename _Tp> struct rebind {
    typedef std::allocator_traits<_Alloc>::template rebind_alloc<_Tp> other;
  };
};
namespace std {
template <typename...> class tuple;
template <typename _Tp, typename _Alloc> struct _Vector_base {
  typedef __alloc_traits<_Alloc>::template rebind<_Tp>::other _Tp_alloc_type;
  typedef __alloc_traits<_Tp_alloc_type>::pointer pointer;
};
template <typename _Tp, typename _Alloc = allocator<_Tp>> struct vector {
  typedef __normal_iterator<typename _Vector_base<_Tp, _Alloc>::pointer>
      iterator;
  vector(initializer_list<_Tp>);
};
template <typename _Callable, typename... _Args>
void invoke(_Callable __fn, _Args... __args) {
  __invoke(__fn, __args...);
}
struct _Optional_payload {
  ~_Optional_payload();
};
struct _Optional_base {
  _Optional_payload _M_payload;
};
} // namespace std
template <typename> struct lw_shared_ptr {};
template <typename T> lw_shared_ptr<T> make_lw_shared();
template <typename> struct future {};
template <typename> struct futurize {
  template <typename Func> static void invoke(Func);
};
template <typename Func, typename... Args> void futurize_invoke(Func, Args...) {
  futurize<std::invoke_result_t<Func, Args...>>::invoke(std::forward<Args>...);
}
struct gc_clock {
  using time_point = int;
};
using schema_ptr = int;
namespace db {
enum class consistency_level;
}
class nonwrapping_interval;
template <typename> using interval = nonwrapping_interval;
template <typename> using optional = std::_Optional_base;
struct wrapping_interval {
  optional<int> _end;
};
struct nonwrapping_interval {
  wrapping_interval _interval;
  static nonwrapping_interval make_singular(int);
};
template <typename T> using nonwrapping_range = interval<T>;
namespace cql3 {
class query_processor;
}
namespace dht {
using partition_range = nonwrapping_range<int>;
using partition_range_vector = std::vector<partition_range>;
} // namespace dht
template <typename> class foreign_ptr;
namespace query {
struct read_command;
}
namespace service {
namespace pager {
struct paging_state;
}
} // namespace service
namespace query {
class result;
}
struct trace_state_ptr {};
namespace std {
template <typename...> struct coroutine_traits;
template <typename = void> struct coroutine_handle {
  operator coroutine_handle<>();
};
struct suspend_never {
  bool await_ready() noexcept;
  void await_suspend(coroutine_handle<>) noexcept;
  void await_resume() noexcept;
};
} // namespace std
namespace internal {
template <typename T> struct coroutine_traits_base {
  struct promise_type {
    void unhandled_exception();
    std::suspend_never initial_suspend();
    std::suspend_never final_suspend() noexcept;
    void get_return_object() { }
  };
};
template <int, typename> struct awaiter {
  bool await_ready();
  template <typename U> void await_suspend(std::coroutine_handle<U>);
  void await_resume();
};
} // namespace internal
template <typename... T> auto operator co_await(future<T...>) {
  return internal::awaiter<true, T...>();
}
namespace coroutine {
template <typename Func> class lambda {
  Func *_func;

public:
  lambda(Func) {}
  template <typename... Args> auto operator()(Args... args) {
    std::invoke(*_func, args...);
  }
};
} // namespace coroutine
namespace std {
template <typename... T, typename... Args>
struct coroutine_traits<future<T...>, Args...>
    : internal::coroutine_traits_base<T...> {};
} // namespace std
namespace service {
struct query_state {
  trace_state_ptr get_trace_state();
  int get_client_state();
  int get_permit();
};
} // namespace service
namespace {
struct query_options {
  db::consistency_level get_consistency() const;
};
} // namespace
namespace boost {
template <class, class, class> struct basic_result;
template <class R, class S, class NoValuePolicy>
using boost_result = basic_result<R, S, NoValuePolicy>;
template <class R, class S, class NoValuePolicy>
using result = boost_result<R, S, NoValuePolicy>;
} // namespace boost
namespace utils {
template <typename...> struct exception_container;
}
namespace bo = boost;
namespace utils {
struct exception_container_throw_policy;
}
namespace exceptions {
using coordinator_exception_container = utils::exception_container<>;
template <typename T = void>
using coordinator_result = bo::result<T, coordinator_exception_container,
                                      utils::exception_container_throw_policy>;
} // namespace exceptions
namespace cql3 {
namespace statements {
schema_ptr _schema;
struct primary_key {
  int partition;
};
struct select_statement {
  template <typename>
  using coordinator_result = exceptions::coordinator_result<>;
};
struct indexed_table_select_statement : select_statement {
  future<coordinator_result<std::tuple<>>> do_execute_base_query(
      query_processor &, std::vector<primary_key> &&, service::query_state &,
      const query_options &, gc_clock::time_point,
      lw_shared_ptr<const service::pager::paging_state>) const;
};
} // namespace statements
} // namespace cql3
namespace service {
class storage_proxy;
}
namespace cql3 {
struct query_processor {
  service::storage_proxy proxy();
};
} // namespace cql3
namespace service {
struct coordinator_query_options {
  coordinator_query_options(int, int, int, trace_state_ptr);
};
struct storage_proxy {
  template <typename = void> using result = exceptions::coordinator_result<>;
  future<result<>> query_result(schema_ptr, lw_shared_ptr<query::read_command>,
                                dht::partition_range_vector,
                                db::consistency_level,
                                coordinator_query_options);
};
} // namespace service
namespace utils {
template <typename Iterator, typename Mapper, typename Reducer>
void result_map_reduce(Iterator begin, Iterator, Mapper mapper, Reducer) {
  futurize_invoke(mapper, *begin);
}
} // namespace utils
template <typename T>
using coordinator_result =
    cql3::statements::select_statement::coordinator_result<T>;
namespace cql3 {
namespace statements {
int __trans_tmp_1;
future<coordinator_result<std::tuple<>>>
indexed_table_select_statement::do_execute_base_query(
    query_processor &qp, std::vector<primary_key> &&,
    service::query_state &state, const query_options &options,
    gc_clock::time_point,
    lw_shared_ptr<const service::pager::paging_state>) const {
  auto timeout = __trans_tmp_1;
  std::vector<primary_key>::iterator key_it;
  auto key_it_end = key_it;
  void oneshot_merger();
  utils::result_map_reduce(
      key_it, key_it_end,
      coroutine::lambda(
          [&](auto key) -> future<coordinator_result<
                            foreign_ptr<lw_shared_ptr<query::result>>>> {
            auto command = make_lw_shared<query::read_command>();
            co_await qp.proxy().query_result(
                _schema, command,
                {dht::partition_range::make_singular(key.partition)},
                options.get_consistency(),
                {timeout, state.get_permit(), state.get_client_state(),
                 state.get_trace_state()});
          }),
      oneshot_merger);
 return {}; }
} // namespace statements
} // namespace cql3
