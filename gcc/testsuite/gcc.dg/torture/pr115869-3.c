/* PR tree-optimization/115869 */
/* { dg-do run } */
/* { dg-require-effective-target int32plus } */
/* { dg-additional-options "-floop-interchange" } */

unsigned a[16];
unsigned B[8][1024];

void __attribute__((noipa))
foo (int nf, int ni, int nj)
{
  for (int f = 0; f < nf; f++)
    for (int i = 0; i < ni; i++)
      for (int j = 0; j < nj; j++)
	a[i - j + 8] = a[i - j + 8] * 3 + B[i][f];
}

int
main ()
{
  for (int i = 0; i < 8; i++)
    for (int x = 0; x < 1024; x++)
      B[i][x] = x * 7 + i * 13 + 1;
  for (int x = 0; x < 16; x++)
    a[x] = x;
  foo (2, 2, 2);
  unsigned s = 0;
  for (int x = 0; x < 16; x++)
    s = s * 31 + a[x];
  if (s != 2738069132u)
    __builtin_abort ();
  return 0;
}
