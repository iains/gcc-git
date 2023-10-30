// { dg-options "-std=c++26 -fcontracts -fcontracts-nonattr" }

int foo (int x)
pre [[unlikley]] ( x > 41 )  // { dg-warning "attributes are ignored on function contract specifiers" }
{
  return x + 1;
}

int main(int ac, char *av[])
{
   int i = ac;
   contract_assert [[deprecated]] (i == 3);  // { dg-warning "attributes are ignored on contract assertions" }
  return i;
}
