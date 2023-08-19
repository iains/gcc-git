// Exception Handling support header (exception_ptr class) for -*- C++ -*-

// This is a version that abstracts the facilities provided by libc++abi.

/** @file bits/exception_ptr.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{exception}
 */

#ifndef _EXCEPTION_PTR_H
#define _EXCEPTION_PTR_H

#include <bits/c++config.h>
#include <bits/exception_defines.h>
#include <bits/cxxabi_init_exception.h>
//#if __cplusplus >= 201103L
//# include <bits/move.h>
//# include <new>
//#endif

#define _LIBCPP_VISIBILITY(vis) __attribute__((__visibility__(vis)))
#define _LIBCPP_HIDDEN _LIBCPP_VISIBILITY("hidden")
#define _LIBCPP_FUNC_VIS _LIBCPP_VISIBILITY("default")
#define _LIBCPP_TYPE_VIS _LIBCPP_VISIBILITY("default")
#define _LIBCPP_TEMPLATE_DATA_VIS _LIBCPP_VISIBILITY("default")
#define _LIBCPP_EXPORTED_FROM_ABI _LIBCPP_VISIBILITY("default")
#define _LIBCPP_EXCEPTION_ABI _LIBCPP_VISIBILITY("default")
#define _LIBCPP_EXTERN_TEMPLATE_TYPE_VIS _LIBCPP_VISIBILITY("default")
#define _LIBCPP_CLASS_TEMPLATE_INSTANTIATION_VIS
#define _LIBCPP_INLINE_VISIBILITY
#define _LIBCPP_NORETURN __attribute__((noreturn))

extern "C++" {

namespace std _GLIBCXX_VISIBILITY(default)
{

class exception_ptr;

exception_ptr current_exception() _GLIBCXX_USE_NOEXCEPT;
_GLIBCXX_NORETURN void rethrow_exception(exception_ptr);

class _LIBCPP_TYPE_VIS exception_ptr
{
    void* __ptr_;
public:
    _LIBCPP_INLINE_VISIBILITY exception_ptr() _GLIBCXX_USE_NOEXCEPT
      : __ptr_() {}
   exception_ptr(const exception_ptr&) _GLIBCXX_USE_NOEXCEPT;

   explicit exception_ptr(void* __e) _GLIBCXX_USE_NOEXCEPT;

#if __cplusplus >= 201103L
   exception_ptr(nullptr_t) _GLIBCXX_USE_NOEXCEPT
      : __ptr_(nullptr) {}

   exception_ptr(exception_ptr&& __o) noexcept
      : __ptr_(__o.__ptr_)
      { __o.__ptr_ = nullptr; }

   exception_ptr& 
    operator=(exception_ptr&& __o) noexcept
      {
        exception_ptr(static_cast<exception_ptr&&>(__o)).swap(*this);
        return *this;
      }
#endif
     exception_ptr& operator=(const exception_ptr&) _GLIBCXX_USE_NOEXCEPT;

    ~exception_ptr() _GLIBCXX_USE_NOEXCEPT;

   void 
   swap(exception_ptr&) _GLIBCXX_USE_NOEXCEPT;

#if __cplusplus >= 201103L
    explicit operator bool() const _GLIBCXX_USE_NOEXCEPT
    {return __ptr_ != nullptr;}
#endif

    friend _LIBCPP_INLINE_VISIBILITY
    bool operator==(const exception_ptr& __x, const exception_ptr& __y) _GLIBCXX_USE_NOEXCEPT
        {return __x.__ptr_ == __y.__ptr_;}

    friend _LIBCPP_INLINE_VISIBILITY
    bool operator!=(const exception_ptr& __x, const exception_ptr& __y) _GLIBCXX_USE_NOEXCEPT
        {return !(__x == __y);}

    friend _LIBCPP_FUNC_VIS exception_ptr current_exception() _GLIBCXX_USE_NOEXCEPT;
    friend _LIBCPP_FUNC_VIS void rethrow_exception(exception_ptr);
};

__attribute__((__used__))
inline void
exception_ptr::swap(exception_ptr &__other) _GLIBCXX_USE_NOEXCEPT
{
      void *__tmp = __ptr_;
      __ptr_ = __other.__ptr_;
      __other.__ptr_ = __tmp;
}

#if __cpp_exceptions
  template<class _Ep>
  exception_ptr
  make_exception_ptr(_Ep __e) _GLIBCXX_USE_NOEXCEPT
  {
    // Use the runtime to initialise it.
    try { throw __e; }
    catch (...) { return current_exception(); }
  }
#else
  // This is always_inline so the linker will never use this useless definition
  // instead of a working one compiled with exceptions enabled.
  template<typename _Ex>
    __attribute__ ((__always_inline__))
    inline exception_ptr
    make_exception_ptr(_Ex) _GLIBCXX_USE_NOEXCEPT
    { return exception_ptr(); }
#endif

  /// @} group exceptions
} // namespace std

} // extern "C++"

#endif
