#ifndef  INPUT_IMAGE_H_
#define INPUT_IMAGE_H_



#include "face_detect.h"
#include <opencv2/opencv.hpp>
#include <string>
#include<armadillo>


class InputImage
{
	using Landmarks = std::vector<cv::Point>;

	using Mat=cv::Mat;
	using Point=cv::Point;
	using Vec3b = cv::Vec3b;
	using Scalar = cv::Scalar;

	using vec2=arma::vec2;
	using vec3=arma::vec3;
	using ivec3=arma::ivec3;


public:

	InputImage(int rows = IMAGE_WIDTH, int cols = IMAGE_HEIGHT, int type = CV_8UC3, const Scalar& s = Scalar(255, 255, 255));
	InputImage(const Mat& data);

	static void ChooseLandmark(int event, int x, int y, int flags, void* userdata);

	void Show() const;

	Mat Clone(){ return data_.clone(); }


	class LandmarkNumberError{};

	enum 
	{
		Step=3
	};


	enum class Direction
	{
		X,
		Y
	};


	enum LandmarkId
	{
		LEFT_EYEBROW_RIGHT_CORNER = 24,
		LEFT_EYE_LEFT_CORNER = 27,
		RIGHT_EYE_LEFT_CORNER = 34,
		LEFT_EARBOE = 1,
		NOSE_LEFT_CORNER = 40,
		NOSE_RIGHT_CORNER = 42,
		RIGHT_EARLOBE = 13,
		UPER_LIP_CENTER = 51,
		MOUTH_LEFT_CORNER = 48
	};

	vec2 LandmarkPosition(int i)
	{
		vec2 pos;
		pos[0] = landmarks_[i].x;
		pos[1] = landmarks_[i].y;
		return pos;
	}

	vec3 GetColor(Point& pos) const;
	vec3 Sobel(const Point& pos, const Direction& d) const;
	unsigned char SobelX(const Mat& data, const Point& pos) const;
	unsigned char SobelY(const Mat& data, const Point& pos) const;


	int Rows() const { return data_.rows; }
	int Cols() const { return data_.cols; }

	class SobelError{};

public:
	static Point pos;
	static Landmarks landmarks_;

	const int DistanceThreshold = 90;

private:
	const int MaskSize = 3;
	Mat data_;
};






#endif