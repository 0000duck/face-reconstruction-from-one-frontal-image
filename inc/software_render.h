#ifndef SOFTWARE_RENDER_H_
#define SOFTWARE_RENDER_H_

#include <memory>

#include "model_image.h"
#include "mesh.h"
#include "depth_buffer.h"
#include "Raster.h"
#include "camera.h"
#include "illumination.h"
#include "common.h"


class CameraParameter;
class IlluminationParameter;


class SoftwareRender
{
	using vec3 = arma::vec3;
	using vec4 = arma::vec4;
	using ivec3 = arma::ivec3;
	using mat = arma::mat;
	using ivec2 = arma::ivec2;
	using ModelPtr = std::shared_ptr<ModelImage>;
	using MeshPtr = std::shared_ptr<Mesh>;
	using DepthPtr = std::shared_ptr<DepthBuffer>;
	using RasterPtr = std::shared_ptr<Raster>;
	using CameraPtr = std::shared_ptr<Camera>;
	using IlluminationPtr = std::shared_ptr<Illumination>;


public:
	SoftwareRender(CameraPtr camera, IlluminationPtr illumination, DepthPtr depth, RasterPtr raster);


	void Rasterization(MeshPtr mesh);

	void Rending(MeshPtr mesh, ModelPtr model);

	void TwoPassZbuffer(CameraPtr shadow_camera,
		MeshPtr mesh,
		ModelPtr model,
		std::vector<float>& depth_map,
		std::vector<float>& shadow_map);

	void TwoPassZbufferSegment(CameraPtr shadow_camera,
		MeshPtr mesh,
		ModelPtr model,
		std::vector<float>& depth_map,
		std::vector<float>& shadow_map);


	void DrawLandmark(MeshPtr mesh, ModelPtr model);


	bool BackFaceCulling(const vec3& normal) const;



private:


	SoftwareRender(const SoftwareRender&);
	SoftwareRender& operator=(const SoftwareRender&);
	
	void PrimitiveAssembly(Primitive& primitive, const MeshPtr mesh, int i) const;
	void PrimitiveAssembly(Vertex& vertex, const MeshPtr mesh, int i) const;

	void VertexShader(Primitive& primitive) const;
	void VertexShader(Vertex& triangle_center) const;

	void FragmentShader(const Vertex& triangle_center,ModelPtr model) const;

	void CalculatePositionAndArea(const Primitive& primitive,  Vertex& center) const;
	void ComputeCenterCoordinate(Vertex& triangle_center, const MeshPtr mesh, int id) const;
	double ComputeTriangleArea(const Primitive& primitive) const;


	vec3 World2Screen(const vec4& world) const;
	unsigned Doubule2Unsigned(double num) const;

	bool IsVisible(int real_z, int map_z, int threshold) const;
	bool IsVisible(double real_z, double map_z, int threshold) const;

		
	DepthPtr depth_; 
	RasterPtr raster_;
    CameraPtr camera_;
	IlluminationPtr illumination_;
	int width_;
	int height_;

};







#endif