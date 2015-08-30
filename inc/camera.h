#ifndef CAMERA_H_
#define CAMERA_H_

#include <armadillo>


class Camera
{

	using vec4= arma::vec4 ;
	using vec3= arma::vec3 ;
	using mat44 = arma::mat44;

public:
	virtual ~Camera(){}

	virtual vec4 ModelViewTransform(const vec4& vertex_model) const = 0;
	virtual vec4 OpenglTransform(const vec4 & vertex_model) const = 0;
	virtual vec3 PerspectiveDivision(const vec4 & vertex_clip) const = 0;
	virtual vec3 ViewportTransform(const vec3 & normalized_coordinate) const = 0;


	virtual const mat44& GetModel() const= 0;
	virtual const mat44& GetView() const= 0;
	virtual const mat44& GetProjection() const= 0;


	virtual int Width() const = 0;
	virtual int Height() const = 0;



protected:
private:
};












#endif