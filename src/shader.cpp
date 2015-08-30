#include "shader.h"
#include<gl/glew.h>
#include<string>
#include<fstream>
#include<iostream>
#include<vector>


using namespace std;

static string TextFileRead(const char* file_name)
{
	string file_string;
	string line;
	ifstream file(file_name);
	if (file.is_open())
	while (!file.eof())
	{
		getline(file, line);
		file_string.append(line);
		file_string.append("\n");
		//file_string += line;
		//file_string+="\n";
	}
	file.close();
	return file_string;
}


Shader::Shader(const char* vertex_shader, const char* fragment_shader)
{
	Init(vertex_shader, fragment_shader);
}

Shader::~Shader()
{
	glDetachShader(program_id_,vertex_shader_id_);
	glDetachShader(program_id_, fragment_shader_id_);

	glDeleteShader(vertex_shader_id_);
	glDeleteShader(fragment_shader_id_);
	glDeleteShader(program_id_);
}

void Shader::Init(const char* vertex_shader, const char* fragment_shader)
{
	if (initialized_) return;
	initialized_ = true;

	vertex_shader_id_ = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader_id_ = glCreateShader(GL_FRAGMENT_SHADER);

	string vertex_text = TextFileRead(vertex_shader);
	string fragment_text = TextFileRead(fragment_shader);

	const char* vertex_shader_text = vertex_text.c_str();
	const char* fragment_shader_text = fragment_text.c_str();
	if (vertex_shader_text == nullptr || fragment_shader_text == nullptr)
	{
		cout << "Either vertex shader or fragment shader file not found." << endl;
		return;
	}

	GLint result = GL_FALSE;
	int information_length;

	// Compile Vertex Shader
	//printf("Compiling shader : %s\n", vertex_shader);
	glShaderSource(vertex_shader_id_, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader_id_);


	glGetShaderiv(vertex_shader_id_, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertex_shader_id_, GL_INFO_LOG_LENGTH, &information_length);
	if (information_length > 0){
		std::vector<char> VertexShaderErrorMessage(information_length + 1);
		glGetShaderInfoLog(vertex_shader_id_, information_length, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	//printf("Compiling shader : %s\n", fragment_shader);
	glShaderSource(fragment_shader_id_, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader_id_);
	
	// Check Fragment Shader
	glGetShaderiv(fragment_shader_id_, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragment_shader_id_, GL_INFO_LOG_LENGTH, &information_length);
	if (information_length > 0){
		std::vector<char> FragmentShaderErrorMessage(information_length + 1);
		glGetShaderInfoLog(fragment_shader_id_, information_length, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}


	// Link the program
	//printf("Linking program\n");
	program_id_ = glCreateProgram();
	glAttachShader(program_id_,vertex_shader_id_);
	glAttachShader(program_id_,fragment_shader_id_);
//	glBindAttribLocation(program_id_, 0, "in_Position"); // Bind a constant attribute location for positions of vertices
//	glBindAttribLocation(program_id_, 1, "in_Color"); // Bind another constant attribute location, this time for color
	glLinkProgram(program_id_);

	// Check the program
	glGetProgramiv(program_id_, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &information_length);
	if (information_length > 0){
		std::vector<char> ProgramErrorMessage(information_length + 1);
		glGetProgramInfoLog(program_id_, information_length, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}
}

void Shader::Bind()
{
	glUseProgram(program_id_);
}

void Shader::Unbind()
{
	glUseProgram(0);
}