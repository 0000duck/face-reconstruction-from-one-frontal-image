#ifndef optimizer_H_
#define optimizer_H_


#include "coust_function.h"
#include <armadillo>
#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <opencv2/opencv.hpp>

#include "common.h"
#include "Parameter.h"


class Mesh;
class Face3dShape;
class Face3dTexture;
class Parameter;
class CostFunction;
class IlluminationParameter;
class CameraParameter;


class Optimizer
{
public:

	using mat = arma::mat;
	using ivec2 = arma::ivec2;
	using vec3 =arma::vec3;
	using MeshPtr = std::shared_ptr<Mesh>;
	using ShapePtr = std::shared_ptr<Face3dShape>;
	using TexturePtr = std::shared_ptr<Face3dTexture>;
	using InputPtr = std::shared_ptr<InputImage>;
	using ModelPtr = std::shared_ptr<ModelImage>;


	Optimizer();


	double ComputeCost(InputPtr input_image, ModelPtr model, Alpha& alpha) const;
	double ComputeCost(InputPtr input_image, Alpha& alpha) const;


	void TwoPassZbuffer(const CameraParameter& rho,
		const IlluminationParameter& lamda,
		MeshPtr mesh,
		ModelPtr model,
		bool segment = false);


	void ShowPoints(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture,
		int point_type);



	void FitPose(mat& alpha,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	void FitIllumination(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture,
		const cv::Mat& ready);


	void FitFirstPrincipalComponents(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture,
		const cv::Mat& ready);


	void FitAll(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	void FitSegments(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	void GenerateRandomPoints(ModelPtr model, int num);

	double GetLamdaVariance(int i) const { return lamda_variance_[i]; }
	double GetLamdaMean(int i) const { return lamda_mean_[i]; }
	double GetRhoVariance(int i) const { return rho_variance_[i]; }
	double GetRhoMean(int i) const { return rho_mean_[i]; }


	enum PointsType
	{
		RANDOM,
		VISIBLE_TRIANGLE,
		SHADOW_TRIANGLE,
		SEGMENT,
	};

	enum   // 7
	{
		FOCAL = 6
	};



private:

	double ComputeGradient(CostFunction* cost_ptr, Parameter* para, int index) const;

	double ComputeLandmarkGradient(InputPtr input_image, Parameter*para, int index) const;
	double ComputeIntensityGradient(InputPtr input_image, Parameter*para, int index) const;


	void ShowRandomPoints(InputPtr input, ModelPtr model, Parameter* para) const;
	void ShowVisiblePoints(InputPtr input, ModelPtr model, Parameter* para) const;
	void ShowShadowPoints(InputPtr input, ModelPtr model, Parameter* para) const;
	void ShowSegmentPoints(InputPtr input, ModelPtr model, Parameter* para) const;


	void SetStartingValue(const mat& rho, const mat& lamda);


	void VisualizeResult(const CameraParameter& rho,
		const IlluminationParameter& lamda,
		InputPtr input_image,
		MeshPtr mesh,
		const std::string& name,
		int num=0);

	void  CpuRendering(const CameraParameter& rho,
		const IlluminationParameter& lamda,
		MeshPtr mesh,
		ModelPtr model);

	void  GpuRendering(const CameraParameter& rho,
		const IlluminationParameter& lamda,
		MeshPtr mesh,
		ModelPtr model);


	void FitEye(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model ,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	void FitNose(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	void FitMouth(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);

	void FitRest(mat& alpha,
		mat& beta,
		mat& rho,
		mat& lamda,
		InputPtr input,
		ModelPtr model,
		MeshPtr mesh,
		ShapePtr shape,
		TexturePtr texture);


	std::vector<int> random_points_;

	std::array<double, LamdaNum> lamda_variance_;
	std::array<double, LamdaNum> lamda_mean_;
	std::array<double, LamdaNum> rho_variance_;
	std::array<double, LamdaNum> rho_mean_;


	const double H =1.0e-4;

};








#endif