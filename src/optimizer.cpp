#include "optimizer.h"
#include "parameter.h"
#include "model_image.h"
#include "input_image.h"
#include "face3d_shape.h"
#include "face3d_texture.h"


#include "mesh.h"
#include "face3d_model.h"
#include "pinhole_camera.h"
#include "phong_illumination.h"
#include "depth_buffer.h"
#include "box_raster.h"

#include "software_render.h"
#include "hardware_render.h"

#include "opengl_common.h"

#include <vector>
#include <array>
#include <random>
#include <sstream>


using namespace std;
using namespace cv;
using namespace arma;



Optimizer::Optimizer()
{

	rho_variance_[0] = 1.5573e-07;
	rho_variance_[1] = 1.5573e-07;
	rho_variance_[2] = 1.5573e-07;

	rho_variance_[3] = 331.9330;
	rho_variance_[4] = 17.9917;
	rho_variance_[5] = 1.2977e05;

	rho_variance_[6] = 6.9505e-04;

	lamda_variance_[0] = 0.0761;
	lamda_variance_[1] = 0.0761;

	lamda_variance_[2] = 0.0391;
	lamda_variance_[3] = 0.1103;
	lamda_variance_[4] = 0.2803;

	lamda_variance_[5] = 0.0160;
	lamda_variance_[6] = 0.0399;
	lamda_variance_[7] = 0.1653;

	lamda_variance_[8] = 0.01;

	lamda_variance_[9] = 0.01;
	lamda_variance_[10] = 0.01;
	lamda_variance_[11] = 0.01;

	lamda_variance_[12] = 0.01;
	lamda_variance_[13] = 0.01;
	lamda_variance_[14] = 0.01;
}


void Optimizer::ShowPoints(mat& alpha, 
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input, 
	ModelPtr model, 
	MeshPtr mesh, 
	ShapePtr shape, 
	TexturePtr texture, 
	int point_type)
{

	mat alpha_gradient(PrincipalNum, 1);
	alpha_gradient.fill(0);

	mat beta_gradient(PrincipalNum, 1);
	beta_gradient.fill(0);


	mat rho_gradient(RhoNum, 1);
	rho_gradient.fill(0);

	mat lamda_gradient(LamdaNum, 1);
	lamda_gradient.fill(0);


	Alpha alpha_para(alpha, input, model, mesh, shape, texture);
	Beta beta_para(beta, model, mesh, shape, texture);
	Rho rho_para(rho, input, model, mesh, shape, texture);
	Lamda lamda_para(lamda, model, mesh, shape, texture);



	switch (point_type)
	{
	case RANDOM:
		model->InitialRandomGenerator();
		GenerateRandomPoints(model, 40);
		ShowRandomPoints(input, model, &alpha_para);
		break;
	case VISIBLE_TRIANGLE:
		model->EnableIterator();
		ShowVisiblePoints(input, model, &alpha_para);
		break;
	case SHADOW_TRIANGLE:
		model->EnableIterator();
		ShowShadowPoints(input, model, &alpha_para);
		break;
	case SEGMENT:
		ShowSegmentPoints(input, model, &alpha_para);
		break;

	default:
		break;
	}

	model->Show();
}



void Optimizer::FitPose(mat& alpha,
	mat& rho, 
	mat& lamda, 
	InputPtr input, 
	ModelPtr model, 
	MeshPtr mesh, 
	ShapePtr shape,
	TexturePtr texture)
{



	mat alpha_gradient=mat(PrincipalNum, 1,fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat rho_gradient=mat(RhoNum, 1,fill::zeros);
	mat rho_hessian_inv = mat(RhoNum, RhoNum, fill::zeros);


	double weight = 0.05;
	int para_num = 10;
	int counter = 0;

	while (counter<300)  // 300
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Rho rho_para(rho, input, model, mesh);
		Lamda lamda_para(lamda, model, mesh);

		//stringstream file;
		//double residual = 0;
		//residual = ComputeCost(input, alpha_para);
		//ofstream cost;
		//cost.open("cost", ios::app);
		//cost << residual << "\n";
		//cost.close();

		if (counter == 0 || counter % 1000 == 0)
		{
			for (int i = 0; i < para_num; ++i)
			{
				double variance = shape->GetVariance(i);
				mat alpha1 = alpha;
				alpha1[i] -= H;
				Alpha alpha_para1(alpha1, input, model, mesh, shape, texture);
				double first_derivative1 = weight * ComputeLandmarkGradient(input, &alpha_para1, i);


				mat alpha2 = alpha;
				alpha2[i] += H;
				Alpha alpha_para2(alpha2, input, model, mesh, shape, texture);
				double first_derivative2 = weight * ComputeLandmarkGradient(input, &alpha_para2, i);

				double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
				alpha_hessian_inv(i, i) = 1 / (second_derivative + 2 / variance);

			}

			for (int i = 0; i < RhoNum;++i)
			{
				// rho
				mat rho1 = rho;
				rho1[i] -= H;
				Rho rho_para1(rho1, input, model, mesh);
				double first_derivative1 = weight * ComputeLandmarkGradient(input, &rho_para1, i);

				mat rho2 = rho;
				rho1[i] += H;
				Rho rho_para2(rho2, input, model, mesh);
				double first_derivative2 = weight * ComputeLandmarkGradient(input, &rho_para2, i);

				double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
				rho_hessian_inv(i, i) = 1 / second_derivative;
			}

		}


		for (int i = 0; i < para_num; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight*ComputeLandmarkGradient(input, &alpha_para, i)+2 * alpha[i] / variance;
		}


		for (int i = 0; i < RhoNum; ++i)
		{
			rho_gradient[i] = weight*ComputeLandmarkGradient(input, &rho_para, i);
		}


		alpha -= 0.0006*alpha_hessian_inv*alpha_gradient;
		rho -= 0.01*rho_hessian_inv*rho_gradient;

		++counter;
	}

}



