#include "Parameter.h"
#include "mesh.h"
#include "input_image.h"
#include "model_image.h"
#include "face3d_shape.h"
#include "face3d_texture.h"
#include "illumination_parameter.h"
#include "common.h"
#include "opengl_common.h"
#include "common_function.h"

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace arma;
using namespace std;



mat Parameter::alpha_;
mat Parameter::beta_;

vec3 Parameter::camera_position_;
vec3 Parameter::shift_object_;

vec3 Parameter::camera_angle_;
mat33 Parameter::rotation_;
vec3 Parameter::translation_;
double Parameter::focal_;

vec3 Parameter::albedo_[3];
vec3 Parameter::world_[3];
vec3 Parameter::eye_[3];

vec3 Parameter::nw_;
vec3 Parameter::ne_;
vec3 Parameter::le_;
vec3 Parameter::ve_;
vec3 Parameter::re_;

bool Parameter::attached_shadow_;
vec2 Parameter::light_angle_;
vec3 Parameter::direct_;
vec3 Parameter::ambient_;
mat33 Parameter::mc_;
mat33 Parameter::gain_;
vec3 Parameter::offset_;



Parameter::Parameter(const mat& para,
	Type type,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture) :model_(model), mesh_(mesh), shape_(shape), texture_(texture)
{
	width_ = model_->Rows();
	height_ = model_->Cols();

	attached_shadow_ = false;

	if (type == Type::ALPHA)
	{
		alpha_ = para;
	}
	else if (type == Type::BETA) 
	{
		 beta_ = para;
	}
	else if (type == Type::RHO)
	{
		// rho relative parameters
		camera_angle_ = para.rows(ROTATION, ROTATION + 2);
		ComputeRotation(camera_angle_);
		translation_ = para.rows(TRANSLATION, TRANSLATION + 2);
		focal_ = para[FOCAL];

		camera_position_[0] = 0;
		camera_position_[1] = 0;
		camera_position_[2] = CameraPosition;

		shift_object_[0] = 0;
		shift_object_[1] = 0;
		shift_object_[2] = ShiftObject;

	}
	else if (type == Type::LAMDA)
	{
		// lamda relative parameters
		light_angle_ = para.rows(DIRECTION, DIRECTION + 1);
		le_ = IlluminationParameter::Angle2Direction(light_angle_); // two angle
		direct_ = para.rows(DIRECT, DIRECT + 2);
		ambient_ = para.rows(AMBIENT, AMBIENT + 2);

		ComputeMc(para[CC]);

		mat33 gain = diagmat(para.rows(GAIN, GAIN + 2));
		gain_ = gain;
		offset_ = para.rows(OFFSET, OFFSET + 2);
	}
	else 
	  return;
}


void Parameter::ComputeMc(double cc)
{
	mat33 fixed;
	fixed.cols(0, 0).fill(ColorCoefficents[0]);
	fixed.cols(1, 1).fill(ColorCoefficents[1]);
	fixed.cols(2, 2).fill(ColorCoefficents[2]);

	mat33 identity;
	identity.eye();

	mat33 part1 = identity;
	part1 *= cc;

	mat33 part2 = fixed;
	part2 *= (1 - cc);

	mc_ = part1;
	mc_ += part2;
}

ivec2 Parameter::ComputePosition(int id)
{
	ivec3 vertex_index = mesh_->GetThreeCornerIndex(model_->visible_triangles_[id].id);

	for (int t = 0; t < 3; ++t)
	{
		mat principle = shape_->GetPrinciple(vertex_index[t]);
		vec3 mean = shape_->GetMean(vertex_index[t]);
		world_[t] = mean + principle*alpha_;
		eye_[t] = rotation_*(ModelScale*world_[t]+shift_object_) + translation_ - camera_position_;
	}

	vec3 eye = (eye_[0] + eye_[1] + eye_[2]) / 3;
	ivec2 pos;

	pos[0] = static_cast<int>(width_ / 2 - focal_ *eye[0] / eye[2]);
	pos[1] = static_cast<int>(height_ / 2 + focal_ *eye[1] / eye[2]);

	return pos;
}



