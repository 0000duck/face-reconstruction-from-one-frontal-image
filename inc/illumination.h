#ifndef ILLUMINATION_H_
#define ILLUMINATION_H_

#include "entity.h"
#include "illumination_parameter.h"


class Illumination
{
public:
	virtual ~Illumination(){}

	virtual void Illuminate(Pixel& pixel, const Vertex& triangle_center) const=0;



protected:
private:
};








#endif