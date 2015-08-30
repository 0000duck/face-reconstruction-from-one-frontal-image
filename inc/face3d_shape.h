#ifndef FACE3D_SHAPE_H_
#define FACE3D_SHAPE_H_


#include <armadillo>


class Face3dShape 
{
	using mat = arma::mat;
	using imat=arma::imat;
	using vec3=arma::vec3;
	using ivec3=arma::ivec3;

public:
	Face3dShape();
	void Construction(mat& shape, const mat& alpha_unit);

	const vec3 GetMean(int vertex_id) const{ return mean_shape_.rows(3*vertex_id,3*vertex_id+2); }
	const mat GetPrinciple(int vertex_id) const{ return spv_.rows(3 * vertex_id, 3 * vertex_id + 2); }
	const vec3 GetPrinciple(int vertex_id, int parameter_id) const{ return GetPrinciple(vertex_id).col(parameter_id); }
	double GetVariance(int id){ return variance_(id, 0); }
	const ivec3 GetThreeCornerIndex(int triangle_index) const{ return triangle_list_.col(triangle_index); }


private:
	mat mean_shape_;
	mat spv_;
	mat variance_;
	imat triangle_list_;
};







#endif