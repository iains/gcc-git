/* PR tree-optimization/115869 */
/* { dg-do run } */
/* { dg-additional-options "-floop-interchange" } */

struct st { int v; };

__attribute__((noipa)) int
f (struct st *res)
{
  res[0].v = 1;
  for (int d = 1; d < 3; d++)
    for (int i = 0; i < 2; i++)
      for (int k = 0; k + d < 3; k++)
	res[k + d].v += res[k].v;
  return res[2].v;
}

int
main ()
{
  struct st res[3] = { { 0 }, { 0 }, { 0 } };
  if (f (res) != 5)
    __builtin_abort ();
  return 0;
}
