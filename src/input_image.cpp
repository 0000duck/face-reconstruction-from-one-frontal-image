#include "input_image.h"
#include "common.h"

#include <string>

using namespace cv;
using namespace std;
using namespace arma;

// define static member

vector<Point> InputImage::landmarks_;
Point InputImage::pos;


InputImage::InputImage(int rows, int cols, int type, const Scalar& s) :data_(Mat(rows, cols, type, s))
{
}


InputImage::InputImage(const cv::Mat& data) :data_(data)
{ 

}

void InputImage::Show() const
{
	const string window_name = "input_image";
	namedWindow(window_name);
	imshow(window_name, data_);
	waitKey(0);
	destroyWindow(window_name);
}


void InputImage::ChooseLandmark(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		pos = Point(x, y);
		landmarks_.push_back(pos);
	}

}


vec3 InputImage::GetColor(Point& pos) const
{
	vec3 rgb;
	Vec3d cv_bgr = data_.at<Vec3b>(pos.y, pos.x);

	rgb[0] = cv_bgr[2]; // R
	rgb[1] = cv_bgr[1]; // G
	rgb[2] =cv_bgr[0]; // B

	return rgb;
}


vec3 InputImage::Sobel(const Point& pos, const Direction& d) const
{
	vec3 res;
	if (d==Direction::X)
	{
		Vec3d col00 = data_.at<Vec3b>(pos.y - 1, pos.x - 1);
		Vec3d col01 = data_.at<Vec3b>(pos.y, pos.x - 1);
		Vec3d col02 = data_.at<Vec3b>(pos.y + 1, pos.x - 1);  

		Vec3d col20 = data_.at<Vec3b>(pos.y - 1, pos.x + 1);
		Vec3d col21 = data_.at<Vec3b>(pos.y, pos.x + 1);
		Vec3d col22 = data_.at<Vec3b>(pos.y + 1, pos.x + 1);

		Vec3d sobel_x = (col20 + 2 * col21 + col22) - (col00 + 2 * col01 + col02);

		sobel_x /= 8;
	//	sobel_x /= MaxGrayScale; // convert to  double

		res[0] = sobel_x[2]; // R 
		res[1] = sobel_x[1]; // G
		res[2] = sobel_x[0]; // B

		return res;
	}

	else if (d==Direction::Y)
	{
		Vec3d row00 = data_.at<Vec3b>(pos.y - 1, pos.x - 1);
		Vec3d row01 = data_.at<Vec3b>(pos.y - 1, pos.x);
		Vec3d row02 = data_.at<Vec3b>(pos.y - 1, pos.x + 1);

		Vec3d row20 = data_.at<Vec3b>(pos.y + 1, pos.x - 1);
		Vec3d row21 = data_.at<Vec3b>(pos.y + 1, pos.x);
		Vec3d row22 = data_.at<Vec3b>(pos.y + 1, pos.x + 1);

		Vec3d sobel_y = (row20 + 2 * row21 + row22) - (row00 + 2 * row01 + row02);

		sobel_y /= 8;
	//	sobel_y /= MaxGrayScale; // convert to double

		res[0] = sobel_y[2]; // R
		res[1] = sobel_y[1]; // G
		res[2] = sobel_y[0]; // B

		return res;
	}

	throw SobelError();
}

unsigned char InputImage::SobelX(const Mat& data, const Point& pos) const
{
	double col00 = data.at<unsigned char>(pos.x - 1, pos.y - 1);
	double col01 = data.at<unsigned char>(pos.x, pos.y - 1);
	double col02 = data.at<unsigned char>(pos.x + 1, pos.y - 1);
	double col20 = data.at<unsigned char>(pos.x - 1, pos.y + 1);
	double col21 = data.at<unsigned char>(pos.x, pos.y + 1);
	double col22 = data.at<unsigned char>(pos.x + 1, pos.y + 1);
	double sobel_x = (col20 + 2 * col21 + col22) - (col00 + 2 * col01 + col02);

	//sobel_x /= 8;
	sobel_x = abs(sobel_x);
	sobel_x = std::min(sobel_x, 255.0);
	return static_cast<unsigned char>(sobel_x);
}

unsigned char InputImage::SobelY(const Mat& data, const Point& pos) const
{
	double row00 = data.at<unsigned char>(pos.x - 1, pos.y - 1);
	double row01 = data.at<unsigned char>(pos.x - 1, pos.y);
	double row02 = data.at<unsigned char>(pos.x - 1, pos.y + 1);
	double row20 = data.at<unsigned char>(pos.x + 1, pos.y - 1);
	double row21 = data.at<unsigned char>(pos.x + 1, pos.y);
	double row22 = data.at<unsigned char>(pos.x + 1, pos.y + 1);
	double sobel_y = (row20 + 2 * row21 + row22) - (row00 + 2 * row01 + row02);

	//sobel_y /= 8;
	sobel_y = abs(sobel_y);
	sobel_y = std::min(sobel_y, 255.0);
	return static_cast<unsigned char>(sobel_y);
}





