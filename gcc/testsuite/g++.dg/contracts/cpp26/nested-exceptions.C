// { dg-do run }
// { dg-options "-std=c++23 -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=observe" }

// Test that we produce the correct exception object when
// evaluation_exception () is queried from within a user violation
// handler that throws internally.

#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <cxxabi.h>
#include <contracts>
#include <cassert>

#ifdef OUTPUT
# include <iostream>
#endif

using namespace std;

unsigned vh_catches = 0;
bool die = false;

int
test_func (int x, int *p)
  pre (p != nullptr)
  pre ((p == nullptr)
       ? throw std::out_of_range{"nullptr is not allowed"}
       : *p < 100)
  post (r: r < 10)
{
  if (die)
    std::terminate ();
  return x + *p;
}

using namespace std::contracts;

int
main ()
{
  /* We should detect the case that we cannot continue and exit.  */
  std::set_terminate
  ( []()
    {
#ifdef OUTPUT
      std::cerr << "Punting ... catches = " << vh_catches << "\n";
#endif
      assert (vh_catches == 4);
      std::exit (0);
    });

  int c = 0, d = 0;
  try {
    int a = 4;
    int b = 54321;
    // pre OK, pre FAIL post FAIL
    d = test_func (a, &b);
    // pre FAIL, pre THROW post NOT_REACHED.
    c = test_func (a, nullptr);
  } catch (...) { // #1
#ifdef OUTPUT
    std::cerr << "something bad happened \n";
#endif
    __builtin_abort ();
  }
  // If we reach here, the testcase will fail.
  return c + d;
}

/* A custom handler - that throws and catches internally. */
void
handle_contract_violation (const std::contracts::contract_violation &violation)
{
  try {
    throw "oops";
  } catch (...) {
    ++vh_catches;
    // This should be the string thrown above.
    std::exception_ptr other = std::current_exception ();
    // (if it is present) this should be the runtime error thrown by
    // the pre
    std::exception_ptr epr = violation.evaluation_exception ();
    if (epr)
      {
#ifdef OUTPUT
	const type_info *ti = epr.__cxa_exception_type();
	const char *n = ti->name();
	int status = -1;
	char *dem = __cxxabiv1::__cxa_demangle(n, 0, 0, &status);
	std::cerr << "evaluation exception type: " << dem << "\n";
#endif
	die = true;
      }
    else
#ifdef OUTPUT
      std::cerr << "no evaluation_exception\n";
#endif
    assert (other);
#ifdef OUTPUT
    const type_info *ti = other.__cxa_exception_type();
    const char *n = ti->name();
    int status = -1;
    char *dem = __cxxabiv1::__cxa_demangle(n, 0, 0, &status);
    std::cerr << "other exception was : " << dem << "\n";
#endif
  }
  invoke_default_contract_violation_handler (violation);
}
