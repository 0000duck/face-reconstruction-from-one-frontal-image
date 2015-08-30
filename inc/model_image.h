#ifndef OPENGL_model_H_
#define OPENGL_model_H_

#include <opencv2/opencv.hpp>
#include <array>
#include <random>
#include <memory>
#include "common.h"
#include "entity.h"



struct VisibleTriangle
{
	int id;
	int area;
	bool cast_shadow;
};

class ModelImage
{
	using uchar=unsigned char;
	using vec3=arma::vec3;
	using ivec3=arma::ivec3;
	using Scalar = cv::Scalar;
	using Mat=cv::Mat;
	using Point=cv::Point;
	using Vec3b = cv::Vec3b;
	using TriangleIterator = std::vector<VisibleTriangle>::const_iterator;


public:
	ModelImage(int rows = IMAGE_WIDTH, int cols = IMAGE_HEIGHT, int type = CV_8UC3, const Scalar& s = Scalar(255, 255, 255));
	ModelImage(Mat src);


	void Write(const std::string& image_name){ imwrite(image_name,data_); }
	void WriteColor(const Pixel& pixel);
	void WriteColor(int row, int col, const Color& color);
	void WriteColor(int row, int col, const Scalar& color);
	void WriteColor(int row, int col, const Vec3b& color);

	void Initial()
	{ 
		//data_.release(); 
		for (int i = 0; i < IMAGE_WIDTH;++i)
		{
			for (int j = 0; j < IMAGE_HEIGHT;++j)
			{
				data_.at<Vec3b>(i, j) = Vec3b(255,255,255);
			}
		}
	}


	void Circle(const Point& center)
	{
		circle(data_, center, 1, Scalar(0,0,255), 2);
	}

	void Show(const std::string& window_name) const{ imshow(window_name,data_); }
	void Show() const;


	int Rows() { return data_.rows; }
	int Cols(){ return data_.cols; }

	void SetSize(int width, int height){ data_.rows = width; data_.cols = height; }

	enum Segments
	{
		NOSE,
		EYE,
		MOUTH,
		REST,
		ALL
	};

	void EnableIterator(int s = ALL);
	void InitialRandomGenerator(int s=ALL);
	void GenerateRandomNumbers(int num, std::vector<int>& ids) const;
	void GenerateRandomNumbers2(int num, std::vector<int>& ids) const;

	void PushBack(const VisibleTriangle& visible_triange, int s = ALL);
	void Clear(int s = ALL);
	void Shrink2Fit(int s = ALL);
	int Size(int s = ALL);

	enum LandmarkId
	{
		//LEFT_EYEBROW_RIGHT_CORNER = 6450,
		LEFT_EYE_LEFT_CORNER = 2088,
		//RIGHT_EYE_LEFT_CORNER = 10475,
		RIGHT_EYE_RIGHT_CORNER = 14472,


		//LEFT_EAR_LOBE=21140,
		//LEFT_EAR_UPER= 19963,
		//LEFT_EAR_MIDDLE = 19325,
		//LEFT_EAR_LOWER = 20205,


		//NOSE_LEFT_CORNER = 5879,
		NOSE_CENTER = 8320,
		//NOSE_RIGHT_CORNER = 10782,

		//RIGHT_EAR_LOBE=34162,
		//RIGHT_EAR_UPER = 35151,
		//RIGHT_EAR_MIDDLE = 35802,
		//RIGHT_EAR_LOWER= 34983,

		LEFT_OUTLINE = 43070,


		//UPER_LIP_CENTER = 8474,
		MOUTH_LEFT_CORNER = 5394,
		MOUTH_RIGHT_CORNER = 11326,

		RIGHT_OUTLINE = 53307,

	};

	std::array<int, LandmarkNum>landmarks_ = 
	{ 
		{
			LEFT_EYE_LEFT_CORNER,
			RIGHT_EYE_RIGHT_CORNER,

		//	LEFT_EAR_UPER,
		//	LEFT_EAR_LOWER,
			NOSE_CENTER ,
		//	RIGHT_EAR_UPER,
		//	RIGHT_EAR_LOWER,

			LEFT_OUTLINE,
			MOUTH_LEFT_CORNER,
			MOUTH_RIGHT_CORNER,

			RIGHT_OUTLINE ,

		}
	};

	TriangleIterator visible_triangles_;

private:

	std::vector<VisibleTriangle> all_;
	std::vector<VisibleTriangle> eye_;
	std::vector<VisibleTriangle> nose_;
	std::vector<VisibleTriangle> mouth_;
	std::vector<VisibleTriangle> rest_;
	std::vector<int> random_;

	Mat data_;
	int visible_num_ = 0;
	int max_area_=0;

	static std::default_random_engine engine_;
	static std::uniform_int_distribution<unsigned> distribution_id_;
	static std::uniform_int_distribution<unsigned> distribution_area_;


	void InitialRandomGenerator(const std::vector<VisibleTriangle>& source);
	void InitialRandomGenerator2(const std::vector<VisibleTriangle>& source);

	int NextInt() const;


};


// as #define in c, can't be accessed outside class
//static const int IMAGE_WIDTH=512; 
//static const int IMAGE_HEIGHT=512;
// in c++11
// static constexpr int IMAGE_WIDTH=512;  // get value at compile time 
// static constexpr int IMAGE_HEIGHT=512; // get value at compile time











#endif