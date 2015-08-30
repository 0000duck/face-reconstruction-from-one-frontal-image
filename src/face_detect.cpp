#include "face_detect.h"

using namespace cv;
using namespace std;

FaceDetector::FaceDetector()
{
	face_cascade.load(face_cascade_name);
}

vector<Rect> FaceDetector::Detect(Mat& src, bool show) 
{
	vector<Rect> faces;
	Mat small;
	pyrDown(src, small, Size(src.cols / 2, src.rows / 2));

	Mat target_image_gray;
	cvtColor(small, target_image_gray, COLOR_BGR2GRAY);
	equalizeHist(target_image_gray,target_image_gray);
	face_cascade.detectMultiScale(target_image_gray,faces);

	for (auto& e : faces)
	{
		Rect real;
		real.x = e.x * 2;
		real.y = e.y * 2;
		real.width = e.width * 2;
		real.height = e.height * 2;
		e = real;
		//rectangle(src, real, Scalar(255, 0, 0), 2);
	}
	
	//namedWindow("face");
	//imshow("face", src);
	//waitKey(0);

	return faces;
}

void FaceDetector::Crop(cv::Mat& src, cv::Mat& dst)
{
	
	Rect rect = Detect(src,true)[0];
	Rect crop_rect;
	int new_size = rect.height+rect.height/2;

	int center_x = rect.x + rect.width / 2;
	int center_y = rect.y + rect.height / 2;

	crop_rect.x = max(center_x - new_size / 2, 0);
	crop_rect.y = max(center_y - new_size / 2, 0);
	crop_rect.width = min(new_size,src.cols-crop_rect.x);
	crop_rect.height = min(new_size, src.rows-crop_rect.y);
	dst = src(crop_rect);

}


void FaceDetector::Resize(Mat& src, Mat& dst,Size new_size)
{
	resize(src, dst, new_size);
}






