#ifndef IMAGE_FACE_DETECT_H_
#define IMAGE_FACE_DETECT_H_


#include <opencv2/opencv.hpp>
#include <vector>
#include "common.h"

class FaceDetector
{
	using Rect = cv::Rect;
	using Mat= cv::Mat;
	using Size=cv::Size;
	using CascadeClassifier = cv::CascadeClassifier;

public:
	FaceDetector();
	std::vector<Rect> Detect(Mat& target_image, bool show = false);
	void Crop(Mat& src, Mat& dst);
	void Resize(Mat& src, Mat& dst, Size new_size = Size(IMAGE_WIDTH, IMAGE_HEIGHT));

	//const std::string face_cascade_name = "haarcascade_frontalface_default.xml";
	//const std::string face_cascade_name = "haarcascade_frontalface_alt.xml";
	//const std::string face_cascade_name = "haarcascade_frontalface_alt2.xml";
	const std::string face_cascade_name = "haarcascade_frontalface_alt_tree.xml";


private:
	CascadeClassifier face_cascade;

};














#endif 