#include "entity.h"


using namespace std;


ostream& operator<<(ostream& out, const Primitive& primitive)
{
	out<<"Primitive Information \n";
	for (int i = 0; i < 3; ++i)
	{
		out<<"v"<<i<<" 3d coordinate: \n"<<primitive.vertices[i].attribute[ATTRIBUTE_3D_COORDINATE]<<endl;
		out<<"v"<<i<<" color: \n"<<primitive.vertices[i].attribute[ATTRIBUTE_COLOR]<<endl;
		out<<"v"<<i<<" normal: \n"<<primitive.vertices[i].attribute[ATTRIBUTE_NORMAL]<<endl;
		out<<"v"<<i<<" 2d coordinate: \n"<<primitive.vertices[i].attribute[ATTRIBUTE_2D_COORDINATE]<<endl;
	}
	return out;
}


ostream& operator<<(ostream& out, const Fragment& fragment)
{
	out<<"Fragment Information \n";
	out<<"fragment 3d coordinate: \n"<<fragment.attribute[ATTRIBUTE_3D_COORDINATE]<<endl;
	out<<"fragment color: \n"<<fragment.attribute[ATTRIBUTE_COLOR]<<endl;
	out<<"fragment normal: \n"<<fragment.attribute[ATTRIBUTE_NORMAL]<<endl;
	out<<"fragment 2d coordinate: \n"<<fragment.attribute[ATTRIBUTE_2D_COORDINATE]<<endl;

	return out;
}
