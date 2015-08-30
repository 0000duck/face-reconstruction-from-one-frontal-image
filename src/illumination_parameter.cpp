#include "illumination_parameter.h"
#include "common.h"
#include "opengl_common.h"
#include "common_function.h"
using namespace arma;


IlluminationParameter::IlluminationParameter() 
{
	Initial();
}


IlluminationParameter::IlluminationParameter(const mat& lamda)
{
	direct_direction_ = Angle2Direction(lamda.rows(0,1));
	direct_color_ = lamda.rows(2, 4);
	ambient_color_ = lamda.rows(5, 7);
	color_contrast_ = lamda[8];
	color_gain_ = lamda.rows(9, 11);
	color_offset_ = lamda.rows(12, 14);
}


IlluminationParameter::operator mat() const
{
	mat lamda(LamdaNum, 1);
	lamda.rows(0, 1) = Direction2Angle(direct_direction_);
	lamda.rows(2, 4) = direct_color_;
	lamda.rows(5, 7) = ambient_color_;
	lamda[8] = color_contrast_;
	lamda.rows(9, 11) = color_gain_;
	lamda.rows(12, 14) = color_offset_;
	return lamda;
}


void IlluminationParameter::Initial()
{
	// default illumination parameter

	direct_direction_[0] = cos(PI/9)*sin(0);
	direct_direction_[1] = sin(PI/9);
	direct_direction_[2] = cos(PI/9)*cos(0);


	direct_color_[0] = 0.6;
	direct_color_[1] = 0.6;
	direct_color_[2] = 0.6;

	ambient_color_[0] = 0.6;
	ambient_color_[1] = 0.6;
	ambient_color_[2] = 0.6;

	color_contrast_ = 1;

	color_gain_[0] = 1;
	color_gain_[1] = 1;
	color_gain_[2] = 1;

	color_offset_[0] = 0;
	color_offset_[1] = 0;
	color_offset_[2] = 0;
}


vec3 IlluminationParameter::Angle2Direction(const vec2& angle) 
{
	vec3 direction;
	direction[0] = cos(angle[0])*sin(angle[1]); // x
	direction[1] = sin(angle[0]); // y
	direction[2] = cos(angle[0])*cos(angle[1]); // z

	return direction;
}


vec2 IlluminationParameter::Direction2Angle(const vec3& direction) 
{
	vec3 direction_normalized = Normalize(direction);

	vec2 angle;

	angle[0] = asin(direction_normalized[1]); // theta_l 
	angle[1] = atan(direction_normalized[0] / direction_normalized[2]); // phi_l

	return angle;
 }