void Optimizer::FitIllumination(mat& alpha, 
	mat& beta, 
	mat& rho, 
	mat& lamda, 
	InputPtr input, 
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape, 
	TexturePtr texture, 
	const cv::Mat& ready)
{


	mat lamda_gradient=mat(LamdaNum, 1,fill::zeros);
	mat lamda_hessian_inv = mat(LamdaNum, LamdaNum, fill::zeros);


	Face3dModel face3d_model(shape, texture);
	mesh = face3d_model.Construction(alpha, beta);
	//VisualizeResult(rho, lamda, input, mesh, 0);
	TwoPassZbuffer(rho, lamda, mesh, model);
	model->EnableIterator();
	model->InitialRandomGenerator();

	// down sampled version
	cv::Mat down_sampled;
	pyrDown(ready, down_sampled, Size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2));
	input = make_shared<InputImage>(down_sampled);
	model->SetSize(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
	rho[FOCAL] /= 2;

	double weight = 1.0 / 1000;
	int counter = 0;

	while (counter < 500)
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);
		Rho rho_para(rho, input, model, mesh, shape, texture);
		Lamda lamda_para(lamda, model, mesh, shape, texture);

		// Generate random points  
		//if (counter == 0 || counter % 1000 == 0)
		//{
		//	GenerateRandomPoints(model, HessianRandomNum);

		//	for (int i = 0; i < LamdaNum - 7; ++i)
		//	{
		//		mat lamda1 = lamda;
		//		lamda1[i] -= H;
		//		Lamda lamda_para1(lamda1, model, mesh, shape, texture);
		//		double first_derivative1 = weight*ComputeIntensityGradient(input, &lamda_para1, i);


		//		mat lamda2 = lamda;
		//		lamda2[i] += H;
		//		Lamda lamda_para2(lamda2, model, mesh, shape, texture);
		//		double first_derivative2 = weight* ComputeIntensityGradient(input, &lamda_para2, i);
		//		double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);

		//		lamda_hessian_inv(i, i) = 1 / second_derivative;

		//	}


		//}
	
		GenerateRandomPoints(model, GradientRandomNum);

		//double function_value = 0;
		//function_value = ComputeCost(input, model, alpha_para);
		//ofstream cost;
		//cost.open("illumination_cost", ios::app);
		//cost << function_value << "\n";
		//cost.close();


		for (int i = 0; i < LamdaNum; ++i)
		{		 
			lamda_gradient[i] = weight*ComputeIntensityGradient(input, &lamda_para, i);
		}

		lamda -= lamda_para.Step*lamda_gradient;
		//lamda -= 0.5*lamda_hessian_inv*lamda_gradient;
		++counter;
	}

	// restore original version
	input= make_shared<InputImage>(ready);
	model->SetSize(IMAGE_WIDTH, IMAGE_HEIGHT);
	rho[FOCAL] *= 2;

	VisualizeResult(rho, lamda, input, mesh, "illumination");

}


