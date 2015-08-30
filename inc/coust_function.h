#ifndef COST_FUNCTION_H_
#define COST_FUNCTION_H_

class Parameter;
class InputImage;
class ModelImage;
class Mesh;

#include <memory>
#include <vector>
#include <armadillo>


class CostFunction
{
public:

	using InputPtr = std::shared_ptr<InputImage>;
	using ModelPtr = std::shared_ptr<ModelImage>;
	using MeshPtr = std::shared_ptr<Mesh>;

	virtual~CostFunction(){}
	virtual double ComputeGradient(Parameter* para_ptr, int index) const = 0;



protected:
private:

};

class IntensityCost :public CostFunction
{
public:
	explicit IntensityCost(const std::vector<int>& random_points, InputPtr input_image = nullptr)
		:random_points_(random_points), input_image_(input_image)
	{
	}


	double ComputeGradient(Parameter* para_ptr, int index) const override;
	double ComputeCost(Parameter* para) const;
	double ComputeCostOnTrian(Parameter* para) const;



private:


	InputPtr input_image_;
	std::vector<int> random_points_;

};


class LandmarkCost :public CostFunction
{
public:

	using ivec2=arma::ivec2;
	LandmarkCost(InputPtr input_image = nullptr) :input_image_(input_image){}
	double ComputeGradient(Parameter* para, int index) const override;


private:
	InputPtr input_image_;


};





#endif