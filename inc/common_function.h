#ifndef COMMON_FUNCTION_H_
#define COMMON_FUNCTION_H_

#include <armadillo>


inline arma::vec3 Normalize(const arma::vec3& input)
{
	return input / arma::norm(input, 2);
}





#endif