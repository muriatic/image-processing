#include <iostream>
#include <string>
#include <tuple>
#include <set>


using namespace std;

// coordinate data class that stores two short integers
class My_Point
{
	public:
		short int x;
		short int y;

		My_Point(short int x_input = 0, short int y_input = 0)
		{
			x = x_input;
			y = y_input;
		}

		/*void set(pair <short int, short int> ptCoords) {
			x = ptCoords.first;
			y = ptCoords.second;
		}*/

		pair <short int, short int> coords()
		{
			pair <short int, short int> coordinate = { x, y };
			return coordinate;
		}

		string coords_str()
		{
			string coordinate_str = "(" + to_string(x) + ", " + to_string(y) + ")";
			return coordinate_str;
		}
};


// stores 4 Points
class Rectangle
{

	public:
		My_Point pointA;
		My_Point pointB;
		My_Point pointC;
		My_Point pointD;

		unsigned short int w;
		unsigned short int h;

		Rectangle(My_Point ptA = { 0, 0 }, My_Point ptB = { 0, 0 }, My_Point ptC = { 0, 0 }, My_Point ptD = { 0 ,0 })
		{
			pointA = ptA;
			pointB = ptB;
			pointC = ptC;
			pointD = ptD;

			GetWidthAndHeight(pointA, pointC);
		}

		void GetWidthAndHeight(My_Point ptA, My_Point ptC)
		{
			w = (ptC.x - ptA.x);
			h = (ptC.y - ptA.y);
		}

		void CreateFrom_Point_Width_Height(My_Point ptA, unsigned short int width, unsigned short int height)
		{
			// requires top left point, width, and height

			w = width;
			h = height;

			My_Point ptB(ptA.x + w, ptA.y);
			My_Point ptC(ptA.x + w, ptA.y + h);
			My_Point ptD(ptA.x, ptA.y + h);

			pointA = ptA;
			pointB = ptB;
			pointC = ptC;
			pointD = ptD;
		}

		// shifts all 4 points by the width and height differential
		void ShiftPoints(short int widthDif, short int heightDif)
		{
			pointA.x += widthDif;
			pointB.x += widthDif;
			pointC.x += widthDif;
			pointD.x += widthDif;

			pointA.y += heightDif;
			pointB.y += heightDif;
			pointC.y += heightDif;
			pointD.y += heightDif;
		}
};

