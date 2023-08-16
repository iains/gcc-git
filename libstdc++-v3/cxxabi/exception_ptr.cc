#include "cxxabi.h"
#include "exception"

// Implementation for exception pointers.

extern "C++" {

namespace std _GLIBCXX_VISIBILITY(default)
{

exception_ptr::~exception_ptr() noexcept {
  abi::__cxa_decrement_exception_refcount(__ptr_);
}

exception_ptr::exception_ptr(const exception_ptr& other) noexcept
    : __ptr_(other.__ptr_)
{
    abi::__cxa_increment_exception_refcount(__ptr_);
}

exception_ptr& exception_ptr::operator=(const exception_ptr& other) noexcept
{
    if (__ptr_ != other.__ptr_)
    {
        abi::__cxa_increment_exception_refcount(other.__ptr_);
        abi::__cxa_decrement_exception_refcount(__ptr_);
        __ptr_ = other.__ptr_;
    }
    return *this;
}

exception_ptr current_exception() noexcept
{
    // be nicer if there was a constructor that took a ptr, then
    // this whole function would be just:
    //    return exception_ptr(__cxa_current_primary_exception());
    exception_ptr ptr;
    ptr.__ptr_ = abi::__cxa_current_primary_exception();
    return ptr;
}


__attribute__((__noreturn__))
void rethrow_exception(exception_ptr p)
{
    abi::__cxa_rethrow_primary_exception(p.__ptr_);
    // if p.__ptr_ is NULL, above returns so we terminate
    terminate();
}

} // namespace std
} // extern Cxx
