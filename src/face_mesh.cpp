#include "face_mesh.h"
#include "model_image.h"
#include "common.h"

#include <fstream>
#include <random>

using namespace arma;
using namespace std;


#include "common_function.h"

FaceMesh::FaceMesh(bool load_feature) :landmarks_{ {// ModelImage::LEFT_EYEBROW_RIGHT_CORNER,
																				ModelImage::LEFT_EYE_LEFT_CORNER,
																				//ModelImage::RIGHT_EYE_LEFT_CORNER,
																				ModelImage::RIGHT_EYE_RIGHT_CORNER,

																				//ModelImage::LEFT_EAR_LOBE,
																				//ModelImage::LEFT_EAR_UPER,
																				//ModelImage::LEFT_EAR_MIDDLE,
																				//ModelImage::LEFT_EAR_LOWER,


																				//ModelImage::NOSE_LEFT_CORNER,
																				ModelImage::NOSE_CENTER,
																				//ModelImage::NOSE_RIGHT_CORNER,

																				//ModelImage::RIGHT_EAR_LOBE,
																			//	ModelImage::RIGHT_EAR_UPER,
																				//ModelImage::RIGHT_EAR_MIDDLE,
																			//	ModelImage::RIGHT_EAR_LOWER,

																				ModelImage::LEFT_OUTLINE,

																				//ModelImage::UPER_LIP_CENTER,
																				ModelImage::MOUTH_LEFT_CORNER,
																				ModelImage::MOUTH_RIGHT_CORNER, 

																				ModelImage::RIGHT_OUTLINE,

} }
{
	shape_.quiet_load("mean_shape",arma::arma_binary);
	texture_.quiet_load("mean_texture",arma::arma_binary);
	triangle_list_.quiet_load("mesh_topology",arma::arma_binary);

	//segment_model_.quiet_load("segment_model",arma::arma_binary);
	//segment_blend_.quiet_load("segment_blend", arma::arma_binary);
	segment_bin_.quiet_load("segment_bin", arma::arma_binary);

	if (load_feature) LoadFeature();

}



FaceMesh::FaceMesh(const mat& shape, const mat& texture) :shape_(shape), 
																						texture_(texture),
																						landmarks_{ {// ModelImage::LEFT_EYEBROW_RIGHT_CORNER,
																											ModelImage::LEFT_EYE_LEFT_CORNER,
																											//ModelImage::RIGHT_EYE_LEFT_CORNER,
																											ModelImage::RIGHT_EYE_RIGHT_CORNER,

																											//ModelImage::LEFT_EAR_UPER,
																											//ModelImage::LEFT_EAR_MIDDLE,
																										//	ModelImage::LEFT_EAR_LOWER,

																											//ModelImage::NOSE_LEFT_CORNER,
																											ModelImage::NOSE_CENTER,
																											//ModelImage::NOSE_RIGHT_CORNER,

																										//	ModelImage::RIGHT_EAR_UPER,
																											//ModelImage::RIGHT_EAR_MIDDLE,
																										//    ModelImage::RIGHT_EAR_LOWER,

																										    ModelImage::LEFT_OUTLINE,

																											//ModelImage::UPER_LIP_CENTER,
																											ModelImage::MOUTH_LEFT_CORNER,	
																											ModelImage::MOUTH_RIGHT_CORNER,

																											ModelImage::RIGHT_OUTLINE,
} }

{
	triangle_list_.quiet_load("mesh_topology",arma::arma_binary);

	//segment_model_.quiet_load("segment_model", arma::arma_binary);
	//segment_blend_.quiet_load("segment_blend", arma::arma_binary);
	segment_bin_.quiet_load("segment_bin", arma::arma_binary);
}


FaceMesh::FaceMesh()
{
	triangle_list_.quiet_load("mesh_topology", arma::arma_binary);
	shape_.quiet_load("shape.txt", arma::raw_ascii);
	texture_.quiet_load("texture.txt", arma::raw_ascii);
}


