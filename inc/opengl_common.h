#ifndef OPENGL_COMMON_H_
#define OPENGL_COMMON_H_


const double PI = 3.14159265358979323846;
//const double Deg2Rad = 3.14159265358979323846 / 180;
//const double Rad2Deg = 1 / Deg2Rad;

const int DepthThreshold = 1;	// 1


// camera const variable
const double ModelScale = 0.001; //0.0075
const double ShiftObject = -46.125;
const double CameraPosition = 3400; // 5000
const double CameraBias = CameraPosition / 100;
const double DepthRange[2] = { 0.0, 1.0 };



enum Frustum
{
	NEAR_PLANE = 1020,// unit mm
	FAR_PLANE=5100,
};

const double FocalLength = 6000;
const double ObjectSize = 160;
const double ObjectZ[2] = { 3320,3500}; //3300 3500

// illumination const variable
const double Reflection = 25.5; //0.12 25.5
const int Shininess = 8;
const double ColorCoefficents[3] = { 0.3, 0.59, 0.11 };

const int ShadowResolution = 512;


#endif