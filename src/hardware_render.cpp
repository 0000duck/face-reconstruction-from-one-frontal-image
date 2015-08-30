#include<gl/glew.h>
#include<glfw/glfw3.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/opencv.hpp>

#include "hardware_render.h"
#include "software_render.h"
#include "phong_illumination.h"
#include "opengl_common.h"
#include "gl_texture.h"

using namespace arma;
using namespace cv;

//==================================hardware render=================================================//

HardwareRender::HardwareRender(CameraPtr camera, IlluminationPtr illumination)
:camera_(camera),illumination_(illumination), scene_shader_(nullptr),shadow_shader_(nullptr), screen_shader_(nullptr)
{
	image_width_ = camera_->Width();
	image_height_ = camera_->Height();

	//screen_position_ = {
	//	{
	//		0.5f, 1.0f,
	//		1.0f, 1.0f,
	//		1.0f, 0.5f,
	//		1.0f, 0.5f,
	//		0.5f, 0.5f,
	//		0.5f, 1.0f,
	//	}
	//};

	screen_position_ = {
		{
			-1.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f, -1.0f,
			-1.0f, 1.0f,
		}
	};

	screen_st_ = {
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
		}
	};
}


HardwareRender::HardwareRender(CameraPtr camera,CameraPtr light, IlluminationPtr illumination)
:camera_(camera), light_(light), illumination_(illumination), scene_shader_(nullptr), shadow_shader_(nullptr), screen_shader_(nullptr)
{
	image_width_ = camera_->Width();
	image_height_ = camera_->Height();

	screen_position_ = {
		{
			0.5f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.5f,
			1.0f, 0.5f,
			0.5f, 0.5f,
			0.5f, 1.0f,
		}
	};


	//screen_position_ = {
	//	{
	//		-1.0f, 1.0f,
	//		1.0f, 1.0f,
	//		1.0f, -1.0f,
	//		1.0f, -1.0f,
	//		-1.0f, -1.0f,
	//		-1.0f, 1.0f,
	//	}
	//};

	screen_st_ = {
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,
		}
	};

}

void HardwareRender::Initial()
{
	SetViewport(image_width_, image_height_);	

	glClearColor(0.0f, 0.0f,0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

}


void HardwareRender::DrawScene(const MeshPtr mesh)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	scene_shader_->Bind();

	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	PassSceneUniform();

	glBindVertexArray(vao_.scene);
	glDrawElements(GL_TRIANGLES, 3*mesh->Elements().n_cols, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void HardwareRender::DrawShadow(const MeshPtr mesh)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	shadow_shader_->Bind();

	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	PassShadowUniform();

	glBindVertexArray(vao_.shadow);
	glDrawElements(GL_TRIANGLES, 3 * mesh->Elements().n_cols, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}


void HardwareRender::DrawBackgroundAndScene(const MeshPtr mesh)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	scene_shader_->Bind();

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_BACK);
	PassSceneUniform();


	glBindVertexArray(vao_.scene);
	glDrawElements(GL_TRIANGLES, 3 * mesh->Elements().n_cols, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}



void HardwareRender::DrawSceneAndShadow(const MeshPtr mesh)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	scene_shader_->Bind();

	glCullFace(GL_BACK);
	PassSceneUniform();
	PassShadowUniform();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_.tex);

	glBindVertexArray(vao_.scene);
	glDrawElements(GL_TRIANGLES, 3 * mesh->Elements().n_cols, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

}



void HardwareRender::CastShadow(const MeshPtr mesh)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_.fbo);
	shadow_shader_->Bind();

	glViewport(0, 0, ShadowResolution, ShadowResolution);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT);

	PassShadowUniform();

	glBindVertexArray(vao_.shadow);
	glDrawElements(GL_TRIANGLES, 3 * mesh->Elements().n_cols, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
	glViewport(0, 0, image_width_, image_height_);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void HardwareRender::DrawScreen()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	screen_shader_->Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_.tex);

	glBindVertexArray(vao_.screen);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void HardwareRender::DrawBackground()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	screen_shader_->Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, background_.tex);

	glBindVertexArray(vao_.screen);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}



void HardwareRender::CalculateSceneUniform(const IlluminationParameter& lamda)
{
	mat44 projection_matrix = camera_->GetProjection();
	mat44 model_view_matrix = camera_->GetView()*camera_->GetModel();

	projection_matrix_ = glm::make_mat4(projection_matrix.mem);
    model_view_matrix_= glm::make_mat4(model_view_matrix.mem);

	direct_drection_ = glm::make_vec3(lamda.direct_direction_.mem);
	direct_color_ = glm::make_vec3(lamda.direct_color_.mem);
	ambient_color_ = glm::make_vec3(lamda.ambient_color_.mem);
	contrast_ =static_cast<float>( lamda.color_contrast_);
	gain_ = glm::make_vec3(lamda.color_gain_.mem);
	offset_ = glm::make_vec3(lamda.color_offset_.mem);

}

