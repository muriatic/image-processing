#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <filesystem>
#include <tuple>
#include <set>
#include "rectangle.cpp"


namespace fs = std::filesystem;

using namespace std;
using namespace cv;


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
	float scaleFactors[3] = { 1.8f, 1.1f, 1.1f };
	int minNeighbors[3] = { 4, 10, 2 };


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
						/*cout << scaleFactor << endl;
						cout << minNeighbor << endl;*/
						return;
					}
				}


				// empty the feature before running again
				feature.clear();
			}
		}

		cout << "No Single Feature Found; Stopping Execution" << endl;
		throw exception("No Single Feature Found; Stopping Execution");
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

		short int topLeftX = 0;
		// if we're using the right side, then the top left coord should be (halfWidth, 0)
		if (featureName != FeatureNames::LEFT_EYE)
		{
			topLeftX = halfWidth;

			// adjust the x values
			offset = halfWidth;
		}

		// x , y , w , h
		Rect halfImageDimensions(
			// regardless we are using the whole vertical length of the image
			topLeftX, 0,
			halfWidth, grayScaleImage.size().height
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
	string saveFileName;

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

		//std::cout << "Image processed, saving..." << endl;

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

		if (image_mat.empty())
		{
			cout << "error: unable to read image" << endl;
			throw exception("error: unable to read image");
		}


		sourceImageWidth = image_mat.size().width;
		sourceImageHeight = image_mat.size().height;
	}


	// creates save file path 
	void CreateSaveFilePath()
	{
		replace(name.begin(), name.end(), ' ', '_'); // replace spaces in the name with '_'

		saveFileName = "cropped_" + name + ".png";

		fs::path dir(finalizedImagesDirectory);
		fs::path file(saveFileName);

		fs::path saveFilePath_pth = dir / file;
		saveFilePath = saveFilePath_pth.string();
	}
};