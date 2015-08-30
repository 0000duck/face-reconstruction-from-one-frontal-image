#include "face3d_shape.h"

using namespace arma;



Face3dShape::Face3dShape()
{
	mean_shape_.quiet_load("mean_shape",arma::arma_binary);
	spv_.quiet_load("spv",arma::arma_binary);
	variance_.quiet_load("shape_variance",arma::arma_binary);
	triangle_list_.quiet_load("mesh_topology", arma::arma_binary);
}

void Face3dShape::Construction(mat& shape, const mat& alpha_unit) 
{	
	mat ones = mat(1, alpha_unit.n_cols, fill::ones);
	shape=mean_shape_*ones+spv_*alpha_unit;
}


//const vec3 Face3dShape::GetPrinciple(int vertex_id, int parameter_id) const
//{
//	vec3 res;
//	res[0] = spv_(3 * vertex_id, parameter_id);
//	res[1] = spv_(3 * vertex_id + 1, parameter_id);
//	res[2] = spv_(3 * vertex_id + 2, parameter_id);
//	return res;
//	//spv_.submat(3*vertex_id,parameter_id,3*vertex_id+2,parameter_id);
//}







