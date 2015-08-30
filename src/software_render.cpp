#include "software_render.h"
#include "pinhole_camera.h"
#include "phong_illumination.h"
#include "box_Raster.h"
#include "hardware_render.h"
#include "common.h"
#include "opengl_common.h"
#include "common_function.h"

#include <vector>
#include <cmath>
#include <limits>


using namespace arma;
using namespace std;
using namespace cv;

SoftwareRender::SoftwareRender(CameraPtr camera, IlluminationPtr illumination, DepthPtr depth, RasterPtr raster)
	:camera_(camera), illumination_(illumination), depth_(depth), raster_(raster)
{
	width_ = camera_->Width();
	height_ = camera_->Height();
}


void SoftwareRender::Rending(MeshPtr mesh, ModelPtr model)
{
	Vertex triangle_center;
	for (int i=0;i<TriangleNum;++i)
	{		
		PrimitiveAssembly(triangle_center,mesh,i);
		VertexShader(triangle_center);
		if(raster_->Rasterization(triangle_center,*depth_))
		{
			FragmentShader(triangle_center,model);
		}
	}
}


void SoftwareRender::Rasterization(MeshPtr mesh) 
{
	Vertex triangle_center;
	for (int i = 0; i < TriangleNum; ++i)
	{
		ComputeCenterCoordinate(triangle_center, mesh, i);
		triangle_center.attribute[ATTRIBUTE_2D_COORDINATE].rows(0, 2) = World2Screen(triangle_center.attribute[ATTRIBUTE_3D_COORDINATE]);
		raster_->Rasterization(triangle_center, *depth_);
	}
}


void SoftwareRender::DrawLandmark(MeshPtr mesh, ModelPtr model)
{

	Vertex vertex;
	for (int i = 0; i < LandmarkNum; ++i)
	{

     	int id = mesh->Landmarks(i);
		cout << id << endl;
		vertex.attribute[ATTRIBUTE_2D_COORDINATE].rows(0, 2) = World2Screen(mesh->GetVertexCoordinate(id));
		int opengl_x = static_cast<int>(vertex.attribute[ATTRIBUTE_2D_COORDINATE][0]);
		int opengl_y = static_cast<int>(vertex.attribute[ATTRIBUTE_2D_COORDINATE][1]);
		model->Circle(Point(opengl_x, IMAGE_HEIGHT - 1 - opengl_y));
		model->Show("feature");
		waitKey(0);		
	}
	//model->Write("model feature.jpg");
}



void SoftwareRender::TwoPassZbuffer(CameraPtr shadow_camera, 
	MeshPtr mesh, 
	ModelPtr model, 
	std::vector<float>& depth_map, 
	std::vector<float>& shadow_map)
{

	Primitive primitive;
	vec4 center;
	VisibleTriangle visible_triangle;

	model->Clear();

	int visible_counter = 0;
	int shadow_counter = 0;
	int zero_area_counter = 0;

	for (int i = 0; i < TriangleNum; ++i)
	{
		// do triangle center visibility test
		PrimitiveAssembly(primitive, mesh, i);
		center = (primitive.vertices[0].attribute[ATTRIBUTE_3D_COORDINATE] +
						primitive.vertices[1].attribute[ATTRIBUTE_3D_COORDINATE] +
						primitive.vertices[2].attribute[ATTRIBUTE_3D_COORDINATE]) / 3;

		vec3 center_screen = World2Screen(center);
		int x = static_cast<int>(center_screen[0]);
		int y = static_cast<int>(center_screen[1]);
		double z = center_screen[2];

		int camera_z = BoxRaster::Float2Fixed(z, 24);
		int camera_depth = BoxRaster::Float2Fixed(depth_map[y*width_ + x], 24);

		vec3 normal = mesh->GetTriangleNormal(i);
		if (IsVisible(camera_z, camera_depth, 1000) && BackFaceCulling(normal))  //688
		{
			VertexShader(primitive);
			int area = BoxRaster::Float2Fixed(ComputeTriangleArea(primitive), 8); // best choice 4

			if (area != 0)
			{
				++visible_counter;

				visible_triangle.id = i;
				visible_triangle.area = area;

				// compute cast shadow
				vec4 center_clip = shadow_camera->OpenglTransform(center);
				vec3 center_screen = shadow_camera->ViewportTransform(shadow_camera->PerspectiveDivision(center_clip));

				int light_x = static_cast<int>(center_screen[0]);
				int light_y = static_cast<int>(center_screen[1]);
				int light_z = BoxRaster::Float2Fixed(center_screen[2], 12);
				int light_depth = BoxRaster::Float2Fixed(shadow_map[light_y*width_ + light_x], 12);

				if (!IsVisible(light_z, light_depth, 50))  // 12 or  6 or 50
				{
					++shadow_counter;
					visible_triangle.cast_shadow = true;
				}
				else
				{
					visible_triangle.cast_shadow = false;
				}

				model->PushBack(visible_triangle);
			}
			else
			{
				++zero_area_counter;
			}
		}
	}
	cout << "visible triangle num = " << visible_counter << endl;
	cout << "shadow triangle num = " << shadow_counter << endl;
	cout << "zero area num = " <<zero_area_counter<< endl;
	cout << "=================================================" << endl;

	model->Shrink2Fit();

}



