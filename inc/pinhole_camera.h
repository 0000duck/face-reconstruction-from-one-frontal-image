#ifndef PINHOLE_CAMERA_H_
#define PINHOLE_CAMERA_H_

#include "camera.h"
#include "armadillo"
#include "common.h"
#include "illumination.h"
#include "camera_parameter.h"

class PinholeCamera:public Camera
{
	using vec2 = arma::vec2;
	using vec3 = arma::vec3;
	using vec4 = arma::vec4;
	using mat44 = arma::mat44;

public:
	PinholeCamera(const CameraParameter& rho,int image_width = IMAGE_WIDTH, int image_height = IMAGE_HEIGHT);
	PinholeCamera(const CameraParameter& rho, const IlluminationParameter& lamda, int image_width = IMAGE_WIDTH, int image_height = IMAGE_HEIGHT);


	vec4 ModelViewTransform(const vec4& vertex_model) const override;
	vec4 OpenglTransform(const vec4 & vertex_model) const override;
	vec3 PerspectiveDivision(const vec4 & vertex_clip) const override;
	vec3 ViewportTransform(const vec3 & normalized_coordinate) const override;

	const mat44& GetModel() const override{ return model_space_to_world_space_; }
	const mat44& GetView() const override{ return world_space_to_camera_space_; }
	const mat44& GetProjection() const override{ return camera_space_to_clip_space_; }



	int Width() const override { return image_width_; }
	int Height() const override { return image_height_; }


protected:
private:

		int image_width_;
		int image_height_;
		
		vec3 shift_object_;
		vec3 camera_postion_;
		mat44 camera_orientation_;

		mat44 model_space_to_world_space_;
		mat44 world_space_to_camera_space_;
		mat44 camera_space_to_clip_space_;

		mat44 model_translation_;
		mat44 model_rotation_;

		double view_port_size_;


// assist function
	void CalculateCamera();
	void CalculateCamera(const CameraParameter& rho, const IlluminationParameter& lamda);

	void CalculateModelTranslation(const CameraParameter& rho);
	void CalculateModelMatrix(const CameraParameter& rho);
	void CalculateViewMatrix(const CameraParameter& rho);
	void CalculateViewMatrix(const CameraParameter& rho, const IlluminationParameter& lamda);

	void CalculateFrustum(const CameraParameter& rho);
	void CalculateFrustum(const CameraParameter& rho, const IlluminationParameter& lamda);

	void CalculateModelRotation(const CameraParameter& rho);

	mat44 CalculateRotation(const vec3& angle) const;

};





#endif