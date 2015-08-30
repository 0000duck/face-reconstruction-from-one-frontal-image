#include "model_image.h"

#include <array>
#include <string>

using namespace cv;
using namespace std;
using namespace  arma;


default_random_engine ModelImage:: engine_;
uniform_int_distribution<unsigned> ModelImage::distribution_area_;
uniform_int_distribution<unsigned> ModelImage::distribution_id_;


ModelImage::ModelImage(int rows, int cols, int type, const Scalar& s) 
:data_(Mat(rows, cols, type, s)), eye_{ 0 }, nose_{ 0 }, mouth_{ 0 }, rest_{ 0 }, all_{ 0 }
{
	
}

ModelImage::ModelImage(Mat src) : data_(src),  eye_{ 0 }, nose_{ 0 }, mouth_{ 0 }, rest_{ 0 }, all_{ 0 }
{
}


void ModelImage::WriteColor(const Pixel& pixel)
{
	int x = std::min<int>(pixel.coordinate[0],IMAGE_WIDTH-1);
	int y = std::min<int>(pixel.coordinate[1], IMAGE_HEIGHT-1);

	x = std::max<int>(x,0);
	y = std::max<int>(y,0);

	//data_.at<Vec3b>(y, x)[0] = pixel.color[2]; // B
	//data_.at<Vec3b>(y, x)[1] = pixel.color[1]; // G
	//data_.at<Vec3b>(y, x)[2] = pixel.color[0]; // R

	data_.at<Vec3b>(y, x)[0] = 0;
	data_.at<Vec3b>(y, x)[1] = 255;
	data_.at<Vec3b>(y, x)[2] = 255;
}


// read pixel from OpenGL in BGR order
void ModelImage::WriteColor(int row, int col, const Color& color)
{
	int x = std::min<int>(row, IMAGE_WIDTH-1);
	int y = std::min<int>(col, IMAGE_HEIGHT-1);
	x = std::max<int>(x, 0);
	y = std::max<int>(y, 0);


	data_.at<Vec3b>(y, x) = Vec3b(color.BGR[0],color.BGR[1],color.BGR[2]); 
}

void ModelImage::WriteColor(int row, int col, const Scalar& color)
{
	int x = std::min<int>(row, IMAGE_WIDTH-1);
	int y = std::min<int>(col, IMAGE_HEIGHT-1);
	x = std::max<int>(x, 0);
	y = std::max<int>(y, 0);

	data_.at<Vec3b>(y, x)[0] = static_cast<unsigned char>(color[2]); //B
	data_.at<Vec3b>(y, x)[1] = static_cast<unsigned char>(color[1]); //G
	data_.at<Vec3b>(y, x)[2] = static_cast<unsigned char>(color[0]); //R

}

void ModelImage::WriteColor(int row, int col, const Vec3b& color)
{
	int x = std::min<int>(row, IMAGE_WIDTH-1);
	int y = std::min<int>(col, IMAGE_HEIGHT-1);
	x = std::max<int>(x, 0);
	y = std::max<int>(y, 0);


	data_.at<Vec3b>(y, x) = color;
}


void ModelImage::Show() const
{
	const string window_name = "model";
	namedWindow(window_name);
	imshow(window_name, data_);
	waitKey(0);
	destroyWindow(window_name);
}



void ModelImage::InitialRandomGenerator(int s)
{
	if (s == NOSE)
		InitialRandomGenerator(nose_);
	else if (s == EYE)
		InitialRandomGenerator(eye_);
	else if (s == MOUTH)
		InitialRandomGenerator(mouth_);
	else if (s == REST)
		InitialRandomGenerator(rest_);
	else
		InitialRandomGenerator(all_);
}

