#ifndef DEPTH_BUFFER_H_
#define DEPTH_BUFFER_H_


#include "armadillo"
#include "common.h"




class DepthBuffer:public arma::Mat<unsigned int>
{
public:
	using mat = arma::mat;

	DepthBuffer(int width = IMAGE_WIDTH, int height = IMAGE_HEIGHT) :Mat<unsigned int>(width, height), width_(width), height_(height)
	{	
		//fill(0xffffff);
		fill(0x1000000);
	}



	enum class DepthFunction
	{
		NEVER,
		LESS,
		EQUAL,
		LEQUAL,
		GREATER,
		NOTEQUAL,
		GEQUAL,
		ALWAYS,
	};

	bool DepthTestAndUpdata(unsigned int pixel_z, int pixel_x, int pixel_y,DepthFunction depth_function=DepthFunction::LESS) ;

	unsigned At(int x, int y){ return Mat::at(x, y); }

	void Print() const;

private:
	int  width_;
	int height_;


};











#endif