// { dg-do compile { target c++23 } }
// { dg-additional-options "-fcontracts " }

struct contract
{
  int checked = 0;
};

contract a, b;

bool
checkA ()
{
  a.checked++;
  return true;
}

bool
checkB ()
{
  b.checked++;
  return true;
}

void
clear_checks ()
{
  a.checked = b.checked = 0;

}

struct S
{
  friend int f1(S) post (checkB());
  friend int f1(S) pre (checkA()){ return 1;}; // { dg-error "mismatched contract" }
};
