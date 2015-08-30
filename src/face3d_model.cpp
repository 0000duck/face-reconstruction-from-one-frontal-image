#include <fstream>
#include <random>
#include "common.h"
#include "face3d_model.h"

using namespace std;
using namespace arma;



shared_ptr<FaceMesh> Face3dModel::Construction(const mat& alpha, const mat& beta)
{
	mat shape;
	mat texture;
	shape_->Construction(shape,alpha);
	texture_->Construction(texture,beta);
    return shared_ptr<FaceMesh>(new FaceMesh(shape,texture));	
}


shared_ptr<FaceMesh> Face3dModel::GenerateRandomFace()
{

	mat alpha = randn<mat>(PrincipalNum);
	mat beta = randn<mat>(PrincipalNum);

	return Construction(alpha,beta);
}












