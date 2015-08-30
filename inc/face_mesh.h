#ifndef FACE_MESH_H_
#define FACE_MESH_H_

#include "mesh.h"
#include "common.h"

#include <array>

//#define  ARMA_64BIT_WORD
#include <armadillo>


class FaceMesh :public Mesh
{
	using sp_mat = arma::sp_mat;
	using mat=arma::mat;
	using imat=arma::imat;
	using vec3=arma::vec3;
	using ivec3=arma::ivec3;
	using vec4=arma::vec4;


public:
	FaceMesh(bool load_feature);
	FaceMesh(const mat& shape, const mat& texture);
	FaceMesh();

	void UpdateVertexNormal() override;
	vec3 CalcualteTriangleNormal(int i) const override;
	void SaveAsPly(const std::string& file_name)  override;
	void Save(const std::string& file_name) override;
	void Update() override;

	const ivec3 GetThreeCornerIndex(int i) const override
	{
		return (triangle_list_.col(i));
	}

	const vec4 GetVertexCoordinate(int i) const override
	{
		vec4 res;
		res.rows(0, 2) = shape_.rows(3 * i, 3 * i + 2);
		res[3] = 1;
		return res;
	}

	const vec3 GetCoordinate(int i) const override
	{
		return shape_.rows(3 * i, 3 * i + 2);
	}

	const vec4 GetVertexColor(int i) const override
	{
		vec4 res;
		res.rows(0, 2) = texture_.rows(3 * i, 3 * i + 2);
		res[3] = 1;
		return res;
	}

	const vec3 GetColor(int i) const override
	{
		return texture_.rows(3 * i, 3 * i + 2);
	}

	const vec3 GetTriangleNormal(int i) const override
	{   
		return triangle_normals_.col(i);
	}

	const vec3 GetVertexNormal(int i)  const override
	{
		return vertex_normals_.col(i);
	}

    bool IsNose(int i) const override{ return segment_bin_[i]==NOSE; }
	bool IsEye(int i) const override{ return segment_bin_[i] == EYE; }
	bool IsMouth(int i) const override{ return segment_bin_[i]==MOUTH; }
	bool IsRest(int i) const override{ return segment_bin_[i]==REST; }

	void Blend() override;


	enum Segments
	{
		NOSE,
		EYE,
		MOUTH,
		REST
	};


	// needed for OpenGL index draw 
	const mat& Coordinates() const override{ return shape_; }
	const mat& Colors() const override{ return texture_; }
	const mat& Normals() const override { return vertex_normals_; }  
	const imat& Elements() const override{ return triangle_list_; }
	const int Landmarks(int i) const override{ return landmarks_[i]; }


	void ProcessFeatureFile() const;


private:
	void LoadFeature() ;


private:



	FaceMesh(const FaceMesh&);
	FaceMesh& operator=(const FaceMesh&);

	mat shape_;
	mat texture_;
	imat triangle_list_;
	
	sp_mat segment_model_;
	sp_mat segment_blend_;
	imat segment_bin_;
	

	mat vertex_normals_;
	mat triangle_normals_;
	std::vector<int> landmarks_;


};







#endif