#ifndef MESH_H_
#define MESH_H_

#include <armadillo>
#include <vector>
#include <string>


class Mesh
{
	using vec3 = arma::vec3;
	using ivec3 = arma::ivec3;
	using vec4 = arma::vec4;
	using mat = arma::mat;
	using imat = arma::imat;

public:
	virtual ~Mesh(){}

	virtual void UpdateVertexNormal()=0;
	virtual vec3 CalcualteTriangleNormal(int i) const=0;

	virtual void SaveAsPly(const std::string& file_name)=0;
	virtual void Save(const std::string& file_name) = 0;
	virtual void Update() = 0;

	virtual const ivec3 GetThreeCornerIndex(int i) const=0;
	virtual const vec3 GetCoordinate(int i) const = 0;
	virtual const vec3 GetColor(int i) const = 0;
	virtual const vec4 GetVertexCoordinate(int i) const=0;
	virtual const vec4 GetVertexColor(int i) const=0;

	virtual const vec3 GetTriangleNormal(int i) const=0;
	virtual const vec3 GetVertexNormal(int i)  const = 0;

	virtual bool IsEye(int i) const = 0;
	virtual bool IsNose(int i) const = 0;
	virtual bool IsMouth(int i) const = 0;
	virtual bool IsRest(int i) const = 0;
	virtual void Blend() = 0;


	virtual const mat& Coordinates() const = 0;
	virtual const mat& Colors() const = 0;
	virtual const mat& Normals() const = 0;
	virtual const imat& Elements() const = 0;
	virtual const int Landmarks(int i) const  = 0;

public:

};


//inline void Normalize(arma::vec3& input ) 
//{
//	input=input/arma::norm(input,2);
//}








#endif