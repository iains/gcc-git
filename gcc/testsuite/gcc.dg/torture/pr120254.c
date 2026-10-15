/* PR tree-optimization/120254 */
/* { dg-do run } */
/* { dg-require-effective-target int32plus } */

#define N 30

unsigned A[N][N][N];

int
main ()
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      for (int k = 0; k < N; k++)
	A[i][j][k] = i + j + k;

  for (int i = 0; i < N - 1; i++)
    for (int j = 0; j < N; j++)
      for (int k = 0; k < N - 1; k++)
	A[k + 1][i][i] += A[k + 1][i][i + 1] - A[k][i][i] - 6;

  unsigned sum = 0;
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      for (int k = 0; k < N; k++)
	sum += A[i][j][k];
  if (sum != 2337195900u)
    __builtin_abort ();
  return 0;
}
