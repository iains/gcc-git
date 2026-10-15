/* PR tree-optimization/105771 */
/* { dg-do run } */
/* { dg-additional-options "-floop-unroll-and-jam" } */

struct st { unsigned v; };

__attribute__((noipa)) void
f (struct st *p, int m, int n, int d)
{
  for (int i = 0; i < m; i++)
    for (int k = 0; k + d < n; k++)
      p[k + d].v = p[k + d].v * 3 + p[k].v + (unsigned) i;
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
  for (int j = 0; j < 16; j++)
    p[j].v = q[j] = (unsigned) j + 1;
  f (p, 8, 16, 1);
  g (q, 8, 16, 1);
  for (int j = 0; j < 16; j++)
    if (p[j].v != q[j])
      __builtin_abort ();
  return 0;
}