void HardwareRender::CalculateShadowUniform()
{
	mat44 light_projection = light_->GetProjection();
	mat44 light_model_view = light_->GetView()*light_->GetModel();

	light_projection_ = glm::make_mat4(light_projection.mem);
	light_model_view_ = glm::make_mat4(light_model_view.mem);
}


void HardwareRender::LocationUniform()
{
	uniform_.camera_to_clip = glGetUniformLocation(program_.scene, "projection_matrix");
	uniform_.model_to_camera = glGetUniformLocation(program_.scene, "model_view_matrix");

	uniform_.direction = glGetUniformLocation(program_.scene, "direct_direction");
	uniform_.direct = glGetUniformLocation(program_.scene, "direct_intensity");
	uniform_.ambient = glGetUniformLocation(program_.scene, "ambient_intensity");
	uniform_.contrast = glGetUniformLocation(program_.scene, "color_contrast");
	uniform_.gain = glGetUniformLocation(program_.scene, "color_gain");
	uniform_.offset = glGetUniformLocation(program_.scene, "color_offset");


	uniform_.light_camera_to_clip = glGetUniformLocation(program_.shadow, "light_projection");
	uniform_.light_model_to_camera = glGetUniformLocation(program_.shadow, "light_model_view");

}


void HardwareRender::PassSceneUniform()
{
	glUniformMatrix4fv(uniform_.camera_to_clip, 1, GL_FALSE, glm::value_ptr(projection_matrix_));
	glUniformMatrix4fv(uniform_.model_to_camera, 1, GL_FALSE, glm::value_ptr(model_view_matrix_));


	glUniform3fv(uniform_.direction, 1, glm::value_ptr(direct_drection_));
	glUniform3fv(uniform_.direct, 1, glm::value_ptr(direct_color_));
	glUniform3fv(uniform_.ambient, 1, glm::value_ptr(ambient_color_));
	glUniform1f(uniform_.contrast, contrast_);
	glUniform3fv(uniform_.gain, 1, glm::value_ptr(gain_));
	glUniform3fv(uniform_.offset, 1, glm::value_ptr(offset_));
}

void HardwareRender::PassShadowUniform()
{
	glUniformMatrix4fv(uniform_.light_camera_to_clip, 1, GL_FALSE, glm::value_ptr(light_projection_));
	glUniformMatrix4fv(uniform_.light_model_to_camera, 1, GL_FALSE, glm::value_ptr(light_model_view_));
}


void HardwareRender::SetViewport(int window_width, int window_height)
{
	glViewport(0, 0, (GLsizei)window_width, (GLsizei)window_height);
}



void HardwareRender::CreateSceneShader()
{
	scene_shader_.reset(new Shader("scene.vertex","scene.fragment"));
	program_.scene = scene_shader_->Id();
}

void HardwareRender::CreateSceneAndShadowShader()
{
	scene_shader_.reset(new Shader("scene_and_shadow.vertex", "scene_and_shadow.fragment"));
	program_.scene = scene_shader_->Id();
}

void HardwareRender::CreateShadowShader()
{
	shadow_shader_.reset(new Shader("shadow.vertex", "shadow.fragment"));
	program_.shadow = shadow_shader_->Id();
}

void HardwareRender::CreateScreenShader()
{
	screen_shader_.reset(new Shader("screen.vertex", "screen.fragment"));
	program_.screen = screen_shader_->Id();
}


void HardwareRender::LoadSceneVbo(const MeshPtr mesh)//bind buffer
{
	glGenVertexArrays(1, &vao_.scene);
	glBindVertexArray(vao_.scene);

	/**Coordinates*/
	glGenBuffers(1, &scene_vbo_.position);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.position);
	glBufferData(GL_ARRAY_BUFFER, 
		mesh->Coordinates().n_rows*sizeof(double),
		mesh->Coordinates().mem, 
		GL_STATIC_DRAW);

	/**Colors*/
	glGenBuffers(1, &scene_vbo_.color);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.color);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->Colors().n_rows*sizeof(double),
		mesh->Colors().mem,
		GL_STATIC_DRAW);

	/**Normals*/
	glGenBuffers(1, &scene_vbo_.normal);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.normal);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->Normals().n_elem*sizeof(double),
		mesh->Normals().mem,
		GL_STATIC_DRAW);


	/**Elements*/
	glGenBuffers(1, &scene_vbo_.element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_vbo_.element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		mesh->Elements().n_elem*sizeof(int),
		mesh->Elements().mem,
		GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.position);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.color);
	glVertexAttribPointer(1, 3, GL_DOUBLE, GL_FALSE, 0, nullptr);


	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.normal);
	glVertexAttribPointer(2, 3, GL_DOUBLE, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_vbo_.element);

	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}