void FaceMesh::LoadFeature() 
{
	ifstream feature_file;
	feature_file.open("feature.txt");
	vector<int> all_feature;
	if (!feature_file.is_open()) cout << "can't open face feature file !" << endl;
	int feature;
	while (feature_file >> feature)
	{
		landmarks_.push_back(feature);
	}
}



void FaceMesh::ProcessFeatureFile() const
{
	vector<int> features;
	ifstream feature_points;
	feature_points.open("original feature file.txt");
	if (!feature_points.is_open()) { cout << "can't open file! " << "\n"; }

	vector<string> tokens;
	vector<string> words;

	ofstream word_file;
	word_file.open("feature.txt");
	string line;
	while (getline(feature_points, line))
	{
		if (line == "") continue;
		istringstream line_stream(line);
		string word;
		copy(istream_iterator<string>(line_stream), istream_iterator<string>(), back_inserter<vector<string>>(tokens));
		word_file << tokens[0] << endl;
		words.push_back(tokens[0]);
		tokens.clear();
	}
	word_file.close();
}



void FaceMesh::Blend()
{
	mat shape(SegmentsNum*VertexNum, 3);
	mat texture(SegmentsNum*VertexNum, 3);

	for (int i = 0; i < SegmentsNum;++i)
	{
		shape.rows(i*VertexNum, (i + 1)*VertexNum - 1) = reshape(shape_.cols(i, i),3,VertexNum).t();
		texture.rows(i*VertexNum, (i + 1)*VertexNum - 1) = reshape(texture_.cols(i, i), 3, VertexNum).t();
	}
	
	shape.quiet_save("shape_para",raw_ascii);
	texture.quiet_save("texture_para", raw_ascii);

	//shape_ = solve(mat(segment_model_), segment_blend_*shape);
	//texture_ = solve(mat(segment_model_), segment_blend_*texture);
}



void FaceMesh::UpdateVertexNormal()
{

	ivec3 triangle_vertex_id;
	int triangle_num =TriangleNum;
	int vertex_num = VertexNum;

	//calculate triangle normal
	triangle_normals_.set_size(3,triangle_num);
	for (int i = 0; i<triangle_num; ++i)
	{
		triangle_normals_.col(i) = CalcualteTriangleNormal(i);
	}

	//initial vertex normal
	vertex_normals_.set_size(3,vertex_num);
	vertex_normals_.fill(0);

	// sum triangle normal around vertex  
	for (int i = 0; i<triangle_num; ++i)
	{
		triangle_vertex_id = GetThreeCornerIndex(i);// triangle three corner index
		vertex_normals_.col(triangle_vertex_id[0]) += triangle_normals_.col(i);
		vertex_normals_.col(triangle_vertex_id[1]) += triangle_normals_.col(i);
		vertex_normals_.col(triangle_vertex_id[2]) += triangle_normals_.col(i);
	}

	//normalize vertex normal
	for (int i = 0; i < vertex_num; ++i)
	{
		vertex_normals_.col(i) = Normalize(vertex_normals_.col(i));
	}
}

vec3 FaceMesh::CalcualteTriangleNormal(int i) const
{
	ivec3 triangle_vertex_id=GetThreeCornerIndex(i);
	vec3 vertex_a=shape_.rows(3*triangle_vertex_id[0],3*triangle_vertex_id[0]+2);
	vec3 vertex_b=shape_.rows(3*triangle_vertex_id[1],3*triangle_vertex_id[1]+2);
	vec3 vertex_c=shape_.rows(3*triangle_vertex_id[2],3*triangle_vertex_id[2]+2);
	vec3 face_normal=cross((vertex_b-vertex_a),(vertex_c-vertex_a));
	return face_normal;
}