void ModelImage::EnableIterator(int s)
{
	
	if (s == NOSE)
		visible_triangles_ = begin(nose_);
	else if (s == EYE)
		visible_triangles_ = begin(eye_);
	else if (s == MOUTH)
		visible_triangles_ = begin(mouth_);
	else if (s == REST)
		visible_triangles_ = begin(rest_);
	else
		visible_triangles_=begin(all_);
}


void ModelImage::InitialRandomGenerator(const std::vector<VisibleTriangle>& source)
{
	visible_num_ = source.size();
	int id = 0;
	max_area_ = 0;
	for (int i = 0; i<visible_num_; ++i)
	{
		if (max_area_ < source[i].area)
		{
			max_area_ = source[i].area;
			id = source[i].id;
		}
	}

	cout << "max_area = " << max_area_ << endl;
	cout << "it's id = " << id << endl;

	engine_ = default_random_engine(static_cast<unsigned>(time(0)));
	distribution_id_ = uniform_int_distribution<unsigned>(0, visible_num_ - 1);
	distribution_area_ = uniform_int_distribution<unsigned>(1, max_area_);

}


void ModelImage::InitialRandomGenerator2(const std::vector<VisibleTriangle>& source)
{
	random_.clear();
	visible_num_ = source.size();
	for (int i = 0; i < visible_num_; ++i)
	{
		int area = source[i].area;
		for (int j = 0; j <area; ++j)
		{
			random_.push_back(i);
		}	
	}

	random_.shrink_to_fit();
	engine_ = default_random_engine(static_cast<unsigned>(time(0)));
	distribution_id_ = uniform_int_distribution<unsigned>(0, random_.size() - 1);
}



void ModelImage::GenerateRandomNumbers(int num,vector<int>& ids) const
{
	ids.resize(num);
	for (int i = 0; i < num; ++i)
	{
		ids[i] = NextInt();
	}
}


void ModelImage::GenerateRandomNumbers2(int num, vector<int>& ids) const
{
	ids.resize(num);
	for (int i = 0; i < num; ++i)
	{
		ids[i] = random_[distribution_id_(engine_)];
	}
}


int ModelImage::NextInt() const
{
	//static default_random_engine engine(static_cast<unsigned>(time(0)));
	//static uniform_int_distribution<unsigned> distribution_id;
	//static uniform_int_distribution<unsigned> distribution_area;

	//distribution_id = uniform_int_distribution<unsigned>(0, visible_num_ - 1);
	//distribution_area = uniform_int_distribution<unsigned>(1, max_area_);

	int id=0;
	while (true)
	{
		id = distribution_id_(engine_);
		int area = distribution_area_(engine_);
		if (area<=visible_triangles_[id].area) break;
	}

	return id;
}

void ModelImage::PushBack(const VisibleTriangle& visible_triange, int s)
{
    if (s == NOSE)
		nose_.push_back(visible_triange);
	else if (s == EYE)
		eye_.push_back(visible_triange);
	else if (s == MOUTH)
		mouth_.push_back(visible_triange);
	else if (s == REST)
		rest_.push_back(visible_triange);
	else
		all_.push_back(visible_triange);

}


void ModelImage::Clear(int s)
{
	if (s == NOSE)
		nose_.clear();
	else if (s == EYE)
		eye_.clear();
	else if (s == MOUTH)
		mouth_.clear();
	else if (s == REST)
		rest_.clear();
	else
		all_.clear();
}


void ModelImage::Shrink2Fit(int s)
{
	if (s == NOSE)
		nose_.shrink_to_fit();
	else if (s == EYE)
		eye_.shrink_to_fit();
	else if (s == MOUTH)
		mouth_.shrink_to_fit();
	else if (s == REST)
		rest_.shrink_to_fit();
	else
		all_.shrink_to_fit();
}

int ModelImage::Size(int s)
{
	if (s == NOSE)
		return nose_.size();
	else if (s == EYE)
		return eye_.size();
	else if (s == MOUTH)
		return mouth_.size();
	else if (s == REST)
		return rest_.size();
	else
		return all_.size();
}

