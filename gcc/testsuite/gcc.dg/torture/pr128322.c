/* { dg-do run } */
/* { dg-require-effective-target int32plus } */

struct A { unsigned long long lo : 32, y : 19, rest : 13; };
struct BF { unsigned long long x0:4, x1:8, x2:31, x3:3, x4:4, x5:14; };
union U { unsigned char b[8]; struct A a; struct BF bf; };

__attribute__((noipa)) unsigned
f(union U *p, unsigned v)
{
  p->b[6] = 209;
  p->a.y = v;
  p->b[5] = 39;
  return p->bf.x4;
}

int main()
{
  static union U x;
  unsigned r = f(&x, 0x30000);
  if (r != 12)
    __builtin_abort();
  return 0;
}
