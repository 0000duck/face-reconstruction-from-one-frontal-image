#ifndef PHONG_ILLUMINATION_H_
#define PHONG_ILLUMINATION_H_


#include "illumination.h"
#include "entity.h"
#include "illumination_parameter.h"


class PhongIllumination:public Illumination
{
	using mat33 = arma::mat33;
	using vec3 = arma::vec3;

public:
	PhongIllumination(const IlluminationParameter& lamda);
	void Illuminate(Pixel& pixel, const Vertex& triangle_center) const override;


protected:
private:

	// variable member
	vec3 ambient_color_;
	vec3 direct_color_;
	vec3 direct_direction_;  // surface to light
	double color_contrast_;
	vec3 color_offset_;
	vec3 color_gain_;

};





#endif