void Optimizer::FitFirstPrincipalComponents(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture,
	const cv::Mat& ready)
{


	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat rho_gradient = mat(RhoNum, 1, fill::zeros);
	mat lamda_gradient = mat(LamdaNum, 1, fill::zeros);

	vec step(RhoNum, 1);
	step.rows(0, 2).fill(0.00000006);
	step.rows(3, 5).fill(0.0002);
	step.rows(6, 6).fill(2.0);
	mat step_mat = diagmat(step);



	// down sampled version
	cv::Mat down_sampled;
	pyrDown(ready, down_sampled, Size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2));
	input = make_shared<InputImage>(down_sampled);
	model->SetSize(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
	rho[FOCAL] /= 2;


	//SetStartingValue(rho, lamda);
	double para_num = 10;
	double weight = 1.0 / 900;

	int counter = 0;
	while (counter < 1000)
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);
		Rho rho_para(rho, input, model, mesh, shape, texture);
		Lamda lamda_para(lamda, model, mesh, shape, texture);


		if (counter == 0 || counter % 1000 == 0)
		{
			Face3dModel face3d_model(shape, texture);
			mesh = face3d_model.Construction(alpha, beta);
			TwoPassZbuffer(rho, lamda, mesh, model);
			model->EnableIterator();
			model->InitialRandomGenerator();

			//GenerateRandomPoints(model, HessianRandomNum);
			//for (int i = 0; i < para_num; ++i)
			//{
			//	double variance = shape->GetVariance(i);

			//	mat alpha1 = alpha;
			//	alpha1[i] -= H;
			//	Alpha alpha_para1(alpha1, input, model, mesh, shape, texture);
			//	double first_derivative1 = weight * ComputeIntensityGradient(input, &alpha_para1, i) + 2 * alpha1[i] / variance;

			//	mat alpha2 = alpha;
			//	alpha2[i] += H;
			//	Alpha alpha_para2(alpha2, input, model, mesh, shape, texture);
			//	double first_derivative2 = weight * ComputeIntensityGradient(input, &alpha_para2, i) + 2 * alpha2[i] / variance;

			//	double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
			//	alpha_hessian_inv(i, i) = 1 / (second_derivative + 2 / variance);
			//}
		}



		// Generate random points  
		GenerateRandomPoints(model, GradientRandomNum);
		double function_value = 0;
		function_value = ComputeCost(input, model, alpha_para);
		ofstream cost;
		cost.open("first_cost", ios::app);
		cost << function_value << "\n";
		cost.close();



		for (int i = 0; i < para_num; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight * ComputeIntensityGradient(input, &alpha_para, i) + 2 * alpha[i] / variance;
		}


		//for (int i = 0; i < 10; ++i)
		//{
		//	double variance = texture->GetVariance(i);
		//	beta_gradient[i] = weight *ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
		//}


		//for (int i = 0; i < RhoNum; ++i)
		//{
		//	double first_derivative1 = 1.0 / 1000*ComputeIntensityGradient(input, &rho_para, i);
		//	//double variance = rho_para.GetVariance(i);
		//	//double mean = rho_para.GetMean(i);
		//	rho_gradient[i] = first_derivative1;// +2 * (rho[i] - mean) / variance;
		//}

		//for (int i = 0; i < LamdaNum-7; ++i)
		//{
		//	double first_derivative1 = 1.0 / 1000 * ComputeIntensityGradient(input, &lamda_para, i);
		//	//double variance = lamda_para.GetVariance(i);
		//	//double mean = lamda_para.GetMean(i);
		//	lamda_gradient[i] = first_derivative1; //+2 * (lamda[i] - mean) / variance;
		//}

		alpha -= alpha_para.Step*alpha_gradient;

		//beta -= beta_para.Step*beta_gradient;
		//rho -= step_mat*rho_gradient;   
		//lamda -= lamda_para.Step*lamda_gradient;  
		//alpha -= 0.00008*alpha_hessian_inv*alpha_gradient;


		++counter;
	}

	// restore original version
	input= make_shared<InputImage>(ready);
	model->SetSize(IMAGE_WIDTH, IMAGE_HEIGHT);
	rho[FOCAL] *= 2;

	Face3dModel face3d_model(shape, texture);
	mesh = face3d_model.Construction(alpha, beta);
	VisualizeResult(rho, lamda, input, mesh, "first");
}




