#include "detect_edge.h"
#include "input_image.h"

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int DetectEdge(const string& image_name)
{
	//cv::Mat small(100, 100, CV_8UC3);
	//Vec3b color_b(255, 0, 0);
	//Vec3b color_g(0, 255, 0);
	//Vec3b color_r(0, 0, 255);

	//small.at<Vec3b>(49, 50) = color_b;
	//small.at<Vec3b>(50, 50) = color_b;
	//small.at<Vec3b>(51, 50) = color_b;

 //  imwrite("small.jpg",small);

 //  namedWindow("small", CV_WINDOW_AUTOSIZE);
 //  imshow("small", small);
 //  cvWaitKey(0);



	cv::Mat src, src_gray;
	cv::Mat grad;
	char* window_name = "Sobel Demo - Simple Edge Detector";

	/// Load an image
	src = imread(image_name);
	if (!src.data)
	{
		return -1;
	}

	GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);

	/// Convert it to gray
	cvtColor(src, src_gray, CV_RGB2GRAY);

	/// Create window
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);



	int border = 1;
	int rows = src_gray.rows;
	int cols = src_gray.cols;

	/// Generate grad_x and grad_y
	cv::Mat grad_x(rows, cols, src_gray.type());
	cv::Mat grad_y(rows, cols, src_gray.type());

	InputImage input_image;

	for (int r = border; r < rows-border ; ++r)
	{
		for (int c = border; c < cols-border ; ++c)
		{
			grad_x.at<unsigned char>(r,c) = input_image.SobelX(src_gray, Point(r, c));
		}
	}

	for (int r = border; r < rows - border; ++r)
	{
		for (int c = border; c < cols - border; ++c)
		{
			grad_y.at<unsigned char>(r, c) = input_image.SobelY(src_gray, Point(r, c));
		}
	}
	
	/// Total Gradient (approximate)
	addWeighted(grad_x, 0.5, grad_y, 0.5, 0, grad);

	imshow(window_name, grad);

	waitKey(0);

	return 0;

}



