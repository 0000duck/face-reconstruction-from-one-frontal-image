#include <memory>
#include<gl/glew.h>
#include<glm/glm.hpp>

#include"hardware_render.h"
#include"mesh.h"
#include "face_mesh.h"
#include"camera_parameter.h"
#include"illumination_parameter.h"
#include "pinhole_camera.h"
#include "phong_illumination.h"
#include "shader.h"
#include "opengl_common.h"

using namespace std;

void GpuRendering(bool shadow)
{
	shared_ptr<Mesh> mesh = make_shared<FaceMesh>();
	mesh->UpdateVertexNormal();

	CameraParameter rho;
	IlluminationParameter lamda;

	shared_ptr<Camera> camera = make_shared<PinholeCamera>(rho);
	shared_ptr<Camera> light = make_shared<PinholeCamera>(rho, lamda, ShadowResolution,ShadowResolution);
	shared_ptr<Illumination> illumination = make_shared<PhongIllumination>(lamda);


	HardwareRender render(light,illumination);
	HardwareRender render2(camera, light, illumination);


	if (shadow == false)
	{
		render.Rendering(mesh, lamda);
		//render.RenderingWithBackground(mesh, model, lamda);
	}
	else
	{
		render2.RenderingWithShadow(mesh, lamda);
	}

}




