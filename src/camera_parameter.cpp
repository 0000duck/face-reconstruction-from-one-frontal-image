#include "camera_parameter.h"
#include "common.h"

using namespace arma;


CameraParameter::CameraParameter()
{
	Initial();
}

CameraParameter::CameraParameter(const mat& rho)
{
	rotation_= rho.rows(0, 2);
	translation_= rho.rows(3, 5);
	focal_length_ = rho[6];
}

CameraParameter::operator mat() const
{
	mat rho(RhoNum,1);
	rho.rows(0, 2) = rotation_;
	rho.rows(3, 5) = translation_;
	rho[6] = focal_length_;
	return rho;
}

void CameraParameter::Initial()
{
	// default camera parameter

	// -10~10  variance=5 
	rotation_[0] = 0;  // theta  x 
	rotation_[1] = 0;  // phi  y
	rotation_[2] = 0;  // gamma  z

	// -200~200 variance=50
	translation_[0] = 0;
	translation_[1] = 0;
	translation_[2] = 0;

	// 300~700 variance=30
	focal_length_ = 6000;  //2343.75


	// alignment test
	
	//// -10~10  variance=5 
	//rotation_[0] = -0.45897;  // gamma   du
	//rotation_[1] = 2.8864;  // phi
	//rotation_[2] = -2.6295;  // theta

	//// -200~200 variance=50
	//translation_[0] = -13.88;
	//translation_[1] = -181;
	//translation_[2] = 29.875;

	//// 300~700 variance=30
	//focal_length_ = 436.98;

}

