// check that we do not optimise based on contract_asssert
// { dg-options "-O3 -std=c++23 -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=quick_enforce -fdump-tree-optimized " }
// { dg-do compile }

[[gnu::used, gnu::noinline]]
inline int* f(int *y) {
  contract_assert(y != nullptr);
  return y;
}


int foo(int *x) {
    if (f(x) == nullptr)
    {
        return 0;
    }
    return *x;
}

// Check that the if condition in foo isn't optimised away
// { dg-final { scan-tree-dump "if \\(_. == 0B\\)" "optimized" } }
