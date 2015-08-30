#ifndef OPENGL_ENTITY_H_
#define OPENGL_ENTITY_H_

#include "armadillo"
#include <iostream>


enum VertexAttribute
{
	///////////////////////// for shading //////////////////////////////////////////////////
	ATTRIBUTE_3D_COORDINATE,	//  x y z 1 in model space
	ATTRIBUTE_COLOR,	           // x y z 1 no matter what space
	ATTRIBUTE_NORMAL,			// x y z 1 in model space

	////////////////////////// for rasterization/////////////////////////////////////////////////
	ATTRIBUTE_2D_COORDINATE, // x y z( screen space) w (clip space)
	ATTRIBUTE_SIZE
};


struct Vertex
{
	arma::vec4 attribute[ATTRIBUTE_SIZE];
};

struct  Primitive
{
	Vertex vertices[3];
};

struct Fragment
{
	arma::vec4 attribute[ATTRIBUTE_SIZE];
};

struct Pixel
{
	unsigned char color[3];
	int coordinate[2];
};

struct Color
{
	unsigned char BGR[3];
};



std::ostream& operator<<(std::ostream& out, const Primitive& primitive);
std::ostream& operator<<(std::ostream& out, const Fragment& fragment);



#endif