void Optimizer::TwoPassZbuffer(const CameraParameter& rho, const IlluminationParameter& lamda, MeshPtr mesh, ModelPtr model, bool segment)
{
	int width = model->Rows();
	int height = model->Cols();

	mesh->UpdateVertexNormal();
	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho, width, height);
	shared_ptr<Camera> light = make_shared<PinholeCamera>(rho, lamda, width, height);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);

	HardwareRender render(camera, illumination);
	vector<float> depth_map(width*height);
	render.Rendering(mesh, lamda, depth_map);

	HardwareRender render2(light, illumination);
	vector<float> shadow_map(width*height);
	render2.Rendering(mesh, lamda, shadow_map);

	shared_ptr<DepthBuffer> object_depth = nullptr;
	shared_ptr<BoxRaster> raster = nullptr;
	SoftwareRender object_render(camera, illumination, object_depth, raster);
	if (segment)
	{
		object_render.TwoPassZbufferSegment(light, mesh, model, depth_map, shadow_map);
	}
	else
	{
		object_render.TwoPassZbuffer(light, mesh, model, depth_map, shadow_map);
	}
}



void Optimizer::FitAll(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture)
{

	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat beta_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat rho_gradient = mat(RhoNum, 1, fill::zeros);
	mat rho_hessian_inv = mat(RhoNum, RhoNum, fill::zeros);

	mat lamda_gradient = mat(LamdaNum, 1, fill::zeros);
	mat lamda_hessian_inv = mat(LamdaNum, LamdaNum, fill::zeros);


	vec step(RhoNum, 1);
	step.rows(0, 2).fill(0.00000006);
	step.rows(3, 5).fill(0.0002);
	step.rows(6, 6).fill(2.0);
	mat step_mat = diagmat(step);


	SetStartingValue(rho, lamda);

	// 2500 1000 700 500 300 200
	array<double, 6> weight_intensity = { { 1.0 / 1000, 1.0 / 900, 1.0 / 800, 1.0 / 700, 1.0 / 500, 1.0 / 300 } };
	array<int, 6> para_num = { { 10, 15, 25, 35, 55, 99 } };   // 20 40
	array<int, 6> iterations = { { 1000, 1000, 1000, 1000, 1000, 1000 } };


	//array<double, 4> weight_intensity = { { 1.0 / 900, 1.0 / 700, 1.0 / 500, 1.0 / 400 } };
	//array<int, 4> para_num = { { 10, 20, 40, 99 } };   // 20 40
	//array<int, 4> iterations = { { 1000, 1000, 800, 600 } };


	int counter = 0;
	for (int c = 0; c < 6; ++c)
	{
		for (int l = 0; l < iterations[c]; ++l)
		{

			if (counter == 0 || counter % 1000 == 0)
			{
				Face3dModel face3d_model(shape, texture);
				mesh = face3d_model.Construction(alpha, beta);
				TwoPassZbuffer(rho, lamda, mesh, model);
				model->EnableIterator();
				model->InitialRandomGenerator();

				//GenerateRandomPoints(model, HessianRandomNum);

				//for (int i = 0; i < para_num[c]; ++i)
				//{
				//	double variance = shape->GetVariance(i);

				//	mat alpha1 = alpha;
				//	alpha1[i] -= H;
				//	Alpha alpha_para1(alpha1, input, model, mesh, shape, texture);

				//	double first_derivative1 = weight_feature[c] * ComputeLandmarkGradient(input, &alpha_para1, i) +
				//		weight_intensity[c] * ComputeIntensityGradient(input, &alpha_para1, i);


				//	mat alpha2 = alpha;
				//	alpha2[i] += H;
				//	Alpha alpha_para2(alpha2, input, model, mesh, shape, texture);
				//	double first_derivative2 = weight_feature[c] * ComputeLandmarkGradient(input, &alpha_para2, i) +
				//		weight_intensity[c] * ComputeIntensityGradient(input, &alpha_para2, i);

				//	double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
				//	alpha_hessian_inv(i, i) = 1 / (second_derivative +2 / variance);

				//}


				/*	for (int i = 0; i < para_num[c]; ++i)
					{
					double variance = texture->GetVariance(i);

					mat beta1 = beta;
					beta1[i] -= H;
					Beta beta_para1(beta1, model, mesh, shape, texture);
					double first_derivative1 = weight_intensity[c] * ComputeIntensityGradient(input, &beta_para1, i);

					mat beta2 = beta;
					beta2[i] += H;
					Beta beta_para2(beta2, model, mesh, shape, texture);
					double first_derivative2 = weight_intensity[c] * ComputeIntensityGradient(input, &beta_para2, i);

					double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
					beta_hessian_inv(i, i) = 1 / (second_derivative + 2 / variance);

					}*/


				//for (int i = 0; i < RhoNum; ++i)
				//{
				//	double variance = GetRhoVariance(i);

				//	mat rho1 = rho;
				//	rho1[i] -= H;
				//	Rho rho_para1(rho1, input, model, mesh, shape, texture);
				//	double first_derivative1 =/* weight_feature[c] * ComputeLandmarkGradient(input, &rho_para1, i) +*/
				//		weight_intensity[c] * ComputeIntensityGradient(input, &rho_para1, i);

				//	mat rho2 = rho;
				//	rho2[i] += H;
				//	Rho rho_para2(rho2, input, model, mesh, shape, texture);

				//	double first_derivative2 = /*weight_feature[c] * ComputeLandmarkGradient(input, &rho_para2, i) +*/
				//		weight_intensity[c] * ComputeIntensityGradient(input, &rho_para2, i);

				//	double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);
				//	rho_hessian_inv(i, i) = 1 / (second_derivative + 2 / variance);

				//}


				//for (int i = 0; i < LamdaNum - 7; ++i)
				//{

				//	double variance = GetLamdaVariance(i);
				//	mat lamda1 = lamda;
				//	lamda1[i] -= H;
				//	Lamda lamda_para1(lamda1, model, mesh, shape, texture);
				//	double first_derivative1 = weight_intensity[c] * ComputeIntensityGradient(input, &lamda_para1, i);


				//	mat lamda2 = lamda;
				//	lamda2[i] += H;
				//	Lamda lamda_para2(lamda2, model, mesh, shape, texture);
				//	double first_derivative2 = weight_intensity[c] * ComputeIntensityGradient(input, &lamda_para2, i);
				//	double second_derivative = (first_derivative2 - first_derivative1) / (2 * H);

				//	lamda_hessian_inv(i, i) = 1 / (second_derivative + 2 / variance);

				//}

			}

			Alpha alpha_para(alpha, input, model, mesh, shape, texture);
			Beta beta_para(beta, model, mesh, shape, texture);
			Rho rho_para(rho, input, model, mesh, shape, texture);
			Lamda lamda_para(lamda, model, mesh, shape, texture);

			GenerateRandomPoints(model, GradientRandomNum);
			//double function_value = 0;
			//function_value = ComputeCost(input, model, alpha_para);
			//ofstream cost;
			//cost.open("all_cost", ios::app);
			//cost << function_value << "\n";
			//cost.close();
		

			for (int i = 0; i < para_num[c]; ++i)
			{
				double variance = shape->GetVariance(i);
				alpha_gradient[i] = /*weight_feature[c] * ComputeLandmarkGradient(input, &alpha_para, i) +*/
					weight_intensity[c] * ComputeIntensityGradient(input, &alpha_para, i) +2 * alpha[i] / variance;
			}


			for (int i = 0; i < para_num[c]; ++i)
			{
				double variance = texture->GetVariance(i);
				beta_gradient[i] = weight_intensity[c] * ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
			}


			for (int i = 0; i < RhoNum; ++i)
			{
				//double variance = GetRhoVariance(i);
				//double mean = GetRhoMean(i);
				rho_gradient[i] = /*weight_feature[c] * ComputeLandmarkGradient(input, &rho_para, i) +*/
					weight_intensity[c] * ComputeIntensityGradient(input, &rho_para, i);// +2 * (rho[i] - mean) / variance;
			}


			for (int i = 0; i < LamdaNum; ++i)
			{
				double variance = GetLamdaVariance(i);
				double mean = GetLamdaMean(i);
				lamda_gradient[i] = weight_intensity[c] * ComputeIntensityGradient(input, &lamda_para, i) +2 * (lamda[i] - mean) / variance;
			}


			//alpha -= 0.00003*alpha_hessian_inv*alpha_gradient;  // 0.01  0.00003
			//beta -= 0.01*beta_hessian_inv*beta_gradient;
			//lamda -= 0.03*lamda_hessian_inv*lamda_gradient;
			//rho -= 0.0001*rho_hessian_inv*rho_gradient;

			alpha -= alpha_para.Step*alpha_gradient;
		    beta -= beta_para.Step*beta_gradient;
			rho -= step_mat*rho_gradient;   // gradient descent
			lamda -= lamda_para.Step*lamda_gradient;

			++counter;
		}

		Face3dModel face3d_model(shape, texture);
		mesh = face3d_model.Construction(alpha, beta);
		VisualizeResult(rho, lamda, input, mesh, "segmented", c);
	}
	 
}




