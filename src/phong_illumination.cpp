#include "phong_illumination.h"
#include "pinhole_camera.h"
#include "common.h"
#include "opengl_common.h"
#include "common_function.h"

using namespace arma;
using namespace std;


PhongIllumination::PhongIllumination(const IlluminationParameter& lamda) 
	:ambient_color_(lamda.ambient_color_),direct_color_(lamda.direct_color_),direct_direction_(lamda.direct_direction_),
	color_contrast_(lamda.color_contrast_),color_offset_(lamda.color_offset_),color_gain_(lamda.color_gain_)
{
}


void PhongIllumination::Illuminate(Pixel& pixel, const Vertex& triangle_center) const
{
	vec3 fragment_position=triangle_center.attribute[ATTRIBUTE_3D_COORDINATE].rows(0,2); // in camera space
	vec3 fragment_normal=triangle_center.attribute[ATTRIBUTE_NORMAL].rows(0,2); // in camera space
	vec3 fragment_color = triangle_center.attribute[ATTRIBUTE_COLOR].rows(0, 2); // no matter

	//fragment_normal.fill(0.1);
	vec3 direct_direction_normalized=Normalize(direct_direction_);
	vec3 normal_normalized=Normalize(fragment_normal);
	vec3 view_normalized=-Normalize(fragment_position);

	vec3 reflection = 2.0*dot(normal_normalized, direct_direction_normalized)*normal_normalized - direct_direction_normalized;
	
	vec3 reflection_normalized=Normalize(reflection);

	mat33 fragment_color_mat = diagmat(fragment_color);

	// phong illumination model
	vec3 ambient_component = fragment_color_mat*ambient_color_;
	vec3 diffuse_component = fragment_color_mat*direct_color_*(max(dot(normal_normalized, direct_direction_normalized), 0.0));

    vec3 specular_component = Reflection*max(pow(dot(reflection_normalized, view_normalized),Shininess),0.0)*direct_color_;
	vec3 color = ambient_component + diffuse_component +specular_component;

	pixel.coordinate[0]=static_cast<int>(triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][0]);
	pixel.coordinate[1]=IMAGE_HEIGHT-1-static_cast<int>(triangle_center.attribute[ATTRIBUTE_2D_COORDINATE][1]);

	pixel.color[0] = static_cast<unsigned char>(std::min((color[0] + 0.5), 255.0)); // R
	pixel.color[1] = static_cast<unsigned char>(std::min((color[1] + 0.5), 255.0)); // G
	pixel.color[2] = static_cast<unsigned char>(std::min((color[2] + 0.5), 255.0)); // B


}




