#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_



class GLTexture
{
public:
	GLTexture():data_(nullptr),width_(0),height_(0){}
	GLTexture(const char* const file_name);
	~GLTexture(){ Release(); }

	void Load(const char* const file_name){ Release(); data_ = Load(file_name, width_, height_); }
	unsigned char* Load(const char* const file_name, int& width, int& height);
	unsigned char* Data(){ return data_; }
	void Release(){ if (data_) delete data_; data_ = nullptr; }

	int Width(){ return width_; }
	int Height(){ return height_; }

private:
	unsigned char* data_;
	int width_;
	int height_;
};




#endif