void Optimizer::FitSegments(mat& alpha,
	mat& beta, 
	mat& rho, 
	mat& lamda, 
	InputPtr input, 
	ModelPtr model,
	MeshPtr mesh, 
	ShapePtr shape,
	TexturePtr texture)
{

	mat eye_alpha = alpha;
	mat eye_beta = beta;
	mat nose_alpha = alpha;
	mat nose_beta = beta;
	mat mouth_alpha = alpha;
	mat mouth_beta = beta;
	mat rest_alpha = alpha;
	mat rest_beta = beta;


	Face3dModel face3d_model(shape, texture);
	mesh = face3d_model.Construction(alpha, beta);
	TwoPassZbuffer(rho, lamda, mesh, model, true); // cal segment two pass z-buffer


	for (int c = 0; c < 3; ++c)
	{	
		model->EnableIterator(ModelImage::NOSE);
		FitNose(nose_alpha, nose_beta, rho, lamda, input, model, mesh, shape, texture);

		model->EnableIterator(ModelImage::EYE);
		FitEye(eye_alpha, eye_beta, rho, lamda, input, model, mesh, shape, texture);

		model->EnableIterator(ModelImage::MOUTH);
		FitMouth(mouth_alpha, mouth_beta, rho, lamda, input, model, mesh, shape, texture);

		model->EnableIterator(ModelImage::REST);
		FitRest(rest_alpha, rest_beta, rho, lamda, input, model, mesh, shape, texture);

	}


	mat all_alpha(PrincipalNum,SegmentsNum);
	all_alpha.cols(0, 0) = nose_alpha;
	all_alpha.cols(1, 1) = eye_alpha;
	all_alpha.cols(2, 2) = mouth_alpha;
	all_alpha.cols(3, 3) = rest_alpha;

	mat all_beta(PrincipalNum, SegmentsNum);
	all_beta.cols(0, 0) = nose_beta;
	all_beta.cols(1, 1) = eye_beta;
	all_beta.cols(2, 2) = mouth_beta;
	all_beta.cols(3, 3) = rest_beta;


	mesh = face3d_model.Construction(all_alpha, all_beta);

	mesh->Blend();
	rho.quiet_save("rho_para", arma_binary);
	lamda.quiet_save("lamda_para", arma_binary);

}



