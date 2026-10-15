/* PR tree-optimization/115869 */
/* { dg-do run } */
/* { dg-additional-options "-floop-unroll-and-jam" } */

struct st { unsigned v[4]; };

__attribute__((noipa)) void
f (struct st *p, int m, int n, int d, int j)
{
  for (int i = 0; i < m; i++)
    for (int k = 0; k + d < n; k++)
      p[k + d].v[j] = p[k + d].v[j] * 3 + p[k].v[j] + (unsigned) i;
}

__attribute__((noipa)) void
g (volatile unsigned *q, int m, int n, int d)
{
  for (int i = 0; i < m; i++)
    for (int k = 0; k + d < n; k++)
      q[k + d] = q[k + d] * 3 + q[k] + (unsigned) i;
}

int
main ()
{
  struct st p[16];
  unsigned q[16];
  for (int k = 0; k < 16; k++)
    {
      for (int e = 0; e < 4; e++)
	p[k].v[e] = (unsigned) (k * 4 + e);
      q[k] = p[k].v[2];
    }
  f (p, 8, 16, 1, 2);
  g (q, 8, 16, 1);
  for (int k = 0; k < 16; k++)
    if (p[k].v[2] != q[k])
      __builtin_abort ();
  return 0;
}
