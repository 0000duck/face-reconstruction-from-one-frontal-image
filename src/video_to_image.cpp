#include "video_to_image.h"
#include <opencv2/opencv.hpp>


#include <string>
#include <iostream>

using namespace std;
using namespace cv;



int Video2Image(const string& file_name)
{
	
	VideoCapture fitting_video(file_name);
	if (!fitting_video.isOpened())
	{
		return -1;
	}

	int n = 0;

	Mat frame;
	while (fitting_video.read(frame))
	{
		++n;
		stringstream file_num;
		file_num.clear();
		file_num.str("");
		file_num << n;
		string dir = "E:\\study\\my project\\face reconstruction\\face reconstruction paper\\report 6\\face_rec_tutorial\\fitting";
		string file_name =dir+ file_num.str() + ".jpg";
	    imwrite(file_name,frame);
	}

	fitting_video.release();

	return 0;
}