void FaceMesh::SaveAsPly(const std::string& file_name) 
{
	std::ofstream out_file;
	out_file.open(file_name);

	size_t number_vertices=shape_.n_rows/3;
	size_t number_faces=triangle_list_.n_cols;
	size_t number_per_face=triangle_list_.n_rows;

	out_file.clear();

	//write header
	out_file<<"ply"<<std::endl;
	out_file<<"format ascii 1.0"<<std::endl;
	out_file<<"comment author:tpys"<<std::endl;
	out_file<<"element vertex"<<" "<<number_vertices<<std::endl;
	out_file<<"property float"<<" "<<"x"<<std::endl;
	out_file<<"property float"<<" "<<"y"<<std::endl;
	out_file<<"property float"<<" "<<"z"<<std::endl;
	out_file<<"property uchar"<<" "<<"red"<<std::endl;
	out_file<<"property uchar"<<" "<<"green"<<std::endl;
	out_file<<"property uchar"<<" "<<"blue"<<std::endl;
	out_file<<"element face"<<" "<<number_faces<<std::endl;
	out_file<<"property list uchar int vertex_indices"<<std::endl;
	out_file<<"end_header"<<std::endl;

	//write shape and color information
	for (size_t i=0;i<number_vertices;++i)
	{
		out_file<<shape_(3*i,0)<<" "<<shape_(3*i+1,0)<<" "<<shape_(3*i+2,0)<<std::endl;
		out_file<<static_cast<unsigned int>(texture_(3*i,0))<<" "
			<<static_cast<unsigned int>(texture_(3*i+1,0))<<" "
			<<static_cast<unsigned int>(texture_(3*i+2,0))<<std::endl;
	}

	//write triangle list 
	for (size_t j=0;j<number_faces;++j)
	{
		//y x z
		out_file<<number_per_face<<" "<<triangle_list_(0,j)<<" "<<triangle_list_(1,j)<<" "<<triangle_list_(2,j)<<std::endl;
	}

	out_file.close();
}



void FaceMesh::Save(const std::string& file_name)
{
	std::ofstream out_file;
	out_file.open(file_name);

	size_t number_vertices = shape_.n_rows;
	size_t number_faces = triangle_list_.n_cols;
	size_t number_per_face = triangle_list_.n_rows;

	out_file.clear();

	//write header
	out_file << "ply" << std::endl;
	out_file << "format ascii 1.0" << std::endl;
	out_file << "comment author:tpys" << std::endl;
	out_file << "element vertex" << " " << number_vertices << std::endl;
	out_file << "property float" << " " << "x" << std::endl;
	out_file << "property float" << " " << "y" << std::endl;
	out_file << "property float" << " " << "z" << std::endl;
	out_file << "property uchar" << " " << "red" << std::endl;
	out_file << "property uchar" << " " << "green" << std::endl;
	out_file << "property uchar" << " " << "blue" << std::endl;
	out_file << "element face" << " " << number_faces << std::endl;
	out_file << "property list uchar int vertex_indices" << std::endl;
	out_file << "end_header" << std::endl;

	//write shape and color information
	for (size_t i = 0; i < number_vertices; ++i)
	{
		out_file << shape_(i, 0) << " " << shape_( i, 1) << " " << shape_(i, 2) << std::endl;
		out_file << static_cast<unsigned int>(texture_(i, 0)) << " "
			<< static_cast<unsigned int>(texture_(i, 1)) << " "
			<< static_cast<unsigned int>(texture_(i, 2)) << std::endl;
	}

	//write triangle list 
	for (size_t j = 0; j < number_faces; ++j)
	{
		//y x z
		out_file << number_per_face << " " << triangle_list_(0, j) << " " << triangle_list_(1, j) << " " << triangle_list_(2, j) << std::endl;
	}

	out_file.close();
}


void FaceMesh::Update()
{
	shape_.quiet_load("shape.txt", raw_ascii);
	texture_.quiet_load("texture.txt", raw_ascii);
}