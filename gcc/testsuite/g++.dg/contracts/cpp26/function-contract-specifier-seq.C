// generic function-contract-specifier-seq parsing checks
// N5008
// function-contract-specifier-seq :
//  	function-contract-specifier function-contract-specifier-seq opt
// function-contract-specifier :
//	precondition-specifier
// 	postcondition-specifier
// precondition-specifier : pre attribute-specifier-seq opt ( conditional-expression )
// postcondition-specifier :
//	post attribute-specifier-seq opt ( result-name-introducer opt conditional-expression )
// { dg-do compile { target c++23 } }
// { dg-additional-options "-fcontracts" }
// { dg-additional-options "-fcontracts" }

static_assert (__cpp_contracts >= 202502L);

void f(int i) pre (i>3);

int g(int j) post (r:r>4) pre (j !=0);
