
#include "pinhole_camera.h"
#include "opengl_common.h"
#include "common_function.h"

#include <iostream>
#include <math.h>

using namespace arma;


PinholeCamera::PinholeCamera(const CameraParameter& rho, int image_width, int image_height)
:image_width_(image_width), image_height_(image_height)
{
	CalculateModelMatrix(rho);
	CalculateViewMatrix(rho);
	CalculateFrustum(rho);
}
PinholeCamera::PinholeCamera(const CameraParameter& rho,
											const IlluminationParameter& lamda,
											int image_width,
											int image_height) :image_width_(image_width), image_height_(image_height)
																	
{
	CalculateModelMatrix(rho);
	CalculateViewMatrix(rho,lamda);
	CalculateFrustum(rho,lamda);
}

void PinholeCamera::CalculateCamera()
{
	camera_postion_[0] = 0;
	camera_postion_[1] = 0;
	camera_postion_[2] =CameraPosition;

    camera_orientation_.eye();
	camera_orientation_.at(0,0)=1.0;// right vector  +x   (left hand)
	camera_orientation_.at(1,1)=1.0;// up vector  +y
	camera_orientation_.at(2,2)=1.0;// negative forward vector +z
}


void PinholeCamera::CalculateCamera(const CameraParameter& rho, const IlluminationParameter& lamda)
{
	camera_postion_ = rho.translation_ + (CameraPosition + CameraBias)*Normalize(lamda.direct_direction_);
	vec3 forward = Normalize(-lamda.direct_direction_);
	vec3 right;
	vec3 up;

	if (norm(right,2)==0)
	{
		up[0] = 0;
		up[1] = 1;
		up[2] = 0;

		right[0] = 1;
		right[1] = 0;
		right[2] = 0;
	}
	else
	{
		up[0] = 0;
		up[1] = 1;
		up[2] = 0;
		right = Normalize(cross(forward, up));
		up = Normalize(cross(right, forward));
	}
	camera_orientation_.eye();
	camera_orientation_.submat(0, 0, 0, 2) = right.t();
	camera_orientation_.submat(1, 0, 1, 2) = up.t();
	camera_orientation_.submat(2, 0, 2, 2) = -forward.t();
}


void PinholeCamera::CalculateModelTranslation(const CameraParameter& rho)
{
	model_translation_.eye();
	model_translation_.submat(0, 3, 2, 3) = rho.translation_;
}


/**
* @brief      transform from model space to camera space
* @param          Input    -- vertex in model space
* @param          Input    -- camera parameter
* @return     vertex in camera space
*/
vec4 PinholeCamera::ModelViewTransform(const vec4& vertex_model) const 
{
	//model_space_to_world_space.print();
	//world_space_to_camera_space.print();

	// V*M*(vertex_position)
	return world_space_to_camera_space_*model_space_to_world_space_*vertex_model;
}


/**
* @brief      transform from model space to clip space
* @param          Input    -- vertex position in model space
* @param          Input    -- camera parameter
* @return          vertex position in clip space
*/
vec4 PinholeCamera::OpenglTransform(const vec4 & vertex_model) const
{
	mat44 model_space_to_world_space;
	mat44 world_space_to_camera_space;
	mat44 camera_space_to_clip_space;
	//mat44 pvm=camera_space_to_clip_space*world_space_to_camera_space*model_space_to_world_space;
	//pvm.print();

	// P*V*M*(vertex_position)
	return camera_space_to_clip_space_*world_space_to_camera_space_*model_space_to_world_space_*vertex_model;
}


/**
* @brief      transform from clip space to device space 
* @param          Input    -- vertex position in clip space
* @param          Input    -- 
* @return     vertex position in device coordinate
*/
vec3 PinholeCamera::PerspectiveDivision(const vec4 & vertex_clip) const 
{
	vec3 res;
	res[0]=vertex_clip[0]/vertex_clip[3];
	res[1]=vertex_clip[1]/vertex_clip[3];
	res[2]=vertex_clip[2]/vertex_clip[3];
	return res;
}


/**
* @brief      transform from device coordinate to window coordinate
* @param          Input    -- 
* @param          Input    -- 
* @return     vertex position in screen space
*/
vec3 PinholeCamera::ViewportTransform(const vec3 & normalized_coordinate) const 
{
	vec3 res;
	res[0]=(image_width_/2.0f)*(1+normalized_coordinate[0]);
	res[1]=(image_height_/2.0f)*(1+normalized_coordinate[1]);
	res[2]=(DepthRange[1]-DepthRange[0])/2*normalized_coordinate[2]+(DepthRange[0]+DepthRange[1])/2;
	return res;
}

//model_matrix=T*R, T represent model translation, R represent model rotation, R is camera external parameters
void PinholeCamera::CalculateModelMatrix(const CameraParameter& rho) 
{
	CalculateModelTranslation(rho);
	CalculateModelRotation(rho);
	model_space_to_world_space_ = model_translation_;
	model_space_to_world_space_ *= model_rotation_;
}


//view_matrix=R*T, R represent camera orientation, T represent camera position 
void PinholeCamera::CalculateViewMatrix(const CameraParameter& rho) 
{
	CalculateCamera();
	mat44 translation_matrix;
	translation_matrix.eye();
	translation_matrix.submat(0, 3, 2, 3) =- camera_postion_;
	world_space_to_camera_space_ = camera_orientation_;
	world_space_to_camera_space_ *= translation_matrix;
}

