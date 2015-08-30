#ifndef CAMERA_PARAMETER_H_
#define CAMERA_PARAMETER_H_

#include <armadillo>


class CameraParameter
{
public:
	using mat=arma::mat;
	using vec3=arma::vec3;

	CameraParameter();
	CameraParameter(const mat& rho);
	operator mat() const;
	void Initial();

public:
	double focal_length_;// focal length
	vec3 rotation_; // theta phi and gamma(around x, y and z axis)
	vec3 translation_;// 3d translation between camera space and model space 
};











#endif