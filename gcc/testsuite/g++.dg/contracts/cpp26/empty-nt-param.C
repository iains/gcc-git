// check that we do not ICE with an empty nontrivial parameter
// { dg-do run }
// { dg-options "-std=c++23 -fcontracts " }

struct NTClass {
  NTClass(){};
  NTClass(const NTClass&){}
  ~NTClass(){};
};

struct Empty {};

void f (const NTClass i) pre (true){
}

void g (const Empty i) pre (true){
}
int main(){

  NTClass n;
  f(n);
  g(Empty{});


}
