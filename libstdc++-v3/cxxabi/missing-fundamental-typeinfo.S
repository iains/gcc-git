
// This adds the fundamental typeinfo for types missing from libc++abi.

#if defined (__APPLE__)
# if defined (__arm64__)
#   include "missing-fti-arm64.h"
# elif defined (__x86_64__)
#   include "missing-fti-x86_64.h"
# elif defined (__i386__)
#   include "missing-fti-i386.h"
# elif defined (__ppc64__)
#   include "missing-fti-ppc64.h"
# elfi defined (__ppc__)
#   include "missing-fti-ppc.h"
# else
#   error "unexpected Darwin arch"
# endif
#endif
