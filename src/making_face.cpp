#include "making_face.h"
#include "face3d_model.h"

using namespace std;


void GenerateRandomFace(const std::string& random_face_name)
{
	shared_ptr<Face3dShape> shape = make_shared<Face3dShape>();
	shared_ptr<Face3dTexture> texture = make_shared<Face3dTexture>();
	Face3dModel face3d_model(shape, texture);
	shared_ptr<FaceMesh>face_mesh_ptr = face3d_model.GenerateRandomFace();
	face_mesh_ptr->SaveAsPly(random_face_name);
}

void GenerateMeanFace()
{
	shared_ptr<FaceMesh>face_mesh_ptr(new FaceMesh());
	face_mesh_ptr->SaveAsPly("mean face.ply");
}