vec3 Parameter::ComputeColor(int id)
{
	ivec3 vertex_index = mesh_->GetThreeCornerIndex(model_->visible_triangles_[id].id);


	for (int t = 0; t < 3; ++t)
	{
		mat principle = texture_->GetPrinciple(vertex_index[t]);
		vec3 mean = texture_->GetMean(vertex_index[t]);
		albedo_[t] = mean + principle*beta_;
	}

	nw_ = ComputeNormal(world_[0], world_[1], world_[2]);  // make no difference
	ne_ = rotation_*Normalize(nw_);
	ve_ = ComputeView(eye_[0], eye_[1], eye_[2]);
	re_ = ComputeReflection(Normalize(ne_), Normalize(le_));

	vec3 albedo = (albedo_[0] + albedo_[1] + albedo_[2]) / 3;

	mat33 albedo_mat = diagmat(albedo);

	vec3 cl;

	vec3 nen = Normalize(ne_);
	vec3 len = Normalize(le_);

	double nl_term = dot(nen, len);
	if (nl_term < 0)
	{
		attached_shadow_ = true;
	}
	else
	{
		attached_shadow_ = false;
	}

	if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true)
	{
		cl = albedo_mat*ambient_;
	}

	else
	{
		vec3 ren = Normalize(re_);
		vec3 ven = Normalize(ve_);
		cl = albedo_mat*ambient_ + albedo_mat*direct_*dot(nen, len) + Reflection*direct_*pow(dot(ren, ven), Shininess);
	}

	vec3 intensity = gain_*mc_*cl + offset_;

	return intensity;
}



void Parameter::ComputeRotation(const vec3& angle) // angle's unit is radian
{
	// Z Y X

	//double cc12 = cos(angle[1])*cos(angle[2]);
	//double cs02 = cos(angle[0])*sin(angle[2]);
	//double ssc = sin(angle[0])*sin(angle[1])*cos(angle[2]);
	//double ss02 = sin(angle[0])*sin(angle[2]);
	//double csc = cos(angle[0])*sin(angle[1])*cos(angle[2]);
	//double cs12 = cos(angle[1])*sin(angle[2]);
	//double cc02 = cos(angle[0])*cos(angle[2]);
	//double sss = sin(angle[0])*sin(angle[1])*sin(angle[2]);
	//double sc02 = sin(angle[0])*cos(angle[2]);
	//double css = cos(angle[0])*sin(angle[1])*sin(angle[2]);
	//double s1 = sin(angle[1]);
	//double sc01 = sin(angle[0])*cos(angle[1]);
	//double cc01 = cos(angle[0])*cos(angle[1]);


	//rotation_.at(0, 0) = cc12;
	//rotation_.at(0, 1) = -cs02 + ssc;
	//rotation_.at(0, 2) = ss02 + csc;

	//rotation_.at(1, 0) = cs12;
	//rotation_.at(1, 1) = cc02 +sss;
	//rotation_.at(1, 2) = -sc02 + css;

	//rotation_.at(2, 0) = -s1;
	//rotation_.at(2, 1) = sc01;
	//rotation_.at(2, 2) = cc01;


	//  X Y Z
	double cc12 = cos(angle[1])*cos(angle[2]);
	double cs02 = cos(angle[0])*sin(angle[2]);
	double ssc = sin(angle[0])*sin(angle[1])*cos(angle[2]);
	double ss02 = sin(angle[0])*sin(angle[2]);
	double csc = cos(angle[0])*sin(angle[1])*cos(angle[2]);

	double cs12 = cos(angle[1])*sin(angle[2]);
	double cc02 = cos(angle[0])*cos(angle[2]);
	double sss = sin(angle[0])*sin(angle[1])*sin(angle[2]);
	double sc02 = sin(angle[0])*cos(angle[2]);
	double css = cos(angle[0])*sin(angle[1])*sin(angle[2]);

	double s1 = sin(angle[1]);
	double sc01 = sin(angle[0])*cos(angle[1]);
	double cc01 = cos(angle[0])*cos(angle[1]);


	rotation_.at(0, 0) = cc12;
	rotation_.at(0, 1) = -cs12;
	rotation_.at(0, 2) = s1;

	rotation_.at(1, 0) = cs02+ssc;
	rotation_.at(1, 1) = cc02 - sss;
	rotation_.at(1, 2) = -sc01;

	rotation_.at(2, 0) = ss02-csc;
	rotation_.at(2, 1) = sc02+css;
	rotation_.at(2, 2) = cc01;

}