void Optimizer::FitNose(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture)
{

	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat beta_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	Rho rho_para(rho, input, model, mesh, shape, texture);
	Lamda lamda_para(lamda, model, mesh, shape, texture);


	// 2500 1000 700 500 300 200
	model->InitialRandomGenerator(ModelImage::NOSE);
	double weight = 1.0 / 200;

	for (int l = 0; l < 1000; ++l)  // 400 or 500
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);

		// Generate random points  
		GenerateRandomPoints(model, GradientRandomNum);


		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight * ComputeIntensityGradient(input, &alpha_para, i) + 2 * alpha[i] / variance;
		}

		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = texture->GetVariance(i);
			beta_gradient[i] = weight * ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
		}

		alpha -= alpha_para.Step*alpha_gradient;
		beta -= beta_para.Step*beta_gradient;

	}

}




void Optimizer::FitEye(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture)
{

	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat beta_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	Rho rho_para(rho, input, model, mesh, shape, texture);
	Lamda lamda_para(lamda, model, mesh, shape, texture);


	// 2500 1000 700 500 300 200
	model->InitialRandomGenerator(ModelImage::EYE);
	double weight = 1.0 / 200;


	for (int l = 0; l < 1000; ++l)
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);

		// Generate random points  
		GenerateRandomPoints(model, GradientRandomNum);


		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight * ComputeIntensityGradient(input, &alpha_para, i) + 2 * alpha[i] / variance;
		}

		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = texture->GetVariance(i);
			beta_gradient[i] = weight * ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
		}

		alpha -= alpha_para.Step*alpha_gradient;
		beta -= beta_para.Step*beta_gradient;


	}

}



void Optimizer::FitMouth(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture)
{
	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat beta_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	Rho rho_para(rho, input, model, mesh, shape, texture);
	Lamda lamda_para(lamda, model, mesh, shape, texture);

	// 2500 1000 700 500 300 200

	model->InitialRandomGenerator(ModelImage::MOUTH);

	double weight = 1.0 / 200;


	for (int l = 0; l < 1000; ++l)
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);

		// Generate random points  
		GenerateRandomPoints(model, GradientRandomNum);

		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight * ComputeIntensityGradient(input, &alpha_para, i) + 2 * alpha[i] / variance;
		}

		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = texture->GetVariance(i);
			beta_gradient[i] = weight * ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
		}

		alpha -= alpha_para.Step*alpha_gradient;
		beta -= beta_para.Step*beta_gradient;

	}

}



