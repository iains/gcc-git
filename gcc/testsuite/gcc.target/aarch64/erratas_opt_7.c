/* { dg-do link } */
/* { dg-additional-options "-mcpu=neoverse-v1 -march=armv8-a -mfix-cortex-a53-835769 -###" } */

int main()
{
  return 0;
}

/* The input is conflicting, but take cpu over arch.  */
/* { dg-message "-mno-fix-cortex-a53-835769" "note" { target *-*-* } 0 } */
/* { dg-excess-errors "" } */
