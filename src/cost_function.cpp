#include "coust_function.h"
#include "common.h"
#include "mesh.h"
#include "input_image.h"
#include "model_image.h"
#include "Parameter.h"
#include <opencv2/opencv.hpp>
#include <array>

using namespace  cv;
using namespace arma;
using namespace std;




double IntensityCost::ComputeGradient(Parameter* para, int index) const
{
	double gradient = 0;
	double part1 = 0;
	double part2 = 0;

	int random_num = random_points_.size();

	for (int t = 0; t < random_num; ++t)
	{
		int id = random_points_[t];

		ivec2 pos = para->ComputePosition(id);

		vec3 input_color = input_image_->GetColor(Point(pos[0], pos[1])); // in RGB order 
		vec3 model_color = para->ComputeColor(id);  // in RGB order

		vec3 part1 = 2 * (model_color - input_color); // R G B
		vec3 part2 = para->ComputeGradient(id, index, pos); // R G B

		gradient += dot(part1, part2);
	}

	return gradient;
}


double IntensityCost::ComputeCost(Parameter* para) const
{
	double function_value = 0;
	int random_num = random_points_.size();

	for (int t = 0; t < random_num; ++t)
	{
		int triangle_id = random_points_[t];
		ivec2 pos = para->ComputePosition(triangle_id);

		vec3 input_color = input_image_->GetColor(Point(pos[0], pos[1])); // in RGB order
		vec3 model_color = para->ComputeColor(triangle_id);  // in RGB order 
		vec3 diff = (model_color - input_color);
		function_value += dot(diff, diff);
	}
	return function_value;
}



double IntensityCost::ComputeCostOnTrian(Parameter* para) const
{
	double function_value = 0;
    int random_num = random_points_.size();
	 
	for (int t = 0; t < random_num; ++t)
	{
		int triangle_id = random_points_[t];
		ivec2 pos = para->ComputePosition(triangle_id);
		vec3 input_color = input_image_->GetColor(Point(pos[0], pos[1])); // in RGB order
		vec3 model_color = para->ComputeColor(triangle_id);  // in RGB order 
		vec3 diff = (model_color - input_color);
		function_value += dot(diff, diff);
	}
	return function_value;
}




double LandmarkCost::ComputeGradient(Parameter* para, int index) const
{
	double gradient = 0;
	int n = input_image_->landmarks_.size();

	for (int t = 0; t < n; ++t)
	{	
		gradient += para->ComputeGradientLandmark(t, index);
	}
	return gradient;
}