void HardwareRender::LoadShadowVbo(const MeshPtr mesh)//bind buffer
{
	glGenVertexArrays(1, &vao_.shadow);
	glBindVertexArray(vao_.shadow);

	/**Coordinates*/
	glGenBuffers(1, &scene_vbo_.position);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.position);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->Coordinates().n_rows*sizeof(double),
		mesh->Coordinates().mem,
		GL_STATIC_DRAW);

	/**Elements*/
	glGenBuffers(1, &scene_vbo_.element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_vbo_.element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		mesh->Elements().n_elem*sizeof(int),
		mesh->Elements().mem,
		GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_vbo_.position);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, nullptr);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_vbo_.element);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void HardwareRender::LoadScreenVbo()
{
	glGenVertexArrays(1, &vao_.screen);
	glBindVertexArray(vao_.screen);

	glGenBuffers(1,&screen_vbo_.position);
	glBindBuffer(GL_ARRAY_BUFFER,screen_vbo_.position);
	glBufferData(GL_ARRAY_BUFFER,sizeof(screen_position_),&screen_position_[0],GL_STATIC_DRAW);

	glGenBuffers(1, &screen_vbo_.st);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_.st);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_st_), &screen_st_[0], GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_.position);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_.st);
	glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 0, nullptr);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


bool HardwareRender::CreateBackground()
{
	glGenFramebuffers(1, &background_.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, background_.fbo);

	glGenTextures(1, &background_.tex);
	glBindTexture(GL_TEXTURE_2D, background_.tex);

	LoadTexture("background.jpg");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, background_.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	return true;
}


bool HardwareRender::CreateShadow()
{
	glGenFramebuffers(1, &shadow_.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_.fbo);

	glGenTextures(1, &shadow_.tex);
	glBindTexture(GL_TEXTURE_2D, shadow_.tex);

	glTexImage2D(GL_TEXTURE_2D,
						0, 
						GL_DEPTH_COMPONENT, 
						ShadowResolution, 
						ShadowResolution, 
						0, 
						GL_DEPTH_COMPONENT, 
						GL_FLOAT,
						nullptr);

	// wrapping & filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_.tex, 0);
	glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	return true;
}



void HardwareRender::LoadTexture(const string& file_name)
{
	//load texture
	cv::Mat texture = imread(file_name, CV_LOAD_IMAGE_UNCHANGED);
	//cv::namedWindow("tex");
	//cv::imshow("tex", texture);
	//cv::waitKey(0);

	if (texture.empty()) { fprintf(stderr, "error::can not open %s\n", file_name.c_str()); }
	flip(texture, texture, 0);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		texture.rows,
		texture.cols,
		0,
		GL_BGR,
		GL_UNSIGNED_BYTE,
		texture.ptr());

	//GLTexture gl_texture("background_xiao.png");

	//glTexImage2D(GL_TEXTURE_2D,
	//	0,
	//	GL_RGBA,
	//	gl_texture.Width(),
	//	gl_texture.Height(),
	//	0,
	//	GL_RGBA,
	//	GL_UNSIGNED_BYTE,
	//	gl_texture.Data());

	// wrapping & filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}




void HardwareRender::ReadDepth(vector<float>& depth_map) const
{
	glReadPixels(0,0,image_width_,image_height_,GL_DEPTH_COMPONENT,GL_FLOAT, &depth_map[0]);
}

// time consuming
void HardwareRender::WriteImage(ModelPtr model)
{
	Color color;
	int col_cv;
	for (int row = 0; row < image_width_; ++row)
	for (int col_gl = 0; col_gl < image_height_;++col_gl)
	{
		col_cv = image_height_ - col_gl - 1;
		glReadPixels(row, col_gl, 1, 1, GL_BGR, GL_UNSIGNED_BYTE, color.BGR);

		// time consuming
		model->WriteColor(row,col_cv,color);
	}
}

void HardwareRender::ClearShadowVbo()
{
	glDeleteBuffers(1, &scene_vbo_.position);
	glDeleteBuffers(1, &scene_vbo_.element);
}