inline vec3 Parameter::ComputeNormal(const vec3& x0, const vec3& x1, const vec3& x2)
{
	vec3 a = x1 - x0; 
	vec3 b = x2 - x0; 
	return cross(a, b);
}

inline vec3 Parameter::ComputeReflection(const vec3& n, const vec3& d) 
{
	return 2 * dot(n, d)*n - d;
}

inline vec3 Parameter::ComputeView(const vec3& e0, const vec3& e1, const vec3& e2) 
{
	return -(e0 + e1 + e2) / 3;  
}


vec2 Alpha::ComputeLandmarkPosition(int i)
{
	int id = model_->landmarks_[i];
	mat principle = shape_->GetPrinciple(id);  // 3*99
	vec3 mean = shape_->GetMean(id);
	vec3 world = mean + principle*alpha_;
	vec3 eye = rotation_*(ModelScale*world+ shift_object_) + translation_ - camera_position_;

	vec2 pos;
	pos[0] = width_ / 2 - focal_ *eye[0] / eye[2];
	pos[1] = height_ / 2 + focal_ *eye[1] / eye[2];

	return pos;
}


vec3 Alpha::ComputeGradient(int id, int parameter_index, const ivec2& pos) 
{

	Point position(pos[0], pos[1]);
	vec3 sobel_x = input_->Sobel(position, InputImage::Direction::X);
	vec3 soble_y = input_->Sobel(position, InputImage::Direction::Y);

	ivec3 vertex_index = mesh_->GetThreeCornerIndex(model_->visible_triangles_[id].id);


	for (int t = 0; t < 3; ++t)
	{
		sub_principle_[t] = shape_->GetPrinciple(vertex_index[t],parameter_index);
	}

	vec3 center_principle = (sub_principle_[0] + sub_principle_[1] + sub_principle_[2]) / 3;
	vec3 center_eye = (eye_[0] + eye_[1] + eye_[2]) / 3;

	vec3 gradient_input = sobel_x*ComputeSubPosition(center_principle, center_eye, Variable::X) +
									soble_y*ComputeSubPosition(center_principle, center_eye, Variable::Y);
	vec3 gradient_model;

	if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true)
		gradient_model.fill(0);
	else
		 gradient_model = gain_*mc_*ComputeColorGradient();

	return gradient_model - gradient_input;

}



double Alpha::ComputeSubPosition(const vec3& principle, const vec3& eye, Variable variable) const
{

	vec3 temp = ModelScale*rotation_*principle;

	if (variable == Variable::X)
	{
		return -focal_*(temp[0] * eye[2] - temp[2] * eye[0]) / (eye[2] * eye[2]);
	}

	else if (variable == Variable::Y)
	{
		return focal_*(temp[1] * eye[2] - temp[2] * eye[1]) / (eye[2] * eye[2]);
	}

	throw DirectionNumError();
}


