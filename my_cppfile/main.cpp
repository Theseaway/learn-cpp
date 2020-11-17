#include<opencv2\opencv.hpp>
#include<opencv2\calib3d\calib3d.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<stdio.h>
#include<iostream>
#include<vector>
using namespace std;
using namespace cv;
int CHECKBOARD[2]{ 6,7 };//6是图像中像素块的行数-1；9是图像中像素块的列数-1
int main() {
	vector<vector<Point3f>> obj_points;// Creating vector to store vectors of 3D points for each checkerboard image
	vector<vector<Point2f>> img_points;// Creating vector to store vectors of 2D points for each checkerboard image
	vector<Point3f> objp; // Defining the world coordinates for 3D points
	for (int i{ 0 }; i < CHECKBOARD[1]; i++) {
		for (int j{ 0 }; j < CHECKBOARD[0]; j++) {
			objp.push_back(Point3f(j, i, 0));
		}
	}
	vector<String> images;//路径
	string path = "C:\\Users\\****\\Pictures\\Camera Roll/*.jpg";
	glob(path, images);
	Mat frame, gray;
	vector<Point2f> corner_pts;
	bool success;
	for (int i{ 0 }; i < images.size(); i++) {
		frame = imread(images[i]);
		cvtColor(frame, gray, COLOR_BGR2GRAY);

		success = findChessboardCorners(gray, Size(CHECKBOARD[0], CHECKBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
		if (success) {
			TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);
			cornerSubPix(gray, corner_pts, Size(11, 11), Size(-1, -1), criteria);
			drawChessboardCorners(frame, Size(CHECKBOARD[0], CHECKBOARD[1]), corner_pts, success);
			obj_points.push_back(objp);
			img_points.push_back(corner_pts);
		}

		printf("Now %d th is processing\n", i);
	}

	Mat cameraMatrix, distCoeffs, R, T;
	calibrateCamera(obj_points, img_points, Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);
	cout << "cameraMatrix" << cameraMatrix << endl;
	cout << "distCoeffs :" << distCoeffs << endl;
	cout << "Rotation vector :" << R << endl;
	cout << "Translation vector :" << T << endl;


	// Code below is used for undistorting image which is obtained by the camera
	double matrix[3][3] = { { 267.1085443915335,        0,         318.4638534837586 },
							{ 0,         358.2230940457637, 255.2666235562296 },
							{ 0,                 0,                1 } };

	double distcoffes[5] = { -0.341127276068348, 0.0954028479124459, -0.004101616596085672, -0.000713512722151006, 0.0219552842269545 };
	Mat image = imread(images[0], 0);
	int cols = frame.cols;//列数，即长度
	int rows = frame.rows;//行数，即宽度
	double fx = matrix[0][0]; double fy = matrix[1][1];
	double cx = floor(matrix[0][2]); double cy = floor(matrix[1][2]);
	Mat image_undistort(rows, cols, CV_8UC1, Scalar(0, 0, 0));

	for (int i{ 0 }; i < 19; i++) {
		image = imread(images[i], 0);

		for (int j{ 0 }; j < rows; j++)
		{
			for (int k{ 0 }; k < cols; k++)
			{
				double x = double((k - cx)) / double(fx);
				double y = double((j - cy)) / double(fy);
				double r_2 = x*x + y*y;
				double x_d = x*(1 + distcoffes[0] * r_2 + distcoffes[1] * r_2*r_2);
				double y_d = y*(1 + distcoffes[0] * r_2 + distcoffes[1] * r_2*r_2);
				double k_distort = x_d*fx + cx;
				double j_distort = y_d*fy + cy;

				if (k_distort >= 0 && j_distort >= 0 && k_distort < cols && j_distort < rows)
				{
					int index_k = int(floor(k_distort));
					int index_j = int(floor(j_distort));
					try
					{
						image_undistort.at<uchar>(j, k) = image.at<uchar>(index_j, index_k);
					}
					catch (const std::exception&)
					{
						cout << "index_k and index_j is : " << index_k << "\t" << index_j << endl;
						cout << "   k    and    j    is : " << k << "\t" << j << endl;
						break;
					}
				}
				else
				{
					try
					{
						image_undistort.at<uchar>(j, k) = 0;
					}
					catch (const std::exception&)
					{
						break;
					}
				}
			}
		}
		String last_file_type = ".jpg";
		string temp = last_file_type;
		images[i].replace(60, images.size() - 60, temp);
		imwrite(images[i], image_undistort);
	}
	return 1;
}


/*
cameraMatrix
[267.1085443915335, 0, 318.4638534837586;
0, 358.2230940457637, 255.2666235562296;
0, 0, 1]
distCoeffs :[-0.341127276068348, 0.0954028479124459, -0.004101616596085672, -0.000713512722151006, 0.0219552842269545]


*/