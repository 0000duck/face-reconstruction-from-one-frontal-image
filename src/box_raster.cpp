#include "box_Raster.h"
using namespace arma;


const int BoxRaster::RESOLUTION=Power2(FRACTIONAL_BIT);

BoxRaster::BoxRaster()
{
}

bool BoxRaster::SetUp(const Primitive& primitive) 
{
	ivec3 x,y;
	// x
	x[0]=Float2Fixed(primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][0],FRACTIONAL_BIT);
	x[1]=Float2Fixed(primitive.vertices[1].attribute[ATTRIBUTE_2D_COORDINATE][0],FRACTIONAL_BIT);
	x[2]=Float2Fixed(primitive.vertices[2].attribute[ATTRIBUTE_2D_COORDINATE][0],FRACTIONAL_BIT);

	// y
	y[0]=Float2Fixed(primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][1],FRACTIONAL_BIT);
	y[1]=Float2Fixed(primitive.vertices[1].attribute[ATTRIBUTE_2D_COORDINATE][1],FRACTIONAL_BIT);
	y[2]=Float2Fixed(primitive.vertices[2].attribute[ATTRIBUTE_2D_COORDINATE][1],FRACTIONAL_BIT);

	ComputeBoudingBox(x,y);

	int signed_area = (x[1]-x[0]) * (y[2]-y[0]) - (x[2]-x[0]) * (y[1]-y[0]); // actually  singed_area=2 times triangle area
	triangle_area_ = fabs(signed_area / 2.0f);

	if(signed_area==0) return false;

	//---- setup edge functions: note the order; an edge function with number n is derived from vertices_ not including n
	edge_functions_[2].SetUp(signed_area,x[0],y[0],x[1],y[1]);
	edge_functions_[0].SetUp(signed_area,x[1],y[1],x[2],y[2]);
	edge_functions_[1].SetUp(signed_area,x[2],y[2],x[0],y[0]);

	return true;
}


void BoxRaster::ComputeBoudingBox(const ivec3& x, const ivec3& y)  
{
	// compute integer bounding box
	bbx_.min_x=MIN3<int>(x[0],x[1],x[2]);
	bbx_.min_y=MIN3<int>(y[0],y[1],y[2]);
	bbx_.min_x=(bbx_.min_x>>FRACTIONAL_BIT);  // rounding down 
	bbx_.min_y=(bbx_.min_y>>FRACTIONAL_BIT);
	bbx_.min_x=MAX2<int>(bbx_.min_x,0);
	bbx_.min_y=MAX2<int>(bbx_.min_y,0);

	bbx_.max_x=MAX3<int>(x[0],x[1],x[2]);
	bbx_.max_y=MAX3<int>(y[0],y[1],y[2]);

	//bbx_.max_x=((bbx_.max_x>>FRACTIONAL_BIT)+1); // rounding up 
	//bbx_.max_y=((bbx_.max_y>>FRACTIONAL_BIT)+1);

	bbx_.max_x=((bbx_.max_x+RESOLUTION-1)>>FRACTIONAL_BIT); // rounding up 
	bbx_.max_y=((bbx_.max_y+RESOLUTION-1)>>FRACTIONAL_BIT);

	bbx_.max_x=MIN2<int>(bbx_.max_x,IMAGE_WIDTH);
	bbx_.max_y=MIN2<int>(bbx_.max_y,IMAGE_HEIGHT);
}


void BoxRaster::Rasterization(DepthBuffer& depth_buffer,const Primitive& primitive) 
{
	for(int yi=bbx_.min_y; yi<=bbx_.max_y; ++yi)
	{
		for(int xi=bbx_.min_x; xi<bbx_.max_x; ++xi)	
		{		
			if(IsInsideTriangle(xi,yi))
				PerFragment(depth_buffer, xi, yi, primitive);
		}
	}
}

bool BoxRaster::Rasterization(Vertex& triangle_center, DepthBuffer& depth_buffer) const
{
	return PerFragment(triangle_center,depth_buffer);
}