vec3 Alpha::ComputeColorGradient() const
{
	vec3 derivate_nw = cross(sub_principle_[1] - sub_principle_[0], world_[2] - world_[0]) +
		cross(world_[1] - world_[0], sub_principle_[2] - sub_principle_[0]);

	double norm_nw = norm(nw_, 2);
	vec3 derivate_nwn = 1 / norm_nw*(derivate_nw - nw_ / (norm_nw*norm_nw)*dot(nw_, derivate_nw));
	vec3 derivate_nen = rotation_*derivate_nwn;

	vec3 nen = Normalize(ne_);
	vec3 len = Normalize(le_);
	vec3 ren = Normalize(re_);
	vec3 ven = Normalize(ve_);

	vec3 derivate_ren = 2 * (dot(derivate_nen, len)*nen + dot(nen, len)*derivate_nen);

	//vec3 derivate_ve = -ModelScale*rotation_ * (sub_principle_[0] + sub_principle_[1] + sub_principle_[2]) / 3;
	//double norm_ve = norm(ve_, 2);
	//vec3 derivate_ven = 1 / norm_ve*(derivate_ve - ve_ / (norm_ve*norm_ve)*dot(ve_, derivate_ve));


	vec3 albedo = (albedo_[0] + albedo_[1] + albedo_[2]) / 3;
	mat33 albedo_mat = diagmat(albedo);

	vec3 gradient = albedo_mat*direct_*dot(derivate_nen, len) +
							Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*dot(derivate_ren, ven);
		//Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*(dot(derivate_ren, ven) + dot(ren, derivate_ven));

	return gradient;
}


double Alpha::ComputeGradientLandmark(int num, int parameter_index) const
{
	int landmark_id = mesh_->Landmarks(num);

	mat principle = shape_->GetPrinciple(landmark_id);  // 3*99
	vec3 sub_principle = principle.col(parameter_index);
	vec3 mean = shape_->GetMean(landmark_id);
	vec3 world = mean + principle*alpha_;

	vec3 eye = rotation_*(ModelScale*world + shift_object_) + translation_ - camera_position_;

	double model_x = width_ / 2  - focal_*eye[0] / eye[2];
	double model_y = height_ / 2 + focal_*eye[1] / eye[2]; 

	Point input_pos = input_->landmarks_[num];

	double part1 = 2 * (model_x - input_pos.x)*ComputeSubPosition(sub_principle, eye, Variable::X);
	double part2 = 2 * (model_y - input_pos.y)*ComputeSubPosition(sub_principle, eye, Variable::Y);

	return part1 + part2;
}




vec3 Beta::ComputeGradient(int id, int parameter_index, const ivec2& pos)   
{ 
	ivec3 vertex_index = mesh_->GetThreeCornerIndex(model_->visible_triangles_[id].id);

	for (int t = 0; t < 3; ++t)
	{
		sub_principle_[t] = texture_->GetPrinciple(vertex_index[t], parameter_index);
	}

	return gain_*mc_*ComputeColorGradient(id, parameter_index);
}

vec3 Beta::ComputeColorGradient(int id, int parameter_index) const
{
   if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true)
		return diagmat((sub_principle_[0] + sub_principle_[1] + sub_principle_[2]) / 3) * ambient_;
	else
	{
		vec3 nen = Normalize(ne_);
		vec3 len = Normalize(le_);
		return diagmat((sub_principle_[0] + sub_principle_[1] + sub_principle_[2]) / 3)*(ambient_ + direct_*dot(nen, len));
	}
}



vec3 Rho::ComputeGradient(int id, int parameter_index, const ivec2& pos) 
{
	Point position(pos[0], pos[1]);

	vec3 sobel_x = input_->Sobel(position, InputImage::Direction::X); // horizontal
	vec3 soble_y = input_->Sobel(position, InputImage::Direction::Y); //vertical 

	vec3 world = (world_[0] + world_[1] + world_[2]) / 3;
	vec3 scale_world = ModelScale*world;

	vec3 eye = (eye_[0] + eye_[1] + eye_[2]) / 3;

	vec3 gradient_input = sobel_x*ComputeSubPosition(parameter_index, scale_world, eye, Variable::X); +
									soble_y*ComputeSubPosition(parameter_index, scale_world, eye, Variable::Y);
	vec3 gradient_model;
    if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true || parameter_index==FOCAL)
		gradient_model.fill(0);
	else 
	    gradient_model = gain_*mc_*ComputeColorGradient(id, parameter_index);

	return  gradient_model- gradient_input;
}



