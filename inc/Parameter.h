#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <armadillo>
#include <memory>
#include <array>
#include "common.h"


class Mesh;
class Face3dShape;
class Face3dTexture;
class InputImage;
class ModelImage;

enum class Variable
{
	X,
	Y
};


class Parameter
{
public:

	using MeshPtr = std::shared_ptr<Mesh>;
	using InputPtr = std::shared_ptr<InputImage>;
	using ModelPtr = std::shared_ptr<ModelImage>;
	using ShapePtr = std::shared_ptr<Face3dShape>;
	using TexturePtr = std::shared_ptr<Face3dTexture>;

	using mat = arma::mat;
	using mat33 = arma::mat33;
	using mat44 = arma::mat44;
	using vec2=arma::vec2;
	using ivec2=arma::ivec2;
	using vec3 = arma::vec3;


	enum class Type
	{
		ALPHA,
		BETA,
		RHO,
		LAMDA
	};


	Parameter(const mat& para,
					Type type,
					ModelPtr model,
					MeshPtr mesh ,
					ShapePtr shape = nullptr ,
					TexturePtr texture= nullptr);



	virtual ~Parameter(){}

	virtual vec3 ComputeGradient(int triangle_index, int parameter_index, const ivec2& pos)  = 0;
	virtual double ComputeGradientLandmark(int num, int parameter_index) const = 0;

    ivec2 ComputePosition(int triangle_id);
	vec3 ComputeColor(int triangle_id);


	vec3 ComputeNormal(const vec3& x0, const vec3& x1, const vec3& x2) ;
	vec3 ComputeReflection(const vec3& n, const vec3& d);
	vec3 ComputeView(const vec3& e0, const vec3& e1, const vec3& e2);


	enum  // 7
	{
		ROTATION = 0,
		TRANSLATION = 3,
		FOCAL = 6
	};

	
	enum // 15
	{
		DIRECTION = 0,
		DIRECT = 2,
		AMBIENT = 5,
		CC = 8,
		GAIN = 9,
		OFFSET = 12
	};


protected:
	static mat alpha_;
	static mat beta_;

	
	static vec3 camera_position_;
	static vec3 shift_object_;
	static vec3 camera_angle_;
	static mat33 rotation_;
	static vec3 translation_;
	static double focal_;

	static vec3 albedo_[3];
	static vec3 world_[3];
	static vec3 eye_[3];

	static vec3 nw_;
	static vec3 ne_;
	static vec3 le_;
	static vec3 ve_;
	static vec3 re_;

	static bool attached_shadow_;
	static vec2 light_angle_;
	static vec3 direct_;
	static vec3 ambient_;
	static mat33 mc_;
	static mat33 gain_;
	static vec3 offset_;

	ShapePtr shape_;
	TexturePtr texture_;
	MeshPtr mesh_;
	ModelPtr model_;


	int width_;
	int height_;
	int test_;
	
private:

	void ComputeRotation(const vec3& angle); // angle's unit is radian
	void ComputeMc(double cc);
};


class Alpha :public Parameter 
{

public:
	Alpha(const mat& alpha,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture) :Parameter(alpha, Type::ALPHA, model, mesh, shape, texture), input_(input){}

	vec2 ComputeLandmarkPosition(int i);
	vec3 ComputeGradient(int triangle_index, int parameter_index, const ivec2& pos)  override;
	double ComputeGradientLandmark(int num, int parameter_index) const override;


	class DirectionNumError{};

	const double Step = 0.0001; // 500: 0.0003


private:

	vec3 ComputeColorGradient() const;
	double  ComputeSubPosition(const vec3& principle, const vec3& eye, Variable  p) const;


private:
	InputPtr input_;

	vec3 sub_principle_[3];
};


class Beta :public Parameter
{
public:
	Beta(const mat&beta,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture) :Parameter(beta, Type::BETA, model, mesh, shape, texture){}


	vec3 ComputeGradient(int triangel_index, int parameter_index, const ivec2& pos)  override;
	double ComputeGradientLandmark(int num, int parameter_index) const override{ return 0; }


	const double Step = 0.0005; // 500: 0.0005


private:
	vec3 ComputeColorGradient(int triangle_index, int parameter_index) const;


private:
	vec3 sub_principle_[3];

};



class Rho:public Parameter
{
public:

	// for fitting
	Rho(const mat& rho,	
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture) :Parameter(rho, Type::RHO, model, mesh, shape, texture), input_(input)
	{
		ComputeAngleDerivate();
	}


	// for alignment
	Rho(const mat& rho,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh) :Parameter(rho, Type::RHO, model, mesh)
	{
		ComputeAngleDerivate();
	}


	vec3 ComputeGradient(int triangel_index, int parameter_index, const ivec2& pos)  override;
	double ComputeGradientLandmark(int num, int parameter_index) const override;


	mat33 GetAngleDerivate(int id) const
	{
		if (id == 0) return theta_derivate_;
		if (id == 1) return phi_derivate_;
		if (id == 2) return gamma_deriave_;
		throw AngleNumError();
	}

//	const double Step = 0.03;// 0.0000006
	const double Step = 0.00000003;// 0.0000006


	class AngleNumError{};
	class RhoNumError{};
	class DirectionNumError{};


private:
	vec3 ComputeColorGradient(int triangle_index, int parameter_index) const;
	double ComputeSubPosition(int parameter_index, const vec3& world, const vec3& eye, const Variable&  p) const;
	mat33 ComputeThetaDifferential(const vec3& angle) const;
	mat33 ComputePhiDifferential(const vec3& angle) const;
	mat33 ComputeGammaDifferential(const vec3& angle) const;
	void ComputeAngleDerivate();



private:
	InputPtr input_;


	mat33 theta_derivate_;
	mat33 phi_derivate_;
	mat33 gamma_deriave_; 

};



class Lamda:public Parameter
{
public:

	// fitting
	Lamda(const mat& lamda,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture) :Parameter(lamda, Type::LAMDA, model, mesh, shape, texture)
	{
		ComputeAngleDerivate();
	}


	// alignment
	Lamda(const mat& lamda,
		ModelPtr model,
		MeshPtr mesh) :Parameter(lamda, Type::LAMDA, model, mesh)
	{
	}


	vec3 ComputeGradient(int triangel_index, int parameter_index, const ivec2& pos)  override;
	double ComputeGradientLandmark(int num, int parameter_index) const override{ return 0; }


	vec3 GetAngleDerivate(int id) const 
	{
		if (id == 0) return theta_derivate_;
		if (id == 1)return phi_derivate_;
		throw  AngleNumError();
	}

	class AngleNumError{};
	class LamdaNumError{};


	const double Step = 0.00006;//0.00003


private:

	vec3 ComputeColorGradient(int triangle_index, int parameter_index) const;
	void ComputeAngleDerivate();
	void ManuallySet(double variance, int id);

	std::array<double, LamdaNum> variance_;
	std::array<double, LamdaNum> mean_;

	vec3 theta_derivate_;
	vec3 phi_derivate_;
};





#endif