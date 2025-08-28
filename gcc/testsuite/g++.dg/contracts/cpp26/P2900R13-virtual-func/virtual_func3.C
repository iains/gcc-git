// { dg-do run }
// { dg-options "-std=c++2a -fcontracts -fcontracts-nonattr -fcontract-evaluation-semantic=observe -fcontracts-on-virtual-functions=P2900R13" }
template<typename T>
struct Base
{
  virtual char f(const int a) pre(a > 5) post( i > 4 ){ return 'b';}
private:
  int i = 2;
};

template<typename T>
struct Child1 : Base<T>
{
  virtual char f(const int a) pre(a > 3){ return 'c'; }
};
template<typename T>
void foo(Base<T>& b){
  b.f(1);
}


int main()
{
  Child1<int> c;
  foo(c);

}
// { dg-output "contract violation in function char Base<T>::f.int. .with T = int. at .*: a > 5.*(\n|\r\n|\r)" }
// { dg-output "contract violation in function char Child1<T>::f.int. .with T = int. at .*: a > 3.*(\n|\r\n|\r)" }
// { dg-output "contract violation in function char Base<T>::f.int. .with T = int. at .*: i > 4.*(\n|\r\n|\r)" }
