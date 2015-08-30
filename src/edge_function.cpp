#include "edge_function.h"



void EdgeFunction::SetUp(int signed_triangle_area ,int x0,int y0,int x1,int y1)
{
	A_=y0-y1;  // A
	B_=x1-x0;  // B

	if(signed_triangle_area<0)	// need to flip the sign of edge functions?
	{
		A_*=-1;
		B_*=-1;
	}

	//	C= -A*x0 - B*y0 = x0*y1-x1*y0
	C_=-A_*x0-B_*y0; // C
}


bool EdgeFunction::IsInside(int x, int y, int& signed_distance) const
{
	// distance= Ax+ By+C
	signed_distance =A_*x + B_*y + C_;

	if(signed_distance>0) return true;
	else if(signed_distance<0) return false;
	if(A_>0) return true; // A>0
	else if(A_<0) return false;  // A<0
	if(B_>0) return true; // B>0
	return false;
} 