void HardwareRender::ClearSceneVbo()
{
	glDeleteBuffers(1, &scene_vbo_.position);
	glDeleteBuffers(1, &scene_vbo_.color);
	glDeleteBuffers(1, &scene_vbo_.normal);
	glDeleteBuffers(1, &scene_vbo_.element);
}


void HardwareRender::ClearScreenVbo()
{
	glDeleteBuffers(1, &screen_vbo_.position);
	glDeleteBuffers(1, &screen_vbo_.st);
}


void HardwareRender::ClearVao()
{
	glDeleteVertexArrays(1, &vao_.screen);
	glDeleteVertexArrays(1, &vao_.shadow);
	glDeleteVertexArrays(1, &vao_.screen);
}


int HardwareRender::Rendering(const MeshPtr mesh, const IlluminationParameter& lamda)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(image_width_, image_height_, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Initialize glew*/
	glewExperimental = true;//Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	CreateSceneShader();
	LoadSceneVbo(mesh);

	CalculateSceneUniform(lamda);
	LocationUniform();

	Initial();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		DrawScene(mesh);
		//WriteImage(model);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	ClearSceneVbo();
	glDeleteVertexArrays(1,&vao_.scene);

	glfwTerminate();
	return 0;
}


int HardwareRender::Rendering(const MeshPtr mesh, const IlluminationParameter& lamda, std::vector<float>& depth)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(image_width_, image_height_, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Initialize glew*/
	glewExperimental = true;//Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	CreateSceneShader();
	LoadSceneVbo(mesh);

	CalculateSceneUniform(lamda);
	LocationUniform();

	Initial();

	DrawScene(mesh);

	ReadDepth(depth);

	ClearSceneVbo();
	glDeleteVertexArrays(1,&vao_.scene);

	glfwTerminate();
	return 0;
}



int HardwareRender::RenderingWithShadow(const MeshPtr mesh, const IlluminationParameter& lamda)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(image_width_, image_height_, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Initialize glew*/
	glewExperimental = true;//Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	CreateSceneAndShadowShader();
	CreateShadowShader();
	CreateScreenShader();

	CreateShadow();

	LoadSceneVbo(mesh);
	LoadShadowVbo(mesh);
	LoadScreenVbo();
	
	CalculateSceneUniform(lamda);
	CalculateShadowUniform();
	LocationUniform();

	Initial();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		CastShadow(mesh);
		DrawScreen();
		DrawSceneAndShadow(mesh);
		//WriteImage(model);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		//glfwPollEvents();
	}

		ClearSceneVbo();
		ClearScreenVbo();
		ClearVao();
		glDeleteFramebuffers(1, &shadow_.fbo);
		glDeleteTextures(1, &shadow_.tex);


	glfwTerminate();
	return 0;
}

int HardwareRender::RenderingWithShadow(const MeshPtr mesh, const IlluminationParameter& lamda, vector<float>& depth)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(image_width_, image_height_, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Initialize glew*/
	glewExperimental = true;//Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	CreateShadowShader();
	LoadShadowVbo(mesh);

	CalculateShadowUniform();
	LocationUniform();

	Initial();

	//while (!glfwWindowShouldClose(window))
	//{
	/* Render here */
	DrawShadow(mesh);
	ReadDepth(depth);
	//glfwSwapBuffers(window);
	//}

	/* Clear memory*/
	ClearShadowVbo();
	glDeleteVertexArrays(1, &vao_.shadow);

	glfwTerminate();

	return 0;
}


int HardwareRender::RenderingWithBackground(const MeshPtr mesh, ModelPtr model, const IlluminationParameter& lamda)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(image_width_, image_height_, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*Initialize glew*/
	glewExperimental = true;//Needed in core profile
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}

	CreateSceneShader();
	CreateScreenShader();
	CreateBackground();

	LoadSceneVbo(mesh);
	LoadScreenVbo();

	CalculateSceneUniform(lamda);
	LocationUniform();


	Initial();
	//while (!glfwWindowShouldClose(window))
	//{
		/* Render here */
		DrawBackground();
		DrawBackgroundAndScene(mesh);
		WriteImage(model);
	   // glfwSwapBuffers(window);
//	}

	ClearSceneVbo();
	ClearScreenVbo();
	glDeleteVertexArrays(1, &vao_.scene);
	glDeleteVertexArrays(1, &vao_.screen);
	glDeleteFramebuffers(1, &background_.fbo);
	glDeleteTextures(1, &background_.tex);
	glfwTerminate();
	return 0;
}
