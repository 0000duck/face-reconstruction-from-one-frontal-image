//#include "making_face.h"
#include "cpu_rendering.h"
#include "gpu_rendering.h"
#include "fitting_to_image.h"
//#include "common.h"
//
//#include "illumination_parameter.h"

#include <iostream>
#include <random>
#include <functional>
#include <fstream>
#include <thread>
#include <vector>

//#include "detect_edge.h"

//#define ARMA_64BIT_WORD
#include <armadillo>

using namespace std;
using namespace arma;




int main()
{



	//vector<thread> threads;

	//threads.push_back(thread(GpuRendering, true));
	//threads.push_back(thread(Fitting2Image));


	//for (auto& t:threads)
	//{
	//	t.join();
	//}

	// DetectEdge(ImageDir + "//front" + "//xiao2.jpg");


 
	/////////////////////////Generate Face Test//////////////////////////////////////////////////
	{
		//GenerateMeanFace();
		//GenerateRandomFace("random face.ply");
	}
	//////////////////////////Render Test/////////////////////////////////////////////////
	{
	  //CpuRendering();
	   //GpuRendering(false);
	 // GpuRendering(true);
    }

	//////////////////////////Construct Test////////////////////////////////////////////////
	{
		Fitting2Image();
	}

	//////////////////////////Input Image Test////////////////////////////////////////////////
	{
		//cv::Mat src = cv::imread("dong.jpg");
		//cv::Mat dst;
		//PreprocessImage(src, dst);
		//InputImage input_image(dst); 
		//input_image.PrintLandMark();
	}
	//////////////////////////Bias Variance Trade Off Test////////////////////////////////////////////////
	{




	}

	return 0; 
} 