double Rho::ComputeGradientLandmark(int num, int parameter_index) const
{
	int landmark_id = mesh_->Landmarks(num);

	vec3 word = mesh_->GetCoordinate(landmark_id);
	
	vec3 eye = rotation_*(ModelScale*word+shift_object_) + translation_ - camera_position_;

	vec3 scale_word = ModelScale*word+shift_object_;

	vec2 model_pos;
	model_pos[0] = width_ / 2 - focal_*eye[0] / eye[2];
	model_pos[1] = height_ / 2 + focal_*eye[1] / eye[2]; // opencv coordinate system


	vec2 input_pos;
	input_pos[0] = input_->landmarks_[num].x;
	input_pos[1] = input_->landmarks_[num].y;



	vec2 model_gradient;
	model_gradient[0] = ComputeSubPosition(parameter_index, scale_word, eye, Variable::X);
	model_gradient[1] = ComputeSubPosition(parameter_index, scale_word, eye, Variable::Y);


	return 2 * dot(model_pos - input_pos, model_gradient);
}


double Rho::ComputeSubPosition(int parameter_index,const vec3& scale_world, const vec3& eye ,const Variable&  variable) const
{

	// angle is related to e
	if (parameter_index >= ROTATION && parameter_index < ROTATION+3)  // rotation variable
	{

		vec3 temp = GetAngleDerivate(parameter_index)*scale_world;

		if (variable == Variable::X)
		{
			return -focal_*(temp[0] * eye[2] - temp[2] * eye[0]) / (eye[2] * eye[2]);
		}

		else if (variable == Variable::Y)
		{
			return focal_*(temp[1] *eye[2] - temp[2]*eye[1]) / (eye[2] * eye[2]);
		}

	}

	// translate is related to e 
	else if (parameter_index >= TRANSLATION && parameter_index < TRANSLATION+3) // translation variable 
	{

		int id = parameter_index - TRANSLATION;
		vec3 delta;
		delta.fill(0);
		delta[id] = 1;

		if (variable == Variable::X)
		{
			return -focal_*(delta[0] * eye[2] - delta[2] * eye[0]) / (eye[2] * eye[2]);
		}

		else if (variable == Variable::Y)
		{
			return focal_*(delta[1] * eye[2] - delta[2] * eye[1]) / (eye[2] * eye[2]);
		}
	}

	// focal is related to nothing
	else  if (parameter_index==FOCAL)
	{
		if (variable == Variable::X)
		{
			return -eye[0] / eye[2];
		}

		else if (variable == Variable::Y)
		{		 
			return eye[1] / eye[2];
		}

	}

	throw RhoNumError();
}


vec3 Rho::ComputeColorGradient(int triangle_index, int parameter_index) const
{
	// angle is related to e n r v, unrelated to x 
	if (parameter_index >= ROTATION && parameter_index < ROTATION+3)  
	{
		vec3 world = (world_[0] + world_[1] + world_[2]) / 3;
		vec3 scale_world = ModelScale*world+shift_object_;

		vec3 center_albedo = (albedo_[0] + albedo_[1] + albedo_[2]) / 3;


		mat33 albedo_mat = diagmat(center_albedo);

		vec3 nwn = Normalize(nw_);
		vec3 nen = Normalize(ne_);
		vec3 len = Normalize(le_);
		vec3 ven = Normalize(ve_);
		vec3 ren = Normalize(re_);
	   
		vec3 derivate_nen = GetAngleDerivate(parameter_index)*nwn;
		vec3 derivate_ren = 2 * (dot(derivate_nen, len)*nen + dot(nen, len)*derivate_nen);

		vec3 derivate_ve = -GetAngleDerivate(parameter_index)*scale_world;
		double norm_ve = norm(ve_, 2);
		vec3 derivate_ven = 1 / norm_ve*(derivate_ve, -ve_ /( norm_ve * norm_ve)*dot(ve_, derivate_ve));


		return albedo_mat*direct_*dot(derivate_nen, len) +
								Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*(dot(derivate_ren, ven) + dot(ren, derivate_ven));
							  //  Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*dot(derivate_ren, ven);

	}


	// translate is  related to e v, unrelated to x n r
	else if (parameter_index >= TRANSLATION && parameter_index < TRANSLATION+3)
	{
		vec3 ven = Normalize(ve_);
		vec3 ren = Normalize(re_);
		int id = parameter_index - TRANSLATION;
		double ve_norm = norm(ve_, 2);

		vec3 delta;
		delta.fill(0);
		delta[id] = -1;

		vec3 deriave_ven = (1 / ve_norm)*(delta - ve_ / (ve_norm * ve_norm)*dot(ve_, delta));
		return  Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*dot(ren, deriave_ven);
	}

	// focal is related to nothing
	else if (parameter_index == FOCAL)
	{
		vec3 res;
		res.fill(0);
		return res;
	}

	throw RhoNumError();
}



