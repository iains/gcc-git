// mutex -*- C++ -*-

// Copyright (C) 2008-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

#include <atomic>
#include <mutex>

#ifdef _GLIBCXX_HAS_GTHREADS

namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

// This calls the trampoline lambda, passing the address of the closure
// repesenting the original function and its arguments.
void
once_flag::__do_call_once (void (*func)(void*), void *arg)
{
  __gthread_mutex_lock(&_M_mutx_);
  while (this->_M_state_.load (std::memory_order_relaxed) == 1)
    __gthread_cond_wait(&_M_condv_, &_M_mutx_);

  // mutex locked, the most likely outcome is that the once-call completed
  // on some other thread, so we are done.
  if (_M_state_.load (std::memory_order_acquire) == 2)
    {
      __gthread_mutex_unlock(&_M_mutx_);
      return;
    }

  // mutex locked; if we get here, we expect the state to be 0, this would
  // correspond to an exception throw by the previous thread that tried to
  // do the once_call.
  __glibcxx_assert (_M_state_.load (std::memory_order_acquire) == 0);

  try
    {
      // mutex locked.
      _M_state_.store (1, std::memory_order_relaxed);
      __gthread_mutex_unlock (&_M_mutx_);
      func (arg);
      // We got here without an exception, so the call is done.
      // If the underlying implementation is pthreads, then it is possible
      // to trigger a sequence of events where wake-ups are lost - unless the
      // mutex associated with the condition var is locked around the relevant
      // broadcast (or signal).
      __gthread_mutex_lock(&_M_mutx_);
      _M_state_.store (2, std::memory_order_release);
      __gthread_cond_broadcast (&_M_condv_);
      __gthread_mutex_unlock (&_M_mutx_);
    }
  catch (...)
    {
      // mutex unlocked.
      // func raised an exception, let someone else try ...
      // See above.
      __gthread_mutex_lock(&_M_mutx_);
      _M_state_.store (0, std::memory_order_release);
      __gthread_cond_broadcast (&_M_condv_);
      __gthread_mutex_unlock (&_M_mutx_);
      // ... and pass the exeception to our caller.
      throw;
    }
}

#if !_GLIBCXX_INLINE_VERSION

// Unless we have a versioned library, provide the symbols for the previous
// once call impl.

#ifdef _GLIBCXX_HAVE_TLS
  __thread void* __once_callable;
  __thread void (*__once_call)();

  extern "C" void __once_proxy()
  {
    // The caller stored a function pointer in __once_call. If it requires
    // any state, it gets it from __once_callable.
    __once_call();
  }

#else // ! TLS

  // Explicit instantiation due to -fno-implicit-instantiation.
  template class function<void()>;

  function<void()> __once_functor;

  mutex&
  __get_once_mutex()
  {
    static mutex once_mutex;
    return once_mutex;
  }

namespace
{
  // Store ptr in a global variable and return the previous value.
  inline unique_lock<mutex>*
  set_lock_ptr(unique_lock<mutex>* ptr)
  {
    static unique_lock<mutex>* __once_functor_lock_ptr = nullptr;
    return std::__exchange(__once_functor_lock_ptr, ptr);
  }
}

  // code linked against ABI 3.4.12 and later uses this
  void
  __set_once_functor_lock_ptr(unique_lock<mutex>* __ptr)
  {
    (void) set_lock_ptr(__ptr);
  }

  // unsafe - retained for compatibility with ABI 3.4.11
  unique_lock<mutex>&
  __get_once_functor_lock()
  {
    static unique_lock<mutex> once_functor_lock(__get_once_mutex(), defer_lock);
    return once_functor_lock;
  }

  // This is called via pthread_once while __get_once_mutex() is locked.
  extern "C" void
  __once_proxy()
  {
    // Get the callable out of the global functor.
    function<void()> callable = std::move(__once_functor);

    // Then unlock the global mutex
    if (unique_lock<mutex>* lock = set_lock_ptr(nullptr))
    {
      // Caller is using the new ABI and provided a pointer to its lock.
      lock->unlock();
    }
    else
      __get_once_functor_lock().unlock();  // global lock

    // Finally, invoke the callable.
    callable();
  }
#endif // ! TLS
#endif // ! _GLIBCXX_INLINE_VERSION
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std

#endif // _GLIBCXX_HAS_GTHREADS
