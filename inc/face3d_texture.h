#ifndef FACE3D_TEXTURE_H_
#define FACE3D_TEXTURE_H_

#include <armadillo>


class Face3dTexture
{
	using mat= arma::mat ;
	using vec3=arma::vec3;

public:
	Face3dTexture();
	void Construction(mat& texture, const mat& beta_unit);

	const vec3 GetMean(int vertex_id) const{ return mean_texture_.rows(3 * vertex_id, 3 * vertex_id + 2); }
	const mat GetPrinciple(int vertex_id) const{ return tpv_.rows(3 * vertex_id, 3 * vertex_id + 2); }
	const vec3 GetPrinciple(int vertex_id, int parameter_id) const{ return GetPrinciple(vertex_id).col(parameter_id); }

	double GetVariance(int id){ return variance_(id, 0); }

private:
	mat mean_texture_;
	mat tpv_;
	mat variance_;

};






#endif