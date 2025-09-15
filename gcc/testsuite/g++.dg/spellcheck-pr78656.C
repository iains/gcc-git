// { dg-options "-fdiagnostics-show-caret" }

#include <memory>

void* allocate(std::size_t n)
{
  // { dg-error ".allocate. is not a member of .std.; did you mean 'relocate'\\?" "" { target c++26 } .+1 }
  return std::allocate<char>().allocate(n); // { dg-error ".allocate. is not a member of .std.; did you mean 'allocator'\\?" "" { target c++23_down } }
  /* { dg-begin-multiline-output "" }
   return std::allocate<char>().allocate(n);
               ^~~~~~~~
               allocator
     { dg-end-multiline-output "" { target c++23_down } } */
  /* { dg-begin-multiline-output "" }
   return std::allocate<char>().allocate(n);
               ^~~~~~~~
               relocate
     { dg-end-multiline-output "" { target c++26 } } */

  // Various errors follow that we don't care about; suppress them:
  // { dg-excess-errors "8: " }
}

void* test_2(std::size_t n)
{
  return std::alocator<char>().allocate(n); // { dg-error ".alocator. is not a member of .std.; did you mean 'allocator'\\?" }
  /* { dg-begin-multiline-output "" }
   return std::alocator<char>().allocate(n);
               ^~~~~~~~
               allocator
     { dg-end-multiline-output "" } */

  // Various errors follow that we don't care about; suppress them:
  // { dg-excess-errors "26: " }
}