void SoftwareRender::TwoPassZbufferSegment(CameraPtr shadow_camera,
	MeshPtr mesh,
	ModelPtr model,
	std::vector<float>& depth_map,
	std::vector<float>& shadow_map)
{

	Primitive primitive;
	vec4 center;
	VisibleTriangle visible_triangle;


	int visible_counter = 0;
	int shadow_counter = 0;
	int zero_area_counter = 0;

	for (int i = 0; i < TriangleNum; ++i)
	{
		// do triangle center visibility test
		PrimitiveAssembly(primitive, mesh, i);
		center = (primitive.vertices[0].attribute[ATTRIBUTE_3D_COORDINATE] +
			primitive.vertices[1].attribute[ATTRIBUTE_3D_COORDINATE] +
			primitive.vertices[2].attribute[ATTRIBUTE_3D_COORDINATE]) / 3;

		vec3 center_screen = World2Screen(center);
		int x = static_cast<int>(center_screen[0]);
		int y = static_cast<int>(center_screen[1]);
		double z = center_screen[2];

		int camera_z = BoxRaster::Float2Fixed(z, 24);
		int camera_depth = BoxRaster::Float2Fixed(depth_map[y*width_ + x], 24);

		vec3 normal = mesh->GetTriangleNormal(i);
		if (IsVisible(camera_z, camera_depth, 888) && BackFaceCulling(normal))  //688
		{
			VertexShader(primitive);
			int area = BoxRaster::Float2Fixed(ComputeTriangleArea(primitive), 6); // best choice 4

			if (area != 0)
			{
				++visible_counter;

				visible_triangle.id = i;
				visible_triangle.area = area;

				// compute cast shadow
				vec4 center_clip = shadow_camera->OpenglTransform(center);
				vec3 center_screen = shadow_camera->ViewportTransform(shadow_camera->PerspectiveDivision(center_clip));

				int light_x = static_cast<int>(center_screen[0]);
				int light_y = static_cast<int>(center_screen[1]);
				int light_z = BoxRaster::Float2Fixed(center_screen[2], 12);
				int light_depth = BoxRaster::Float2Fixed(shadow_map[light_y*width_ + light_x], 12);

				if (!IsVisible(light_z, light_depth, 20))  // 12 or  6
				{
					++shadow_counter;
					visible_triangle.cast_shadow = true;
				}
				else
				{
					visible_triangle.cast_shadow = false;
				}
			    if (mesh->IsNose(i))
					model->PushBack(visible_triangle, ModelImage::NOSE);

				else if (mesh->IsEye(i))
					model->PushBack(visible_triangle, ModelImage::EYE);

				else if (mesh->IsMouth(i))
					model->PushBack(visible_triangle, ModelImage::MOUTH);

				else if (mesh->IsRest(i))
					model->PushBack(visible_triangle, ModelImage::REST);
			}

			else
			{
				++zero_area_counter;
			}
		}
	}
	cout << "visible triangle num = " << visible_counter << endl;
	cout << "shadow triangle num = " << shadow_counter << endl;
	cout << "zero area num = " << zero_area_counter << endl;
	cout << "=================================================" << endl;

}



bool SoftwareRender::IsVisible(int z,int depth, int threshold ) const
{
	if (abs(z-depth) <= threshold)
		return true;
	else return false;
}

bool SoftwareRender::IsVisible(double z, double depth, int threshold) const
{
	if (BoxRaster::Float2Fixed(std::fabs(z-depth),24) < threshold)
		return true;
	else return false;
}