//rotation_[0] = 0;  // theta  x
//rotation_[1] = 0;  // phi  y
//rotation_[2] = 0;  // gamma  z

void Rho::ComputeAngleDerivate() 
{	
	theta_derivate_ = ComputeThetaDifferential(camera_angle_);
	phi_derivate_ = ComputePhiDifferential(camera_angle_);
	gamma_deriave_ = ComputeGammaDifferential(camera_angle_);
}


//rotation_[0] = 0;  // theta  x
//rotation_[1] = 0;  // phi  y
//rotation_[2] = 0;  // gamma  z

mat33 Rho::ComputeThetaDifferential(const vec3& angle) const
{
	mat33 differential_theta;

	// theta
	double csc = cos(angle[0])*sin(angle[1])*cos(angle[2]);
	double ss02 = sin(angle[0])*sin(angle[2]);
	double ssc = sin(angle[0])*sin(angle[1])*cos(angle[2]);
	double cs02 = cos(angle[0])*sin(angle[2]);
	double css = cos(angle[0])*sin(angle[1])*sin(angle[2]);
	double sc02 = sin(angle[0])*cos(angle[2]);
	double sss = sin(angle[0])*sin(angle[1])*sin(angle[2]);
	double cc02 = cos(angle[0])*cos(angle[2]);
	double cc01 = cos(angle[0])*cos(angle[1]);
	double sc01 = sin(angle[0])*cos(angle[1]);

	//differential_theta.at(0, 0) = 0;
	//differential_theta.at(0, 1) = csc + ss02;
	//differential_theta.at(0, 2) = -ssc + cs02;

	//differential_theta.at(1, 0) = 0;
	//differential_theta.at(1, 1) = css - sc02;  
	//differential_theta.at(1, 2) = -sss - cc02;   //

	//differential_theta.at(2, 0) = 0;
	//differential_theta.at(2, 1) = cc01;   //
	//differential_theta.at(2, 2) = -sc01;


	differential_theta.at(0, 0) = 0;
	differential_theta.at(0, 1) = 0;
	differential_theta.at(0, 2) = 0;

	differential_theta.at(1, 0) = -ss02+csc;
	differential_theta.at(1, 1) = -sc02-css;
	differential_theta.at(1, 2) = -cc01;   //

	differential_theta.at(2, 0) = cs02+ssc;
	differential_theta.at(2, 1) = cc02-sss;   //
	differential_theta.at(2, 2) = -sc01;


	return differential_theta;
}


//rotation_[0] = 0;  // theta  x
//rotation_[1] = 0;  // phi  y
//rotation_[2] = 0;  // gamma  z


