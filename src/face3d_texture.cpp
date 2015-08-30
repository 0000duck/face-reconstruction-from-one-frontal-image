#include "face3d_texture.h"


using namespace arma;


Face3dTexture::Face3dTexture()
{
	mean_texture_.quiet_load("mean_texture",arma::arma_binary);
	tpv_.quiet_load("tpv",arma::arma_binary);
	variance_.quiet_load("texture_variance", arma::arma_binary);
}

void Face3dTexture::Construction(mat& texture, const mat& beta_unit)
{
	mat ones = mat(1, beta_unit.n_cols, fill::ones);
	texture=mean_texture_*ones+tpv_*beta_unit; 

}






