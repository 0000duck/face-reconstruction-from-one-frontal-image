#include "gl_texture.h"
#include "stb_image.h"



GLTexture::GLTexture(const char* const file_name)
{
	data_ = Load(file_name,width_,height_);
}

unsigned char* GLTexture::Load(const char* const file_name, int& width, int& height)
{
	int  channel;
	const int opengl_channel = 4;
	unsigned char* image_data = stbi_load(file_name, &width, &height, &channel, opengl_channel);
	if (!image_data)
	{
		fprintf(stderr, "Error: can not load %s\n", file_name);
	}

	int width_in_bytes = width * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = height / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (height - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	return image_data;
}

