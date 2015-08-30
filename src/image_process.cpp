#include "image_process.h"
#include "face_detect.h"

#include "asmfitting.h"
#include "vjfacedetect.h"

#include "input_image.h"

#include <array>

using namespace cv;
using namespace std;


void ChooseLandmark(cv::Mat& src);
int DetectFeature(IplImage *image, InputImage& input_image);


shared_ptr<InputImage> PreprocessImage(cv::Mat& src, cv::Mat& dst)
{
	FaceDetector face_detector;
	cv::Mat after_crop;

	/**Crop and resample*/
	face_detector.Crop(src, after_crop);
	resize(after_crop, dst, Size(IMAGE_WIDTH, IMAGE_HEIGHT));

	shared_ptr<InputImage> input = make_shared<InputImage>(dst);

	//IplImage* face = &IplImage(dst);
	//DetectFeature(face, *input);

	cv::Mat dst_copy = dst.clone();
	ChooseLandmark(dst_copy);

	//imwrite("feature.png", dst_copy);
	return input;
}


void ChooseLandmark(cv::Mat& src)
{
	/**Choose landmark*/
	const string window_name = "input_image";
	namedWindow(window_name);
	setMouseCallback(window_name, InputImage::ChooseLandmark, (void*)&src);

	while (1)
	{

		circle(src, InputImage::pos, 1, Scalar(0, 255, 0), 2);
		imshow(window_name, src);
		if (InputImage::landmarks_.size() == LandmarkNum) break;
		if (cvWaitKey(15) == 27) break;
	}

	destroyWindow(window_name);
}


int DetectFeature(IplImage *image, InputImage& input_image)
{
	//face feature detection
	asmfitting fit_asm;
	char* model_name = "my68-1d.amf";
	char* cascade_name = "haarcascade_frontalface_alt2.xml";


	int n_iteration = 20;
	if (fit_asm.Read(model_name) == false)
		return -1;

	if (init_detect_cascade(cascade_name) == false)
		return -1;
	{	
		//double t = (double)cvGetTickCount();

		int nFaces;
		asm_shape *shapes = NULL, *detshapes = NULL;

		// step 1: detect face
		bool flag = detect_all_faces(&detshapes, nFaces, image);

		// step 2: initialize shape from detect box
		if (flag)
		{
			shapes = new asm_shape[nFaces];
			for (int i = 0; i < nFaces; i++)
			{
				InitShapeFromDetBox(shapes[i], detshapes[i], fit_asm.GetMappingDetShape(), fit_asm.GetMeanFaceWidth());
			}
		}
		else
		{
			fprintf(stderr, "This image doesnot contain any faces!\n");
			exit(0);
		}

		// step 3: image alignment fitting
		fit_asm.Fitting2(shapes, nFaces, image, n_iteration);

		//t = ((double)cvGetTickCount() - t) / (cvGetTickFrequency()*1000.);
		//printf("ASM fitting time cost: %.2f millisec\n", t);



		array<int, LandmarkNum> landmark_id = {
			{
				InputImage::LEFT_EYEBROW_RIGHT_CORNER,
				InputImage::LEFT_EYE_LEFT_CORNER,
				InputImage::RIGHT_EYE_LEFT_CORNER,
				InputImage::LEFT_EARBOE,
				InputImage::NOSE_LEFT_CORNER,
				InputImage::NOSE_RIGHT_CORNER,
				InputImage::RIGHT_EARLOBE,
				//InputImage::UPER_LIP_CENTER,
				//InputImage::MOUTH_LEFT_CORNER
			} 
		};


		//cvNamedWindow("Fitting", 1);
		//if necessary, get the detected shape features

		input_image.landmarks_.resize(LandmarkNum);
		for (int i = 0; i < nFaces; i++)
		{
			for (int n = 0; n < LandmarkNum;++n)
			{
				int x = static_cast<int>(shapes[i][landmark_id[n]].x);
				int y = static_cast<int>(shapes[i][landmark_id[n]].y);
				input_image.landmarks_[n] = Point(x, y);
				//cvCircle(image, Point(x, y), 2, Scalar(0, 0, 255), 2);
				//cvShowImage("Fitting", image);
				//cvWaitKey(0);
			}
		}

		// step 5: free resource
		delete[] shapes;
		free_shape_memeory(&detshapes);
	}

	return 1;
}