bool SoftwareRender::BackFaceCulling(const vec3& normal) const
{
	
	vec3 nw = normal;
	vec4 nwh;
	nwh.rows(0, 2) = nw;
	nwh[3] = 0; // no translation
	vec3 ne = camera_->ModelViewTransform(nwh).rows(0, 2);

	if (ne[2] <= 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}



void SoftwareRender::ComputeCenterCoordinate(Vertex& triangle_center, const MeshPtr mesh_ptr, int id) const
{
	ivec3 triangle_vertex_id = mesh_ptr->GetThreeCornerIndex(id);
	vec4 sum_coordinate;
	sum_coordinate.fill(0);
	for (int i = 0; i < 3; ++i)
	{
		sum_coordinate += mesh_ptr->GetVertexCoordinate(triangle_vertex_id[i]);
	}
	triangle_center.attribute[ATTRIBUTE_3D_COORDINATE] = sum_coordinate / 3;
}


void SoftwareRender::CalculatePositionAndArea(const Primitive& primitive, Vertex& triangle) const
{

	// compute triangle area
	vec3 px;
	vec3 py;
	for (int i = 0; i < 3; ++i)
	{
		px[i] = primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE][0];  // x
		py[i] = primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE][1];  // y
	}

	double signed_area = (px[1] - px[0]) * (py[2] - py[0]) - (px[2] - px[0]) * (py[1] - py[0]); // actually  singed_area=2 times triangle area
	double area = fabs(signed_area / 2.0f);

	// compute triangle center position
	vec4 center = (primitive.vertices[0].attribute[ATTRIBUTE_3D_COORDINATE] +
							primitive.vertices[1].attribute[ATTRIBUTE_3D_COORDINATE] +
							primitive.vertices[2].attribute[ATTRIBUTE_3D_COORDINATE]) / 3;

	vec3 screen = World2Screen(center);


	// save result
	triangle.attribute[ATTRIBUTE_2D_COORDINATE].rows(0, 2) = screen;
	triangle.attribute[ATTRIBUTE_2D_COORDINATE][3] = area;
}


double SoftwareRender::ComputeTriangleArea(const Primitive& primitive) const
{
	// compute triangle area
	vec3 px;
	vec3 py;
	for (int i = 0; i < 3; ++i)
	{
		px[i] = primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE][0];  // x
		py[i] = primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE][1];  // y
	}

	double signed_area = (px[1] - px[0]) * (py[2] - py[0]) - (px[2] - px[0]) * (py[1] - py[0]); // actually  singed_area=2 times triangle area
	return  fabs(signed_area / 2.0f);
}



void SoftwareRender::PrimitiveAssembly(Primitive& primitive, const MeshPtr mesh_ptr, int i) const
{
	ivec3 triangle_vertex_id=mesh_ptr->GetThreeCornerIndex(i);  
   for (int i=0;i<3;++i)
   {  
	   primitive.vertices[i].attribute[ATTRIBUTE_3D_COORDINATE]=mesh_ptr->GetVertexCoordinate(triangle_vertex_id[i]);
   }

}


void SoftwareRender::PrimitiveAssembly(Vertex& triangle_center, const MeshPtr mesh, int id) const
{
	ivec3 triangle_vertex_id=mesh->GetThreeCornerIndex(id);

	vec4 sum_coordinate;
	vec4 sum_color;

	sum_coordinate.fill(0);
	sum_color.fill(0);

	for (int i=0;i<3;++i)
	{
		sum_coordinate+=mesh->GetVertexCoordinate(triangle_vertex_id[i]);
		sum_color+=mesh->GetVertexColor(triangle_vertex_id[i]);
	}

	triangle_center.attribute[ATTRIBUTE_3D_COORDINATE]=sum_coordinate/3;

	triangle_center.attribute[ATTRIBUTE_COLOR] = sum_color / 3;
	vec4 normal;
	normal.rows(0, 2) = Normalize(mesh->CalcualteTriangleNormal(id));
	normal[3] = 0;

	triangle_center.attribute[ATTRIBUTE_NORMAL] =normal;
}


void SoftwareRender::VertexShader(Primitive& primitive) const
{
	for (int i=0;i<3;++i)
	{
		primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE].rows(0, 2) = World2Screen(primitive.vertices[i].attribute[ATTRIBUTE_3D_COORDINATE]);
	}
}


void SoftwareRender::VertexShader(Vertex& vertex) const
{
	vertex.attribute[ATTRIBUTE_2D_COORDINATE].rows(0, 2) = World2Screen(vertex.attribute[ATTRIBUTE_3D_COORDINATE]);

	vertex.attribute[ATTRIBUTE_3D_COORDINATE] =
		camera_->ModelViewTransform(vertex.attribute[ATTRIBUTE_3D_COORDINATE]);

	vertex.attribute[ATTRIBUTE_NORMAL] =
		camera_->ModelViewTransform(vertex.attribute[ATTRIBUTE_NORMAL]);
}


void SoftwareRender::FragmentShader(const Vertex& triangle_center, ModelPtr model) const
{
	Pixel pixel;
	illumination_->Illuminate(pixel,triangle_center);
	model->WriteColor(pixel);
}


vec3 SoftwareRender::World2Screen(const vec4& world) const
{
	vec4 clip_coordinate = camera_->OpenglTransform(world);
	return camera_->ViewportTransform(camera_->PerspectiveDivision(clip_coordinate));
}

unsigned SoftwareRender::Doubule2Unsigned(double num) const
{
	return static_cast<unsigned>(numeric_limits<unsigned>::max()*num); // opengl rounding 
}
