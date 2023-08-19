
#include "new"
#include "exception"
#include "cxxabi.h"
#include "bits/cxxabi_forced.h"

const std::nothrow_t std::nothrow = std::nothrow_t{ };

// Forward some GCC versions to the the LLVM equivalent.

extern "C" {
void
__cxa_call_terminate (void*)
{
  std::terminate();
}
}

namespace std {

bool uncaught_exception () throw()
{
  return abi::__cxa_uncaught_exception ();
}

int uncaught_exceptions () noexcept
{
    return abi::__cxa_uncaught_exceptions();
}


//nested_exception::~nested_exception() noexcept
//{
//}

} // namespace std

abi::__forced_unwind::~__forced_unwind() throw() { }
