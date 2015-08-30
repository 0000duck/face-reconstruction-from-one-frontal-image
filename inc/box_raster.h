#ifndef BOX_Raster_H_
#define BOX_Raster_H_


#include "Raster.h"
#include "edge_function.h"
#include "entity.h"
#include "depth_buffer.h"
#include <cassert>

#include <bitset>

class BoxRaster:public Raster
{

	using ivec3= arma::ivec3 ;
	using ivec2= arma::ivec2 ;
	using vec2= arma::vec2 ;
	using vec4= arma::vec4 ;
	using Fragments= std::vector<Fragment>;


public:
	BoxRaster();

	bool SetUp(const Primitive& primitive)  override;
	void Rasterization(DepthBuffer& depth_buffer,const Primitive& primitive)  override;
	bool Rasterization(Vertex& triangle_center, DepthBuffer& depth_buffer) const override;


	static 
	int Float2Fixed(double num, int fractional_bits = FRACTIONAL_BIT) 
	{
			assert(fractional_bits >= 0);
			return static_cast<int>((1 << fractional_bits)*num);	
	}

	static
	int Float2Fixed(float num, int fractional_bits = FRACTIONAL_BIT)
	{
			assert(fractional_bits >= 0);
			return static_cast<int>((1 << fractional_bits)*num);
	}


protected:
private:
	struct BoundingBox 
	{
		int min_x;
		int max_x;
		int min_y;
		int max_y;
	};

	EdgeFunction edge_functions_[3];  
	BoundingBox bbx_;

	double triangle_area_;
	ivec3 signed_distances_; 


	// assist function
	static const int FRACTIONAL_BIT=8;
	static const int RESOLUTION;

	
	void ComputeBoudingBox(const ivec3& x, const ivec3& y) ;
	bool IsInsideTriangle(int xi, int yi);
	bool PerFragment(DepthBuffer& depth_buffer,int xi, int yi,const Primitive& primitive) const;
	bool PerFragment(Vertex& triangle_center,DepthBuffer& depth_buffer) const;
	inline vec2 ComputeBarycentricCoordinate() const;
	vec2 ComputePerspectiveBarycentricCoordinate(const Primitive& primitive) const;
	inline unsigned int InterpolateZ(const vec2& uv,const Primitive& primitive) const;
	inline unsigned int InterpolateZ(const Vertex& triangle_center) const;
	inline vec4 InterpolateVertexAttribute(int id, const vec2& uv, const Primitive& primitive) const;
		
};


template<typename Type>
inline Type MIN2(Type a, Type b) 
{
	return std::min<Type>(a,b);
}

template<typename Type>
inline Type MAX2(Type a, Type b)
{
	return std::max<Type>(a,b);
}

template<typename Type>
inline Type MIN3(Type a, Type b, Type c)
{
	return MIN2(MIN2(a,b),c);
}

template<typename Type>
inline Type MAX3(Type a, Type b, Type c)
{
	return MAX2(MAX2(a,b),c);
}


inline int Power2(int times)
{
	if(times==0) return 1;
	return 2*Power2(times-1);
}





#endif