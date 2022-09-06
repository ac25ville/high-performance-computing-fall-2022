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

	const long unsigned int lhs_rows = lhs.size1();
	const long unsigned int rhs_columns = rhs.size2();
	const long unsigned int rhs_rows = rhs.size1();

	for(long unsigned int i=0; i<lhs_rows; i++){
		for(long unsigned int j=0; j<rhs_columns; j++){
			sum = 0; //init sum to zero otherwise += operator won't work correctly
			for(long unsigned int k=0; k<rhs_rows; k++){ sum += lhs(i, k) * rhs(k, j); }
			result(i, j) = sum;
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