void PinholeCamera::CalculateViewMatrix(const CameraParameter& rho, const IlluminationParameter& lamda)
{
	CalculateCamera(rho,lamda);
	mat44 translation_matrix;
	translation_matrix.eye();
	translation_matrix.submat(0, 3, 2, 3) = -camera_postion_;
	world_space_to_camera_space_ = camera_orientation_;
	world_space_to_camera_space_ *= translation_matrix;
}



void PinholeCamera::CalculateFrustum(const CameraParameter& rho) 
{
	//camera_space_to_clip_space_.eye();
	//camera_space_to_clip_space_.at(0, 0) = rho.focal_length_*NEAR_PLANE / RIGHT;
	//camera_space_to_clip_space_.at(1, 1) = rho.focal_length_*NEAR_PLANE / TOP;
	//camera_space_to_clip_space_.at(2, 2) = -1; //  -(f+n)/(f-n) where f>>n
	//camera_space_to_clip_space_.at(3, 3) = 0;

	//camera_space_to_clip_space_.at(2, 3) = -2 * NEAR_PLANE;  // -2f*n/(f-n)  where f>>n
	//camera_space_to_clip_space_.at(3, 2) = -1;

	//view_port_size_ = IMAGE_WIDTH*NEAR_PLANE / rho.focal_length_ / 2;
	//camera_space_to_clip_space_.eye();
	//camera_space_to_clip_space_.at(0, 0) = NEAR_PLANE / view_port_size_;
	//camera_space_to_clip_space_.at(1, 1) = NEAR_PLANE / view_port_size_;
	//camera_space_to_clip_space_.at(2, 2) = -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE);//  -(f+n)/(f-n) where f>>n
	//camera_space_to_clip_space_.at(3, 3) = 0;

	//camera_space_to_clip_space_.at(2, 3) = -2*FAR_PLANE* NEAR_PLANE/(FAR_PLANE-NEAR_PLANE);  // -2f*n/(f-n)  where f>>n
	//camera_space_to_clip_space_.at(3, 2) = -1;

	view_port_size_ = IMAGE_WIDTH*NEAR_PLANE / rho.focal_length_ / 2;
	camera_space_to_clip_space_.eye();
	camera_space_to_clip_space_.at(0, 0) = NEAR_PLANE / view_port_size_;
	camera_space_to_clip_space_.at(1, 1) = NEAR_PLANE / view_port_size_;
	camera_space_to_clip_space_.at(2, 2) = -(FAR_PLANE + NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE);//  -(f+n)/(f-n) where f>>n
	camera_space_to_clip_space_.at(3, 3) = 0;

	camera_space_to_clip_space_.at(2, 3) = -2*FAR_PLANE* NEAR_PLANE/(FAR_PLANE-NEAR_PLANE);  // -2f*n/(f-n)  where f>>n
	camera_space_to_clip_space_.at(3, 2) = -1;


}

void PinholeCamera::CalculateFrustum(const CameraParameter& rho, const IlluminationParameter& lamda)
{
	view_port_size_ = rho.focal_length_ / FocalLength;

	camera_space_to_clip_space_.eye();
	camera_space_to_clip_space_.at(0, 0) = view_port_size_/ObjectSize;
	camera_space_to_clip_space_.at(1, 1) = view_port_size_ /ObjectSize;
	camera_space_to_clip_space_.at(2, 2) = -2 / (ObjectZ[1]-ObjectZ[0]);
	camera_space_to_clip_space_.at(2, 3) = -(ObjectZ[1]+ObjectZ[0])/(ObjectZ[1]-ObjectZ[0]);  
	//cout << camera_space_to_clip_space_ << endl;
}


//rotation_[0] = 0;  // theta x
//rotation_[1] = 0;  // phi y
//rotation_[2] = 0;  // gamma z

void PinholeCamera::CalculateModelRotation(const CameraParameter& rho) 
{
	model_rotation_ = CalculateRotation(rho.rotation_);  //  X Y Z

	shift_object_[0] = 0;
	shift_object_[1] = 0;
	shift_object_[2] =ShiftObject;
	vec3 sub_shift = model_rotation_.submat(0, 0, 2, 2)*shift_object_;
	model_rotation_.submat(0, 3, 2, 3) = sub_shift;

	model_rotation_.submat(0, 0, 2, 2) *= ModelScale;
}


mat44 PinholeCamera::CalculateRotation(const vec3& angles) const
{
	mat44 rotation;
	rotation.eye();

	double sx, sy, sz, cx, cy, cz;

	// rotation angle about X-axis (pitch)
	sx = sin(angles[0]);
	cx = cos(angles[0]);

	// rotation angle about Y-axis (yaw)
	sy = sin(angles[1]);
	cy = cos(angles[1]);

	// rotation angle about Z-axis (roll)
	sz = sin(angles[2]);
	cz = cos(angles[2]);

	// determine left axis
	rotation.at(0,0) = cy*cz;
	rotation.at(1,0) = sx*sy*cz + cx*sz;
	rotation.at(2,0) = -cx*sy*cz + sx*sz;

	// determine up axis
	rotation.at(0, 1) = -cy*sz;
	rotation.at(1, 1) = -sx*sy*sz + cx*cz;
	rotation.at(2, 1) = cx*sy*sz + sx*cz;

	// determine forward axis
	rotation.at(0, 2) = sy;
	rotation.at(1, 2) = -sx*cy;
	rotation.at(2, 2) = cx*cy;

	return rotation;
}
