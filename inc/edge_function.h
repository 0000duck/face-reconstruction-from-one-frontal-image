#ifndef EDGE_FUNCTION_H_
#define EDGE_FUNCTION_H_



class EdgeFunction
{
public:
	EdgeFunction(){}

	void SetUp(int signed_triangle_area, int x0, int y0, int x1, int y1);
	bool IsInside(int x, int y, int& signed_distance) const;

protected:
private:
	int A_;
	int B_;
	int C_;

};









#endif