#ifndef FACE3D_MODEL_H_
#define FACE3D_MODEL_H_

#include <memory>
#include "face_mesh.h"
#include "face3d_shape.h"
#include "face3d_texture.h"


class Face3dModel
{
public:	
	using mat= arma::mat ;
	using ShapePtr = std::shared_ptr<Face3dShape>;
	using TexturePtr = std::shared_ptr<Face3dTexture>;

	Face3dModel(ShapePtr shape_ptr = nullptr, TexturePtr texture_ptr = nullptr) :shape_(shape_ptr),texture_(texture_ptr){}


	std::shared_ptr<FaceMesh> Construction(const mat& alpha, const mat& beta);
	std::shared_ptr<FaceMesh> GenerateRandomFace();  

      
private:
	ShapePtr shape_;
	TexturePtr texture_;

};







#endif