bool BoxRaster::IsInsideTriangle(int xi, int yi) 
{
	// pixel center
	xi=xi*RESOLUTION+RESOLUTION/2;
	yi=yi*RESOLUTION+RESOLUTION/2;

	return edge_functions_[0].IsInside(xi,yi,signed_distances_[0])&&
		edge_functions_[1].IsInside(xi,yi,signed_distances_[1])&&
		edge_functions_[2].IsInside(xi,yi,signed_distances_[2]);

}

/**OpenGL zbuffer is used for determining vertex visible => Update z buffer*/
bool BoxRaster::PerFragment(DepthBuffer& depth_buffer,
								int xi, int yi,
								const Primitive& primitive) const
{

	vec2 uv=ComputeBarycentricCoordinate();// since depth (z/w) is interpolated linearly, we can use Barycentric coordinate (no perspective)
	int per_fragment_buffer=InterpolateZ(uv,primitive);
	if(!depth_buffer.DepthTestAndUpdata(per_fragment_buffer,xi,yi))
		return false;
	return true;
}


bool BoxRaster::PerFragment(Vertex& triangle_center,
								DepthBuffer& depth_buffer) const
{

	unsigned per_fragment_buffer=InterpolateZ(triangle_center);

	int xi=static_cast<int>(triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][0]);  // opengl rouding down 
	//int yi=static_cast<int>(triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][1]);
	int yi = static_cast<int>(std::max(triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][1], 0.0));
	return depth_buffer.DepthTestAndUpdata(per_fragment_buffer, xi, yi);
}


vec2 BoxRaster::ComputeBarycentricCoordinate() const
{
	vec2 res;
	res[0]= signed_distances_[1]/(2*triangle_area_); // u
	res[1]= signed_distances_[2]/(2*triangle_area_); // v  
	return res;
}


unsigned int BoxRaster::InterpolateZ(const vec2& uv,const Primitive& primitive) const
{
	double z=primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][2]+
		uv[0]*(primitive.vertices[1].attribute[ATTRIBUTE_2D_COORDINATE][2]-primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][2])+
		uv[1]*(primitive.vertices[2].attribute[ATTRIBUTE_2D_COORDINATE][2]-primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][2]);

	// convert z 2 24 bit fixed z, depth buffer is 24 bit 
	int fixed_z=Float2Fixed(z,24);
	return static_cast<unsigned int>(MIN2(fixed_z,0xffffff));
}

unsigned int BoxRaster::InterpolateZ(const Vertex& triangle_center) const
{
	double z=triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][2];
	// convert z 2 24 bit fixed z, depth buffer is 24 bit 
	unsigned fixed_z=Float2Fixed(z,24);
	//return std::min<unsigned>(fixed_z,0xffffff);
	return fixed_z;
}


vec2 BoxRaster::ComputePerspectiveBarycentricCoordinate(const Primitive& primitive) const
{
	vec2 res;
	double f[3];
	f[0]=signed_distances_[0]/primitive.vertices[0].attribute[ATTRIBUTE_2D_COORDINATE][3]; // f0=e0/hw0
	f[1]=signed_distances_[1]/primitive.vertices[1].attribute[ATTRIBUTE_2D_COORDINATE][3]; // f1=e1/hw1
	f[2]=signed_distances_[2]/primitive.vertices[2].attribute[ATTRIBUTE_2D_COORDINATE][3]; // f2=e2/hw2

	const double sum=f[0] + f[1] + f[2];

	res[0]=f[1]/sum; // u0=f1/(f0+f1+f2);
	res[1]=f[2]/sum; // v0=f2/(f0+f1+f2)

	return res;
}


vec4 BoxRaster::InterpolateVertexAttribute(int id, const vec2& uv, const Primitive& primitive) const
{
	return primitive.vertices[0].attribute[id]+uv[0]*(primitive.vertices[1].attribute[id]-primitive.vertices[0].attribute[id])+
		uv[1]*(primitive.vertices[2].attribute[id]-primitive.vertices[0].attribute[id]);
}