void Optimizer::FitRest(mat& alpha,
	mat& beta,
	mat& rho,
	mat& lamda,
	InputPtr input,
	ModelPtr model,
	MeshPtr mesh,
	ShapePtr shape,
	TexturePtr texture)
{

	mat alpha_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat alpha_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);

	mat beta_gradient = mat(PrincipalNum, 1, fill::zeros);
	mat beta_hessian_inv = mat(PrincipalNum, PrincipalNum, fill::zeros);


	Rho rho_para(rho, input, model, mesh, shape, texture);
	Lamda lamda_para(lamda, model, mesh, shape, texture);


	// 2500 1000 700 500 300 200

	model->InitialRandomGenerator(ModelImage::REST);

	double weight = 1.0 / 200;


	for (int l = 0; l < 1000; ++l)
	{

		Alpha alpha_para(alpha, input, model, mesh, shape, texture);
		Beta beta_para(beta, model, mesh, shape, texture);

		// Generate random points  
		GenerateRandomPoints(model, GradientRandomNum);

		double function_value = 0;


		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = shape->GetVariance(i);
			alpha_gradient[i] = weight * ComputeIntensityGradient(input, &alpha_para, i) + 2 * alpha[i] / variance;
		}

		for (int i = 0; i < PrincipalNum; ++i)
		{
			double variance = texture->GetVariance(i);
			beta_gradient[i] = weight * ComputeIntensityGradient(input, &beta_para, i) + 2 * beta[i] / variance;
		}

		alpha -= alpha_para.Step*alpha_gradient;
		beta -= beta_para.Step*beta_gradient;

	}

}





void Optimizer::VisualizeResult(const CameraParameter& rho,											
	const IlluminationParameter& lamda, 											
	InputPtr input_image,				
	MeshPtr mesh, 		
	const std::string& name,
	int num)
{
	stringstream file;
	file.clear();
	file.str("");
	file << num;
	string file_num = file.str();

	//cv::Mat ready_copy = input_image->Clone();
	//shared_ptr<ModelImage> model = make_shared<ModelImage>(ready_copy);
	//CpuRendering(rho, lamda, mesh, model);
	//model->Write("cpu"+name+ file_num + ".png");
	//model->Show();

	shared_ptr<ModelImage> model2 = make_shared<ModelImage>();
	GpuRendering(rho, lamda, mesh, model2);
	model2->Write(name + file_num + ".png");
	//model2->Show();

}

void Optimizer::CpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	MeshPtr mesh,
	ModelPtr model)
{
	int width = model->Rows();
	int height = model->Cols();

	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho, width, height);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);
	shared_ptr<DepthBuffer> depth = make_shared<DepthBuffer>(width, height);
	shared_ptr<BoxRaster> raster = make_shared<BoxRaster>();
	SoftwareRender cpu_render(camera, illumination, depth, raster);
	cpu_render.Rending(mesh, model);

}


void Optimizer::GpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	MeshPtr mesh,
	ModelPtr model)
{
	int width = model->Rows();
	int height = model->Cols();

	mesh->UpdateVertexNormal();
	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho, width, height);
	shared_ptr<Camera> light = make_shared<PinholeCamera>(rho, lamda, ShadowResolution, ShadowResolution);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);

	//HardwareRender gpu_render(camera, light,illumination);
	HardwareRender gpu_render(camera, illumination);
	gpu_render.RenderingWithBackground(mesh, model, lamda);
	//gpu_render.RenderingWithShadow(mesh, lamda);
}




double Optimizer::ComputeCost(InputPtr input_image, ModelPtr model, Alpha& alpha) const
{
	double function_value = 0;
	
	int random_num = 1000;
	vector<int> triangle_ids;
	model->GenerateRandomNumbers(random_num, triangle_ids);	
	//int random_num = random_points_.size();

	for (int i = 0; i < random_num; ++i)
	{
		int id = triangle_ids[i];
    	//int id = random_points_[i];

		ivec2 pos = alpha.ComputePosition(id);
		vec3 model_color = alpha.ComputeColor(id);  // in RGB order 
		vec3 input_color = input_image->GetColor(Point(pos[0], pos[1])); // in RGB order
		vec3 diff = (model_color - input_color);
		function_value += dot(diff, diff);
	}
	return 40.0/1000*function_value;
}


double Optimizer::ComputeCost(InputPtr input_image,  Alpha& alpha) const
{
	double function_value = 0;

	for (int i = 0; i < LandmarkNum; ++i)
	{
		vec2 model_pos = alpha.ComputeLandmarkPosition(i);
		vec2 input_pos = input_image->LandmarkPosition(i);
		vec2 diff = (model_pos - input_pos);
		function_value += dot(diff, diff);
	}

	return function_value;

}



