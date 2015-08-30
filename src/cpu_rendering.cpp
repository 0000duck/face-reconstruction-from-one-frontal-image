#include "cpu_rendering.h"
#include "software_render.h"
#include "face_mesh.h"
#include "pinhole_camera.h"
#include "camera_parameter.h"
#include "phong_illumination.h"
#include "box_raster.h"

#include "model_image.h"
#include "image_process.h"
#include <memory>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;



void CpuRendering()
{
	std::shared_ptr<ModelImage> model=make_shared<ModelImage>();
	shared_ptr<Mesh> mesh = make_shared<FaceMesh>();

	CameraParameter rho;
	IlluminationParameter lamda;

	int width = model->Rows();
	int height = model->Cols();

	shared_ptr<Camera> object_camera = make_shared<PinholeCamera>(rho, width, height);
	shared_ptr<DepthBuffer> object_depth = make_shared<DepthBuffer>(width, height);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);
	shared_ptr<BoxRaster> raster = make_shared<BoxRaster>();

	SoftwareRender object_render(object_camera, illumination, object_depth, raster);
	object_render.Rending(mesh, model);

	model->Show();
}



