#ifndef COMMON_H_
#define COMMON_H_

#include <string>

enum  ImageSize
{
	IMAGE_WIDTH = 512,// unit px
	IMAGE_HEIGHT = 512
};


const int VertexNum = 53490;
const int TriangleNum = 106466;

const int SegmentsNum = 4;
const int FirstPrincipalNum = 10;
const int PrincipalNum=99;
const int RhoNum = 7;
const int LamdaNum = 15;


const int Iterations = 1000;
const int AlignmentIterations = 500;
const int Circles = 5;


const int LandmarkNum = 7;
const int GradientRandomNum = 40;
const int HessianRandomNum = 300;
const double RandomRatio = static_cast<double>(GradientRandomNum) / HessianRandomNum;


const int ResolutionThreshold = 1024;
const std::string  ImageDir = "..\\Test image\\";



//const double MaxGrayScale = 255.0;



#endif
