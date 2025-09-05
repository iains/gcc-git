// { dg-do run }
// { dg-options "-std=c++2a -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=noexcept_enforce -fcontracts-disable-predicate-exception-translation" }

#include <exception>
#include <cstdlib>

void my_term()
{
  std::exit(0);
}


bool check(int i){
  if (i > 10)
    throw 3;

  return true;
}

void f() pre(check(15)){} // { dg-warning "ignored when the evaluation semantic is .noexcept_enforce. or .noexcept_ignore." }


int main(int, char**)
{
  std::set_terminate (my_term);
  f();
  return 1;
}

// { dg-output "contract violation in function void f.. at .*: check.15..*(\n|\r\n|\r)" }
// { dg-output ".assertion_kind: pre, semantic: unknown: 1001, mode: evaluation_exception: threw an instance of .int., terminating: no.*(\n|\r\n|\r)" }
