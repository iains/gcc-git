/* PR tree-optimization/109184 */
/* { dg-do run } */
/* { dg-additional-options "-floop-interchange -fno-move-loop-stores" } */

static int g_1731[7] = { 42, 0, 0, 0, 0, 0, 42 };

void __attribute__((noipa))
foo ()
{
  int l_1930[5] = { 0, };

  for (int i = 0; i < 15; ++i)
    for (int j = 4; (j >= 1); j -= 1)
#pragma GCC unroll 0
      for (int k = 0; (k <= 4); k += 1)
	g_1731[(j + 1)] = --l_1930[k];
}

int
main ()
{
  foo ();
  if (g_1731[0] != 42
      || g_1731[1] != 0 || g_1731[2] != -60 || g_1731[3] != -59
      || g_1731[4] != -58 || g_1731[5] != -57
      || g_1731[6] != 42)
    __builtin_abort ();
  return 0;
}
