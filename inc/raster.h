#ifndef Raster_H_
#define Raster_H_

#include "entity.h"
#include "depth_buffer.h"


class Raster
{
	typedef arma::ivec2 ivec2;

public:
	virtual ~Raster(){}
	virtual bool SetUp(const Primitive& primitive) = 0;
	virtual void Rasterization(DepthBuffer& depth_buffer, const Primitive& primitive) = 0;
	virtual bool Rasterization(Vertex& triangle_center, DepthBuffer& depth_buffer) const = 0;



protected:
private:

};







#endif