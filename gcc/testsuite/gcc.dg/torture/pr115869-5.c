/* PR tree-optimization/115869 */
/* { dg-do run } */
/* { dg-require-effective-target int32plus } */
/* { dg-additional-options "-floop-interchange -fno-loop-unroll-and-jam" } */

unsigned a[64];
unsigned B[64][64];

void __attribute__((noipa))
foo (int ni, int nj, int nk)
{
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nj; j++)
      for (int k = 0; k < nk; k++)
	a[i + k] = a[i + k] * 3 + B[j][i];
}

int
main ()
{
  for (int j = 0; j < 64; j++)
    for (int i = 0; i < 64; i++)
      B[j][i] = i * 7 + j * 13 + 1;
  for (int x = 0; x < 64; x++)
    a[x] = x;
  foo (2, 2, 2);
  unsigned s = 0;
  for (int x = 0; x < 64; x++)
    s = s * 31 + a[x];
  if (s != 3050518888u)
    __builtin_abort ();
  return 0;
}
