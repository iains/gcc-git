/* PR tree-optimization/115869 */
/* { dg-do run } */
/* { dg-require-effective-target int32plus } */

unsigned A0[16];
unsigned B[6][1024];

void __attribute__((noipa))
foo (int n0, int n1, int n2, int n3)
{
  for (int i0 = 0; i0 < n0; i0++)
    for (int i1 = 0; i1 < n1; i1++)
      for (int i2 = 0; i2 < n2; i2++)
	for (int i3 = 0; i3 < n3; i3++)
	  A0[i1 + i2 + i3]
	    = A0[i1 + i2 + i3] * 3u + B[i0][i1] + (unsigned) i3;
}

int
main ()
{
  for (unsigned x = 0; x < 6 * 1024; x++)
    ((unsigned *) B)[x] = x * 40503u + 117u;
  for (unsigned x = 0; x < 16; x++)
    A0[x] = x * 2654435761u + 117u;
  foo (1, 1, 4, 2);
  unsigned s = 0;
  for (int x = 0; x < 16; x++)
    s = s * 31 + A0[x];
  if (s != 638080400u)
    __builtin_abort ();
  return 0;
}
