#include "depth_buffer.h"

#include <iostream>

using namespace arma;
using namespace std;

bool DepthBuffer::DepthTestAndUpdata(unsigned int pixel_z, int pixel_x, int pixel_y,DepthFunction depth_function/* =LESS */)
{
	
	unsigned  buffer_z=Mat::at(pixel_x,pixel_y);

	bool zpass = false;

	switch(depth_function)
	{
	case DepthFunction::NEVER:
		zpass=false;	
		break;
	case DepthFunction::LESS:
		zpass= pixel_z<buffer_z;	// default method
		break;
	case DepthFunction::EQUAL:
		zpass= pixel_z==buffer_z;
		break;
	case DepthFunction::LEQUAL:
		zpass= pixel_z<=buffer_z;
		break;
	case DepthFunction::GREATER:
		zpass= pixel_z>buffer_z;
		break;
	case DepthFunction::NOTEQUAL:
		zpass= pixel_z!=buffer_z;
		break;
	case DepthFunction::GEQUAL:
		zpass= pixel_z>=buffer_z;
		break;
	case DepthFunction::ALWAYS:
		zpass=true;
		break;
	default:
		break;				// should never happen
	}

	if (zpass)
	{
		Mat::at(pixel_x, pixel_y) = pixel_z;
		return true;
	}
	else
	{
		return false;
	}
}


void DepthBuffer::Print() const
{
	ofstream cpu_depth;
	cpu_depth.open("cpu_depth");

	for (int i = 0; i < width_;++i)
	{
		for (int j = 0; j < height_;++j)
		{
			//if (Mat::at(i, j) != 0xffffff)
		//	cout << Mat::at(i, j) << " ";
			cpu_depth << Mat::at(j,i) << endl;
		}
	//	cout << endl;
	}

	cpu_depth.close();



}


