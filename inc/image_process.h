#ifndef IMAGE_PROCESS_H_
#define IMAGE_PROCESS_H_


#include <opencv2/opencv.hpp>
#include <memory>

//#include "input_image.h"

class InputImage;
std::shared_ptr<InputImage> PreprocessImage(cv::Mat& src, cv::Mat& dst);


#endif