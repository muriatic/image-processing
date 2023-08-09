#include "rectangle.cpp"
#include <iostream>


using namespace std;

void TestRectangle()
{
	My_Point ptA(10, 20);
	My_Point ptB(15, 33);
	My_Point ptC(155, 0);
	My_Point ptD(0, 123);

	Rectangle Box(ptA, ptB, ptC, ptD);

	std::cout << Box.pointA.coords_str() << endl;
	std::cout << Box.pointB.coords_str() << endl;
	std::cout << Box.pointC.coords_str() << endl;
	std::cout << Box.pointD.coords_str() << endl;

	My_Point newPointD(125, 125);

	Box.pointD = newPointD;

	std::cout << Box.pointD.coords_str() << endl;


	Rectangle NewBox;

	My_Point newPtA(100, 100);

	NewBox.CreateFrom_Point_Width_Height(newPtA, 50, 100);

	std::cout << NewBox.pointA.coords_str() << endl;
	std::cout << NewBox.pointB.coords_str() << endl;
	std::cout << NewBox.pointC.coords_str() << endl;
	std::cout << NewBox.pointD.coords_str() << endl;
}