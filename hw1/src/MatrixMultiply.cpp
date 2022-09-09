#include "MatrixMultiply.hpp"

#include <exception>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <numeric>

namespace ublas = boost::numeric::ublas;

scottgs::MatrixMultiply::MatrixMultiply() 
{
	;
}

scottgs::MatrixMultiply::~MatrixMultiply()
{
	;
}


scottgs::FloatMatrix scottgs::MatrixMultiply::operator()(const scottgs::FloatMatrix& lhs, const scottgs::FloatMatrix& rhs) const
{
	// Verify acceptable dimensions
	if (lhs.size2() != rhs.size1())
		throw std::logic_error("matrix incompatible lhs.size2() != rhs.size1()");

	scottgs::FloatMatrix result(lhs.size1(),rhs.size2());

	// YOUR ALGORIHM WITH COMMENTS GOES HERE:

	float sum; //use summator in order to reduce the amount of address resolving
	const long unsigned int lhs_rows = lhs.size1(); //doesn't need to resolve that value with every call
	const long unsigned int lhs_columns = lhs.size2();
	const long unsigned int rhs_columns = rhs.size2();
	const long unsigned int rhs_rows = rhs.size1();

	const float * lhs_init = &lhs(0,0); //get the initial address of the matrix  and store it as a constant
	const float * rhs_init = &rhs(0,0);

	for(long unsigned int i=0; i<lhs_rows; i++){
		for(long unsigned int j=0; j<rhs_columns; j++){
			sum = 0; //init sum to zero otherwise += operator won't work correctly
			for(long unsigned int k=0; k<rhs_rows; k++){ 
				sum += 
				*(lhs_init + k + (i*lhs_columns)) 
				//0,0 position in lhs, plus offset, plus adding the total number of columns, means you get to the next row when necessary
				
				* //multiplication operator, can be kind of hard to read with all the pointer notation in place
				
				*(rhs_init + j + (k*rhs_columns)); 
				//same as above but rhs
			}
			result(i, j) = sum; //from what I can tell, I am unable to use pointer notation to insert something at a specfic location
		}
	}
	return result;
}

scottgs::FloatMatrix scottgs::MatrixMultiply::multiply(const scottgs::FloatMatrix& lhs, const scottgs::FloatMatrix& rhs) const
{
	// Verify acceptable dimensions
	if (lhs.size2() != rhs.size1())
		throw std::logic_error("matrix incompatible lhs.size2() != rhs.size1()");

	return boost::numeric::ublas::prod(lhs,rhs);
}

