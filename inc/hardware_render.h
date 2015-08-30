#ifndef HARDWARE_RENDER_H_
#define HARDWARE_RENDER_H_

#include <armadillo>
#include<glm/glm.hpp>
#include <memory>
#include <array>

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "illumination.h"
#include "model_image.h"

class SoftwareRender;

class HardwareRender
{
	using mat = arma::mat;

	using mat4 = glm::mat4;
	using vec3=glm::vec3;

	using ModelPtr = std::shared_ptr<ModelImage>;
	using MeshPtr = std::shared_ptr<Mesh>;
	using ShaderPtr = std::unique_ptr<Shader>;
	using CameraPtr = std::shared_ptr<Camera>;
	using IlluminationPtr = std::shared_ptr<Illumination>;

public:
	HardwareRender(CameraPtr camera, IlluminationPtr illumination);

	HardwareRender(CameraPtr camera, CameraPtr light, IlluminationPtr illumination);


	int Rendering(const MeshPtr mesh_ptr, const IlluminationParameter& lamda);
	int Rendering(const MeshPtr mesh, const IlluminationParameter& lamda, std::vector<float>& depth);

	int RenderingWithShadow(const MeshPtr mesh, const IlluminationParameter& lamda);
	int RenderingWithShadow(const MeshPtr mesh, const IlluminationParameter& lamda, std::vector<float>& depth);
	int RenderingWithBackground(const MeshPtr mesh, ModelPtr model, const IlluminationParameter& lamda);


	void ReadDepth(std::vector<float>& depth_map) const;

	void WriteImage(ModelPtr model);
		
	void CastShadow(const MeshPtr mesh);
	void DrawShadow(const MeshPtr mesh);

	void DrawScene(const MeshPtr mesh);
	void DrawBackgroundAndScene(const MeshPtr mesh);


	void DrawSceneAndShadow(const MeshPtr mesh);
	void DrawScreen();
	void DrawBackground();

	void SetViewport(int window_width, int window_height);


private:

	void Initial();

	void CreateSceneShader();
	void CreateSceneAndShadowShader();
	void CreateShadowShader();
	void CreateScreenShader();

	bool CreateBackground();
	bool CreateShadow();


	void LoadSceneVbo(const MeshPtr mesh);
	void LoadShadowVbo(const MeshPtr mesh);
	void LoadScreenVbo();

	void LoadTexture(const std::string& file_name);

	void LocationUniform();


	void PassSceneUniform();
	void PassShadowUniform();
	void CalculateSceneUniform(const IlluminationParameter& lamda);
	void CalculateShadowUniform();


	void ClearShadowVbo();
	void ClearSceneVbo();
	void ClearScreenVbo();

	void ClearVao();


	struct Program
	{
		unsigned scene;
		unsigned shadow;
		unsigned screen;
	};

	struct Uniform  
	{
		// relative to rho
		unsigned  camera_to_clip;  // projection matrix
		unsigned  model_to_camera; // model view matrix
		unsigned  light_camera_to_clip;
		unsigned  light_model_to_camera;

		// relative to lamda
		unsigned  direction;
		unsigned  direct;
		unsigned  ambient;
		unsigned  contrast;
		unsigned  gain;
		unsigned  offset;
	};

	struct  Vao
	{
		unsigned  scene;
		unsigned screen;
		unsigned shadow;
	};

	struct SceneVbo
	{
		unsigned  position;
		unsigned  color;
		unsigned  normal;
		unsigned  element;
	};

	struct ScreenVbo
	{
		unsigned position;
		unsigned st;
	};

	struct  Buffer
	{
	    unsigned fbo;
		unsigned tex;
	};

	std::array<double,12> screen_position_;
	std::array<double,12> screen_st_;

	int image_width_;
	int image_height_;

	// in order to get uniform value
	mat4 projection_matrix_;
	mat4 model_view_matrix_;
	mat4 light_projection_;
	mat4 light_model_view_;
	
	vec3 direct_drection_;
	vec3 direct_color_;
	vec3 ambient_color_;
	float contrast_;
	vec3 gain_;
	vec3 offset_;

	Program program_;
	Vao vao_;
	Uniform uniform_;

	SceneVbo scene_vbo_;
	ScreenVbo screen_vbo_;

	Buffer background_;
	Buffer shadow_;

	ShaderPtr scene_shader_;
	ShaderPtr shadow_shader_;
	ShaderPtr screen_shader_;

	CameraPtr camera_;
	CameraPtr light_;
	IlluminationPtr illumination_;
};







#endif