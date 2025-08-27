//Test that noexcept-observe with a non throwing violation handler has
// observe semantics
// { dg-do run { target c++26 } }
// { dg-additional-options "-fcontracts -fcontract-evaluation-semantic=noexcept_observe " }

int f(const int a, const int b) pre (a > 2) post(r : r > 2){ return 1;};

int main(int, char**)
{
  f(1,1);
  return 0;
}

// { dg-output "contract violation in function int f.int, int. at .*: a > 2.*(\n|\r\n|\r)" }
// { dg-output "contract violation in function int f.int, int. at .*: r > 2.*(\n|\r\n|\r)" }
