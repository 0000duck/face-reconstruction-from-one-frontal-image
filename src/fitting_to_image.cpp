#include "fitting_to_image.h"
#include "face_mesh.h"
#include "camera_parameter.h"
#include "illumination_parameter.h"
#include "model_image.h"

#include "pinhole_camera.h"
#include "phong_illumination.h"
#include "depth_buffer.h"
#include "box_raster.h"

#include "software_render.h"
#include "hardware_render.h"

#include "image_process.h"
#include "input_image.h"

#include "face3d_shape.h"
#include "face3d_texture.h"
#include "face3d_model.h"

#include "optimizer.h"
#include "Parameter.h"

#include "opengl_common.h"

#include <armadillo>
#include <gl/glew.h>


using namespace std;
using namespace cv;
using namespace arma;




void TwoPassZbuffer(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage>model);

void  CpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage>model);

void  GpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage> model);


void VisualizeResult(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<InputImage> input_image,
	shared_ptr<Mesh> mesh,
	const string& name,
	int num=0);

void  DrawLandmark(const cv::Mat& background,
	const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	const string& file_name);



shared_ptr<Mesh> Fitting2Image()
{

	/** input image */
	cv::Mat src = imread(ImageDir+"//front/yf.jpg");
	cv::Mat ready;
	shared_ptr<InputImage> input = PreprocessImage(src, ready);
	
	//ready = cv::Mat(ready.rows, ready.cols, CV_8UC3, Scalar(255, 255, 255));

	imwrite("background.jpg", ready);


	/** model image*/
	shared_ptr<ModelImage> model = make_shared<ModelImage>();

	/** mean mesh*/
	shared_ptr<Mesh> mesh = make_shared<FaceMesh>(false);


	/** morphable model */
	shared_ptr<Face3dShape> shape = make_shared<Face3dShape>();
	shared_ptr<Face3dTexture> texture = make_shared<Face3dTexture>();

	/** render */
	CameraParameter rho;
	IlluminationParameter lamda;



	/** final result*/
	//mesh = make_shared<FaceMesh>();
	//mat rho_para;
	//mat lamda_para;
	//rho_para.quiet_load("rho_para", arma_binary);
	//lamda_para.quiet_load("lamda_para", arma_binary);

	//rho = rho_para;
	//lamda = lamda_para;

	//VisualizeResult(rho, lamda, input, mesh, "front");
	//rho.rotation_[1] += 0.45;
	//VisualizeResult(rho, lamda, input, mesh, "side");



	/** fitter*/
	Optimizer fitter;


	/** initialization*/
	mat alpha(PrincipalNum, 1, fill::zeros);
	mat beta(PrincipalNum, 1, fill::zeros);
	mat rho_mat(rho);
	mat lamda_mat(lamda);

	//DrawLandmark(ready, rho, lamda, mesh, "feature.jpg");
	//fitter.TwoPassZbuffer(rho, lamda, mesh, model);
	//fitter.ShowPoints(alpha, beta, rho_mat, lamda_mat, input, model, mesh, shape, texture, 1);


	/** Alignment*/
	fitter.FitPose(alpha, rho_mat, lamda_mat, input, model, mesh, shape, texture);
	//fitter.RoughAlignment(alpha,rho_mat, lamda_mat, input_image, model, mesh,shape,texture);

	Face3dModel face3d_model(shape, texture);
	mesh = face3d_model.Construction(alpha, beta);
	rho = rho_mat;
	VisualizeResult(rho, lamda, input, mesh, "position");

	fitter.FitIllumination(alpha, beta, rho_mat, lamda_mat, input, model, mesh, shape, texture,ready);
	//model->Show();
	
	//fitter.FitFirstPrincipalComponents(alpha, beta, rho_mat, lamda_mat, input, model, mesh, shape, texture, ready);

	/**middle iterations*/
    fitter.FitAll(alpha, beta, rho_mat, lamda_mat, input, model, mesh, shape, texture);


	fitter.FitSegments(alpha, beta, rho_mat, lamda_mat, input, model, mesh, shape, texture);



	return mesh;
}



void TwoPassZbuffer(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage>model)
{
	int width = model->Rows();
	int height = model->Cols();

	mesh->UpdateVertexNormal();
	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho,width,height);
	shared_ptr<Camera> light = make_shared<PinholeCamera>(rho, lamda,ShadowResolution,ShadowResolution);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);

	HardwareRender render(camera, illumination);
	vector<float> depth_map(width*height);
	render.Rendering(mesh, lamda, depth_map);

	HardwareRender render2(light, illumination);
	vector<float> shadow_map(ShadowResolution*ShadowResolution);
	render2.Rendering(mesh, lamda, shadow_map);

	shared_ptr<DepthBuffer> object_depth = nullptr;
	shared_ptr<BoxRaster> raster = nullptr;
	SoftwareRender object_render(camera, illumination, object_depth, raster);
	object_render.TwoPassZbuffer(light, mesh, model, depth_map, shadow_map);
	//object_render.VertexTwoPassZbuffer(light, mesh, model, depth_map, shadow_map);
}



void  CpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage> model)
{
	int width = model->Rows();
	int height = model->Cols();

	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho,width,height);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);
	shared_ptr<DepthBuffer> depth = make_shared<DepthBuffer>(width, height);
	shared_ptr<BoxRaster> raster = make_shared<BoxRaster>();
	SoftwareRender cpu_render(camera, illumination, depth, raster);
	cpu_render.Rending(mesh, model);
	//cpu_render.DrawLandmark(mesh, model);
}


void  GpuRendering(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<Mesh> mesh,
	shared_ptr<ModelImage> model)

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
	//gpu_render.RenderingWithShadow(mesh, model, lamda);
}


void  DrawLandmark(const cv::Mat& background,
								const CameraParameter& rho,
								const IlluminationParameter& lamda,
								shared_ptr<Mesh> mesh,
								const string& file_name)
{
	shared_ptr<ModelImage> model = make_shared<ModelImage>();
	GpuRendering(rho, lamda, mesh, model);
	CpuRendering(rho, lamda, mesh, model);
}




void VisualizeResult(const CameraParameter& rho,
	const IlluminationParameter& lamda,
	shared_ptr<InputImage> input_image,
	shared_ptr<Mesh> mesh,
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
	//model->Write(name + file_num + ".jpg");
	//model->Show();

	shared_ptr<ModelImage> model2 = make_shared<ModelImage>();
	GpuRendering(rho, lamda, mesh, model2);
	model2->Write(name + file_num + ".png");
	//model2->Show();

}