double Optimizer::ComputeGradient(CostFunction* cost, Parameter* para, int index) const
{
	return cost->ComputeGradient(para, index);
}


double Optimizer::ComputeIntensityGradient(InputPtr input, Parameter*para, int index) const
{
	IntensityCost intensity_cost(random_points_, input);
	return ComputeGradient(&intensity_cost, para, index);
}


double Optimizer::ComputeLandmarkGradient(InputPtr input, Parameter*para, int index) const
{
	LandmarkCost landmark_cost(input);
	return ComputeGradient(&landmark_cost, para, index);
}



void Optimizer::GenerateRandomPoints(ModelPtr model, int num)
{

	model->GenerateRandomNumbers(num,random_points_);

	//ofstream random;
 //   random.open("random_points", ios::app);
	//for (auto e:random_points_)
	//{
	//	//cout << e << endl;
	//	random << e << "\n";
	//}
	//random.close();
}


void Optimizer::ShowRandomPoints(InputPtr input, ModelPtr model, Parameter* para) const
{
	int num = random_points_.size();

	for (int t = 0; t < num; ++t)
	{
		int id = random_points_[t];
		ivec2 pos = para->ComputePosition(id);

		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));

		Vec3b color(0, 0, 0);
		model->WriteColor(pos[0], pos[1], color);
	}

}


void Optimizer::ShowVisiblePoints(InputPtr input, ModelPtr model, Parameter* para) const
{
	int num = model->Size();
	for (int i = 0; i<num;++i)
	{
		ivec2 pos = para->ComputePosition(i);
		vec3 rgb = para->ComputeColor(i);
		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
		Vec3b color(0, 0, 255);
		model->WriteColor(pos[0], pos[1], color);
	}
	cout << "visible triangle number = " << num << endl;
}


void Optimizer::ShowShadowPoints(InputPtr input, ModelPtr model, Parameter* para) const
{
	int num = model->Size();
	int shadow_counter = 0;
	for (int i = 0; i< num; ++i)
	{
		
		if (model->visible_triangles_[i].cast_shadow)
		{
			++shadow_counter;
			ivec2 pos = para->ComputePosition(i);
			vec3 rgb = para->ComputeColor(i);
			//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//	Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
			Vec3b color(0, 0, 255);
			model->WriteColor(pos[0], pos[1], color);
		}
	}
	cout << "shadow triangle number = " << shadow_counter << endl;

}


void Optimizer::ShowSegmentPoints(InputPtr input, ModelPtr model, Parameter* para) const
{
	int num = 0;

	model->EnableIterator(ModelImage::NOSE);
	num = model->Size(ModelImage::NOSE);

	for (int i = 0; i < num; ++i)
	{
		ivec2 pos = para->ComputePosition(i);
		vec3 rgb = para->ComputeColor(i);
		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
		Vec3b color(0, 0, 255);
		model->WriteColor(pos[0], pos[1], color);
	}


	model->EnableIterator(ModelImage::EYE);
	num = model->Size(ModelImage::EYE);

	for (int i = 0; i < num; ++i)
	{
		ivec2 pos = para->ComputePosition(i);
		vec3 rgb = para->ComputeColor(i);
		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
		Vec3b color(0, 255, 0);
		model->WriteColor(pos[0], pos[1], color);
	}


	model->EnableIterator(ModelImage::MOUTH);
	num = model->Size(ModelImage::MOUTH);

	for (int i = 0; i < num; ++i)
	{
		ivec2 pos = para->ComputePosition(i);
		vec3 rgb = para->ComputeColor(i);
		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
		Vec3b color(0, 255, 255);
		model->WriteColor(pos[0], pos[1], color);
	}


	model->EnableIterator(ModelImage::REST);
	num = model->Size(ModelImage::REST);

	for (int i = 0; i < num; ++i)
	{
		ivec2 pos = para->ComputePosition(i);
		vec3 rgb = para->ComputeColor(i);
		//vec3 rgb = input_image->GetColor(Point(pos[0], pos[1]));
		//Vec3b color(static_cast<uchar>(rgb[2]), static_cast<uchar>(rgb[1]), static_cast<uchar>(rgb[0]));
		Vec3b color(255, 0, 0);
		model->WriteColor(pos[0], pos[1], color);
	}

}




void Optimizer::SetStartingValue(const mat& rho, const mat& lamda)
{
	for (int i = 0; i < RhoNum;++i)
	{
		rho_mean_[i] = rho[i];
	}

	for (int i = 0; i < LamdaNum;++i)
	{
		lamda_mean_[i] = lamda[i];
	}
}
