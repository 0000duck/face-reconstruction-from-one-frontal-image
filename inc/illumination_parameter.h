#ifndef ILLUMINATION_PARAMETER_H_
#define ILLUMINATION_PARAMETER_H_

#include "armadillo"


class IlluminationParameter
{
public:
	using vec3 = arma::vec3;
	using vec2 = arma::vec2;
	using mat=arma::mat;

	IlluminationParameter();
	IlluminationParameter(const mat& lamda);
	operator mat() const;

	void Initial();
	static vec3 Angle2Direction(const vec2& angle);
	static vec2 Direction2Angle(const vec3& direction) ;

public:
	vec3 ambient_color_;
	vec3 direct_color_;
	vec3 direct_direction_;  // surface to light
	double color_contrast_;
	vec3 color_offset_;
	vec3 color_gain_;
};





#endif