mat33 Rho::ComputePhiDifferential(const vec3& angle) const
{
	mat33 differential_phi;
	// phi
	double sc12 = sin(angle[1])*cos(angle[2]);
	double scc = sin(angle[0])*cos(angle[1])*cos(angle[2]);
	double ccc = cos(angle[0])*cos(angle[1])*cos(angle[2]);
	double ss12 = sin(angle[1])*sin(angle[2]);

	double scs = sin(angle[0])*cos(angle[1])*sin(angle[2]);
	double ccs = cos(angle[0])*cos(angle[1])*sin(angle[2]);
	double c1 = cos(angle[1]);
	double ss01 = sin(angle[0])*sin(angle[1]);
	double cs01 = cos(angle[0])*sin(angle[1]);

	//differential_phi.at(0, 0) = -sc12;
	//differential_phi.at(0, 1) = scc;
	//differential_phi.at(0, 2) = ccc;  //  

	//differential_phi.at(1, 0) = -ss12;
	//differential_phi.at(1, 1) = scs;
	//differential_phi.at(1, 2) = ccs;  

	//differential_phi.at(2, 0) = -c1;//
	//differential_phi.at(2, 1) = -ss01;
	//differential_phi.at(2, 2) = -cs01;


	differential_phi.at(0, 0) = -sc12;
	differential_phi.at(0, 1) = ss12;
	differential_phi.at(0, 2) = c1;  //  

	differential_phi.at(1, 0) = scc;
	differential_phi.at(1, 1) = -scs;
	differential_phi.at(1, 2) = ss12;

	differential_phi.at(2, 0) = -ccc;//
	differential_phi.at(2, 1) = ccs;
	differential_phi.at(2, 2) = -cs01;


	return differential_phi;
}


//rotation_[0] = 0;  // theta  x
//rotation_[1] = 0;  // phi  y
//rotation_[2] = 0;  // gamma  z


mat33 Rho::ComputeGammaDifferential(const vec3& angle) const   // angle is in hudu
{
	mat33 differential_gamma;
	// gamma
	double cs12 = cos(angle[1])*sin(angle[2]);
	double sss = sin(angle[0])*sin(angle[1])*sin(angle[2]);
	double cc02 = cos(angle[0])*cos(angle[2]);
	double css = cos(angle[0])*sin(angle[1])*sin(angle[2]);
	double sc02 = sin(angle[0])*cos(angle[2]);
	double cc12 = cos(angle[1])*cos(angle[2]);
	double ssc = sin(angle[0])*sin(angle[1])*cos(angle[2]);
	double cs02 = cos(angle[0])*sin(angle[2]);
	double csc = cos(angle[0])*sin(angle[1])*cos(angle[2]);
	double ss02 = sin(angle[0])*sin(angle[2]);

	//differential_gamma.at(0, 0) = -cs12;  
	//differential_gamma.at(0, 1) = -sss - cc02;  // 
	//differential_gamma.at(0, 2) = -css + sc02;

	//differential_gamma.at(1, 0) = cc12;  //
	//differential_gamma.at(1, 1) = ssc - cs02;
	//differential_gamma.at(1, 2) = csc + ss02;

	//differential_gamma.at(2, 0) = 0;
	//differential_gamma.at(2, 1) = 0;
	//differential_gamma.at(2, 2) = 0;


	differential_gamma.at(0, 0) = -cs12;
	differential_gamma.at(0, 1) = - cc12;  // 
	differential_gamma.at(0, 2) = 0;

	differential_gamma.at(1, 0) = cc02 - sss;  //
	differential_gamma.at(1, 1) = -cs02-ssc;
	differential_gamma.at(1, 2) = 0;


	differential_gamma.at(2, 0) = sc02+css;
	differential_gamma.at(2, 1) = -ss02+csc;
	differential_gamma.at(2, 2) = 0;

	return differential_gamma;
}



