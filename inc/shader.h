#ifndef SHADER_H_
#define SHADER_H_


class Shader
{
public:

	explicit Shader(const char* vertex_shader = "vertex.shader",
		const char* fragment_shader = "fragment.shader");
	~Shader();


	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void Bind();
	void Unbind();
	unsigned int Id() const { return program_id_; }

protected:
private:

	void Init(const char* vertex_shader, const char* fragment_shader);

	unsigned int program_id_;
	unsigned int vertex_shader_id_ = 0;
	unsigned int fragment_shader_id_ = 0;
	bool initialized_ = false;
};





#endif