#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <filesystem>
#include <tuple>
#include <set>

namespace fs = std::filesystem;

using namespace std;
using namespace cv;


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

		pair <short int, short int> coords() 
		{
			pair <short int, short int> coordinate = {x, y};
			return coordinate;
		}

		string coords_str()
		{
			string coordinate_str = std::format("({}, {})", x, y);
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

		Rectangle(My_Point ptA = {0, 0}, My_Point ptB = {0, 0}, My_Point ptC = {0, 0}, My_Point ptD = {0 ,0})
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


void TestRectangle()
{
	My_Point ptA( 10 , 20 );
	My_Point ptB( 15 , 33 );
	My_Point ptC( 155 , 0 );
	My_Point ptD( 0, 123 );

	Rectangle Box(ptA, ptB, ptC, ptD);

	std::cout << Box.pointA.coords_str() << endl;
	std::cout << Box.pointB.coords_str() << endl;
	std::cout << Box.pointC.coords_str() << endl;
	std::cout << Box.pointD.coords_str() << endl;

	My_Point newPointD ( 125, 125 );

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


enum class FeatureNames
{
	FACE,
	LEFT_EYE,
	RIGHT_EYE,
	EYES
};


// Identifies the Feature
class FeatureProcessor
{
	unsigned short int featureIDX;

	CascadeClassifier featureCascade;

	Mat grayScaleImage;

	string haarcascadeDirectoryPath;

	string cascades[3] = {
		"haarcascade_frontalface_default.xml",
		"haarcascade_eye.xml",
		"haarcascade_eye.xml"
	};

	unsigned short int listLength = 10;

	vector <int> mNList;
	vector <float> sFList;

	// -1 means not implemented and it will default to the mNList and sFList
	float scaleFactors[3] = { 1.8, -1, -1 };
	int minNeighbors[3] = { 4, -1, -1 };


	vector<Rect> feature;


	FeatureNames featureName;

	unsigned short int offset = 0;

	public:
		Rectangle featureBox;

		My_Point centerPoint;

		FeatureProcessor(FeatureNames featureName_input, string haarscadeDirectory_input = "C:/opencv/sources/data/haarcascades/")
		{
			featureName = featureName_input;

			featureIDX = int(featureName_input);

			haarcascadeDirectoryPath = haarscadeDirectory_input;

			// creating 10 length list
			CreateLists(listLength);
		}
		
		// performs all the steps to create the feature and its info
		void CreateFeature(Mat grayScaleImage_input)
		{
			LoadCascadePath();

			grayScaleImage = grayScaleImage_input;


			// HalveGrayScaleImage() if needed
			if (featureName == FeatureNames::LEFT_EYE || featureName == FeatureNames::RIGHT_EYE)
			{
				// convert featureIDX to true or false depending on if its left or right eye
				HalveGrayScaleImage();
			}

			if (!FeatureSearch())
			{
				FeatureDeepSearch();
			}

			CreateBox();

			GetCenterPoint(featureBox.pointA, featureBox.pointC);
		}

		// shifts the FeatureBox x and y values by the difference from the enclosing point 
		// e.g. top left corner of enclosing box is (100, 100) and this feature is (50, 50) within this box 
		// then its true corner is (150, 150)
		void ShiftFeatureBox(Rectangle enclosingBox)
		{
			// we just need to shift over by the amount of the enclosingBox's pointA
			featureBox.ShiftPoints(enclosingBox.pointA.x, enclosingBox.pointA.y);

			// create new centerpoint
			GetCenterPoint(featureBox.pointA, featureBox.pointC);
		}


		// crop grayScaleImage to the feature dimensions
		Mat CropFeature()
		{
			Rect cropDimensions(
				featureBox.pointA.x, featureBox.pointA.y,
				featureBox.w, featureBox.h
			);

			Mat croppedImage = grayScaleImage(cropDimensions);

			return croppedImage;
		}


		// gets the x and y centerpoint from two corner points of a feature
		void GetCenterPoint(My_Point ptA, My_Point ptC)
		{
			int avgX = (ptA.x & ptC.x) + ((ptA.x ^ ptC.x) >> 1);
			int avgY = (ptA.y & ptC.y) + ((ptA.y ^ ptC.y) >> 1);

			centerPoint = My_Point(avgX, avgY);
		}

	private:

		// create featureBox from Rectangle data type
		void CreateBox()
		{
			//									add offset for feature adjustment
			My_Point topLeftPt(feature[0].tl().x + offset, feature[0].tl().y);
			featureBox.CreateFrom_Point_Width_Height(topLeftPt, feature[0].width, feature[0].height);
		}


		// tries many values to find feature
		void FeatureDeepSearch()
		{
			for (const auto& scaleFactor : sFList) {
				for (const auto& minNeighbor : mNList) {
					featureCascade.detectMultiScale(grayScaleImage, feature, scaleFactor, minNeighbor);

					// if there is at least 1 feature detected
					if (!feature.empty())
					{
						if (feature.size() == 1)
						{
							return;
						}
					}


					// empty the feature before running again
					feature.clear();
				}
			}

			throw runtime_error("No Single Feature Found; Stopping Execution");
		}


		// searches for feature with given value
		// returns true if feature is found 
		bool FeatureSearch()
		{
			// check if that feature has an unsupported value
			if (scaleFactors[featureIDX] == -1 || minNeighbors[featureIDX] == -1)
			{
				return false;
			}

			float scaleFactor = scaleFactors[featureIDX];
			int minNeighbor = minNeighbors[featureIDX];

			featureCascade.detectMultiScale(grayScaleImage, feature, scaleFactor, minNeighbor);


			if (feature.size() == 1)
			{
				return true;
			}

			return false;
		}


		// if called, split gray scale image in half
		// need to adjust the x values then
		void HalveGrayScaleImage()
		{
			unsigned short int halfWidth = grayScaleImage.size().width >> 1;
			unsigned short int halfHeight = grayScaleImage.size().height >> 1;

			short int topLeftX = 0;
			// if we're using the right side, then the top left coord should be (halfWidth, 0)
			if (featureName != FeatureNames::LEFT_EYE)
			{
				topLeftX = halfWidth;

				// adjust the x values
				offset = halfWidth;
			}

			// x , y , w , h
			Rect halfImageDimensions (
				// regardless we are using the whole vertical length of the image
				topLeftX, 0,
				halfWidth, halfHeight
			);

			Mat halfImage = grayScaleImage(halfImageDimensions);

			// set grayScaleImage to halfImage
			grayScaleImage = halfImage;
		}


		// load cascade path into featureCascade
		void LoadCascadePath()
		{
			fs::path dir(haarcascadeDirectoryPath);
			fs::path file(cascades[featureIDX]);

			fs::path cascadePath_pth = file / dir;

			//string cascadePath = cascadePath_pth.string();

			string cascadePath = haarcascadeDirectoryPath + cascades[featureIDX];

			featureCascade.load(cascadePath);
		}


		// creates list of values to try
		void CreateLists(unsigned short int ListLength)
		{
			for (int i = 1; i <= ListLength; i++)
			{
				float sF = 1.0 + (i / 10.0);
				sFList.push_back(sF);
				mNList.push_back(i);
			}
		}
};


class ImageProcessor
{
	string sourceImagePath;

	string finalizedImagesDirectory;

	string saveFilePath;

	string name;

	Mat image_mat;

	unsigned short int sourceImageWidth;
	unsigned short int sourceImageHeight;

	Mat grayScaleImage;

	My_Point imageCenterPoint;

	public:
		ImageProcessor(string name_input, string sourceImage_input, string sourceImageDirectory_input, string finalizedImagesDirectory_input)
		{
			fs::path dir(sourceImageDirectory_input);
			fs::path file(sourceImage_input);
			fs::path sourceImagePath_pth = dir / file;
			sourceImagePath = sourceImagePath_pth.string();

			finalizedImagesDirectory = finalizedImagesDirectory_input;

			name = name_input;

			CreateSaveFilePath();
		}
	
		
		// performs main operation to center the face in the program
		void CenterFace() 
		{
			LoadImage();

			MakeGrayScaleImage();

			FeatureProcessor face(FeatureNames::FACE);

			face.CreateFeature(grayScaleImage);

			Mat croppedImage = face.CropFeature();
			
			
			FeatureProcessor leftEye(FeatureNames::LEFT_EYE);

			leftEye.CreateFeature(croppedImage);

			leftEye.ShiftFeatureBox(face.featureBox);

			
			FeatureProcessor rightEye(FeatureNames::RIGHT_EYE);

			rightEye.CreateFeature(croppedImage);

			rightEye.ShiftFeatureBox(face.featureBox);


			FeatureProcessor eyes(FeatureNames::EYES);

			eyes.GetCenterPoint(leftEye.centerPoint, rightEye.centerPoint);


			// gets horizontal offset between face and eyes
			int offset = getOffset(face.centerPoint, eyes.centerPoint);

			imageCenterPoint = face.centerPoint;

			SaveImage(offset);
		}


	private:
		// finds the smallest distance from imageCenterPoint to outsides
		int FindSmallestPadding()
		{
			int padding[4] = {
				imageCenterPoint.x,
				sourceImageWidth - imageCenterPoint.x,
				sourceImageHeight - imageCenterPoint.y,
				imageCenterPoint.y
			};

			// gets the min of padding
			int smallestPadding = padding[0];

			// start at one since we already set the first value which is padding[0]
			for (int i = 1; i < 4; i++)
			{
				int currentNumber = padding[i];

				if (currentNumber < smallestPadding)
				{
					smallestPadding = currentNumber;
				}
			}

			return smallestPadding;
		}


		// saves the image
		void SaveImage(int offset)
		{
			// adjust the image center point by the offset
			imageCenterPoint.x -= offset;

			int smallestPadding = FindSmallestPadding();

			// x , y , w , h
			Rect imageSaveDimensions
			(
				imageCenterPoint.x - smallestPadding, imageCenterPoint.y - smallestPadding,
				smallestPadding * 2, smallestPadding * 2
			);

			// create new image with image saveDimensions
			Mat saveImage = image_mat(imageSaveDimensions);

			// save filePath.string()
			imwrite(saveFilePath, saveImage);
		}


		// gets x offset between two points 
		// (used for finding difference between center point of face and eyes)
		int getOffset(My_Point ptA, My_Point ptB) 
		{
			int offset = ptA.x - ptB.x;
			return offset;
		}


		// converts image to grayscale
		void MakeGrayScaleImage()
		{
			cvtColor(image_mat, grayScaleImage, COLOR_BGR2GRAY);
		}


		// loads image into variable and gets width and height
		void LoadImage()
		{
			image_mat = imread(sourceImagePath);

			sourceImageWidth = image_mat.size().width;
			sourceImageHeight = image_mat.size().height;
		}


		// creates save file path 
		void CreateSaveFilePath()
		{
			replace(name.begin(), name.end(), ' ', '_'); // replace spaces in the name with '_'

			string saveFileName = "cropped_" + name + ".png";

			fs::path dir(finalizedImagesDirectory);
			fs::path file(saveFileName);

			fs::path saveFilePath_pth = dir / file;
			saveFilePath = saveFilePath_pth.string();
		}
};

class commandLineParser
{
	vector<string> argumentsVector = {
		"-name",
		"-imageName",
		"-sourceDir",
		"-finalDir"
	};

	vector <string> arguments;

	vector <string> values;

	vector <unsigned short int> argOrder;

	public:
		string name;
		string imageName;
		string sourceImageDirectory;
		string finalizedImageDirectory;

		commandLineParser(int argc, char** argv)
		{
			bool nextIsValue = false;
			for (int i = 1; i < argc; ++i)
			{
				auto pos = find(argumentsVector.begin(), argumentsVector.end(), argv[i]);

				// arg is in list
				if (pos != argumentsVector.end())
				{
					int idx = pos - argumentsVector.begin();
					argOrder.push_back(idx);
					arguments.push_back(argv[i]);
					nextIsValue = true;
				}
				else if (nextIsValue == true)
				{
					values.push_back(argv[i]);
					nextIsValue = false;
				}
			}

			// go through list of arguments and check if they are in the list of valid args
			for (int i = 0; i < arguments.size(); i++)
			{
				// check if particular argument is doesn't even exist
				if (find(argumentsVector.begin(), argumentsVector.end(), arguments[i]) == argumentsVector.end())
				{
					string errorMessage = "Argument: " + arguments[i] + " is not a valid argument for this program. These are valid arguments: \n";
					errorMessage += PrintValidArguments();

					cout << errorMessage;

					throw invalid_argument(errorMessage);
				}
			}

			// go through list of required arguments and check if those are in the argument list
			for (int i = 0; i < argumentsVector.size(); i++)
			{
				// check if particular argument is doesn't even exist
				if (find(arguments.begin(), arguments.end(), argumentsVector[i]) == arguments.end())
				{
					string errorMessage = "Argument: " + argumentsVector[i] + " is not found in the provided arguments for this program. These are required arguments: \n";
					errorMessage += PrintValidArguments();

					cout << errorMessage;

					throw std::invalid_argument(errorMessage);
				}
			}

			vector <string> commands;
			for (int i = 0; i < values.size(); i++)
			{
				auto pos = find(argOrder.begin(), argOrder.end(), i);
				int idx = pos - argOrder.begin();
				commands.push_back(values[idx]);
			}

			name = commands[0];
			imageName = commands[1];
			sourceImageDirectory = commands[2];
			finalizedImageDirectory = commands[3];
		}
				

		string PrintValidArguments() {
			string validArguments;
			for (int i = 0; i < argumentsVector.size(); i++)
			{
				validArguments += argumentsVector[i] + "\n";
			}

			return validArguments;
		}

};


int main(int argc, char** argv)
{
	// -name "john"
	commandLineParser cmdParser(argc, argv);
	
	ImageProcessor img_proc(cmdParser.name, cmdParser.imageName, cmdParser.sourceImageDirectory, cmdParser.finalizedImageDirectory);

	img_proc.CenterFace();

	return 0;
}