vec3 Lamda::ComputeGradient(int id, int parameter_index, const ivec2& pos)
{
	if (parameter_index >= DIRECTION && parameter_index < DIRECT + 3) // from direction to direct
	{
		//if (model_->triangles_[triangle_id].shadow == true || attached_shadow_ == true)
		if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true)
		{
			vec3 res;
			res.fill(0);
			return res;
		}
		else 
			return gain_*mc_*ComputeColorGradient(id, parameter_index);
	}
	else if (parameter_index >= AMBIENT && parameter_index < AMBIENT + 3)
	{
		return gain_*mc_*ComputeColorGradient(id, parameter_index);
	}
	else if (parameter_index == CC) // contrast
	{
		mat33 identity;
		identity.eye();

		mat33 fixed;
		fixed.cols(0, 0).fill(ColorCoefficents[0]);
		fixed.cols(1, 1).fill(ColorCoefficents[1]);
		fixed.cols(2, 2).fill(ColorCoefficents[2]);

		mat33 derivate_mc = identity - fixed;

		return gain_*derivate_mc*ComputeColorGradient(id, parameter_index);
	}	
	else if (parameter_index >= GAIN && parameter_index < GAIN+3)  // gain
	{
		vec3 delta;
		delta.fill(0);
		delta[parameter_index - GAIN] = 1;
		mat33 derivate_gain = diagmat(delta);

		return derivate_gain*mc_*ComputeColorGradient(id, parameter_index);
	}
	else if (parameter_index>=OFFSET && parameter_index<OFFSET+3) // offset
	{
		vec3 delta;
		delta.fill(0);
		delta[parameter_index - OFFSET] = 1;
		return delta;
	}

	throw LamdaNumError();
}


vec3 Lamda::ComputeColorGradient(int id, int parameter_index) const
{
	vec3 albedo = (albedo_[0] + albedo_[1] + albedo_[2]) / 3;
	mat albedo_mat = diagmat(albedo);

	// light direction is related to nothing 
	if (parameter_index >= DIRECTION && parameter_index < DIRECTION+2)
	{
		double norm_le = norm(le_, 2);

		vec3 nen = Normalize(ne_);
		vec3 ren = Normalize(re_);
		vec3 ven = Normalize(ve_);

		vec3 derivate_len = GetAngleDerivate(parameter_index);
		vec3 derivate_ren = 2 * dot(nen, derivate_len)*nen - derivate_len;

		return albedo_mat*direct_*dot(nen, derivate_len) +
								Reflection*direct_*Shininess*pow(dot(ren, ven), Shininess - 1)*dot(derivate_ren, ven);
	}
	// direct is related to nothing 
	else if (parameter_index >= DIRECT && parameter_index < DIRECT + 3)
	{
		vec3 delta;
		delta.fill(0);
		delta[parameter_index - DIRECT] = 1;

		vec3 nen = Normalize(ne_);
		vec3 len = Normalize(le_);
		vec3 ren = Normalize(re_);
		vec3 ven = Normalize(ve_);

		return albedo_mat*delta*dot(nen, len) + Reflection*pow(dot(ren, ven), Shininess)*delta;
	}

	// ambient is related to nothing 
	else if (parameter_index >= AMBIENT && parameter_index < AMBIENT+3)
	{
		vec3 delta;
		delta.fill(0);
		delta[parameter_index - AMBIENT] = 1;
		return albedo_mat*delta;
	}

	// others are related to nothing 
	else if (parameter_index>=CC && parameter_index<OFFSET+3)
	{
		if (model_->visible_triangles_[id].cast_shadow == true || attached_shadow_ == true)
		{
			return albedo_mat*ambient_;
		}
		else
		{
			vec3 nen = Normalize(ne_);
			vec3 len = Normalize(le_);
			vec3 ren = Normalize(re_);
			vec3 ven = Normalize(ve_);

			return albedo_mat*ambient_ + albedo_mat*direct_*dot(nen, len) + Reflection*direct_*pow(dot(ren, ven), Shininess);
		}
	}

	throw LamdaNumError();
}

void Lamda::ComputeAngleDerivate()
{
	theta_derivate_[0] = -sin(light_angle_[0])*sin(light_angle_[1]);
	theta_derivate_[1] = cos(light_angle_[0]);
	theta_derivate_[2] = -sin(light_angle_[0])*cos(light_angle_[1]);

	phi_derivate_[0] = cos(light_angle_[0])*cos(light_angle_[1]);
	phi_derivate_[1] = 0;
	phi_derivate_[2] = -cos(light_angle_[0])*sin(light_angle_[1]);
}



