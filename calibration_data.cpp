#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cmath>

// Defining the dimensions of checkerboard
// 定义棋盘格的尺寸

int CHECKERBOARD[2]={11,8};

struct FOV {
	double fovx_degree;
	double fovy_degree;
};

std::string getFileName(const std::string& filePath) {
    size_t found = filePath.find_last_of("/\\");
    if (found != std::string::npos) {
        return filePath.substr(found + 1);
    }
    return filePath;
}



cv::Point2d  calculateFOV(const cv::Mat cameraMatrix){
	// 计算FOV
	double w = cameraMatrix.at<double>(0, 2)*2;
	double h = cameraMatrix.at<double>(1, 2)*2;

	double fx = cameraMatrix.at<double>(0, 0);
	double fy = cameraMatrix.at<double>(1, 1);

	double fovx = 2*std::atan(w/(2*fx));
	double fovy = 2*std::atan(h/(2*fy));

	double fovx_degree = fovx * 180 /M_PI;
	double fovy_degree = fovy * 180 /M_PI;

	std::cout<<"fovx_degree: "<<fovx_degree<<"fovy_degree: "<<fovy_degree<<std::endl;
	cv::Point2d fov(fovx, fovy);
	
	return fov;

}



int main()
{

   // Creating vector to store vectors of 3D points for each checkerboard image
   // 创建矢量以存储每个棋盘图像的三维点矢量
    std::vector<std::vector<cv::Point3f> > objpoints;

    // Creating vector to store vectors of 2D points for each checkerboard image
	// 创建矢量以存储每个棋盘图像的二维点矢量
	std::vector<std::vector<cv::Point2f> > imgpoints;

    // Defining the world coordinates for 3D points
	// 为三维点定义世界坐标系
	std::vector<cv::Point3f> objp;
	for (int i{ 0 }; i < CHECKERBOARD[1]; i++)
	{
		for (int j{ 0 }; j < CHECKERBOARD[0]; j++)
		{
			objp.push_back(cv::Point3f(j, i, 0));
		}
	}

    // Extracting path of individual image stored in a given directory
	// 提取存储在给定目录中的单个图像的路径
	std::vector<cv::String> images;
	cv::String display_dir = "./corner_samples";
	// Path of the folder containing checkerboard images
	// 包含棋盘图像的文件夹的路径
	std::string path = "../highquality/*.png";

	// 使用glob函数读取所有图像的路径
	cv::glob(path, images);
    cv::Mat frame, gray;

	// vector to store the pixel coordinates of detected checker board corners
	// 存储检测到的棋盘转角像素坐标的矢量
	std::vector<cv::Point2f> corner_pts;
	bool success;

	// Looping over all the images in the directory
	// 循环读取图像
    for(int i=0; i<images.size();i++)
    {
        frame = cv::imread(images[i]);
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);


        // Finding checker board corners
		// 寻找角点
		// If desired number of corners are found in the image then success = true
		// 如果在图像中找到所需数量的角，则success = true
		// opencv4以下版本，flag参数为CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE
        
        success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

		// 如果检测到所需数量的角点，我们将细化像素坐标并将其显示在棋盘图像上


		if (success)
		{
			
			cv::TermCriteria criteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.001);

			//refining pixel coordinates for given 2d points
			//为给定的二维点细化像素坐标
			cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// 在棋盘上显示检测到的角点
			cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

			objpoints.push_back(objp);
			imgpoints.push_back(corner_pts);

			
			cv::String filename = getFileName(images[i]);
			std::cout<<filename<<std::endl;
			cv::imwrite("corner_samples/"+filename, frame);

		}	    

    }

	cv::Mat cameraMatrix, distCoeffs, R, T;
	/*
	 * Performing camera calibration by
	 * passing the value of known 3D points (objpoints)
	 * and corresponding pixel coordinates of the
	 * detected corners (imgpoints)
	*/
	// 通过传递已知3D点（objpoints）的值和检测到的角点（imgpoints）的相应像素坐标来执行相机校准
	cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

	// 内参矩阵
	std::cout << "cameraMatrix : " << cameraMatrix << std::endl;

	// 透镜畸变系数
	std::cout << "distCoeffs : " << distCoeffs << std::endl;


	// 相机矫正

	// const int ImgWidth = 640;
    //     const int ImgHeight = 480;

	// cv::Mat map1, map2;
	// cv::Size imageSize(ImgWidth, ImgHeight);
	// cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), cameraMatrix, imageSize, CV_16SC2, map1, map2);


	// 相机FOV 计算

	cv::Point2d fov;
    fov = calculateFOV(cameraMatrix); 
	cv::FileStorage fs("camera1.yml", cv::FileStorage::WRITE);

	if(fs.isOpened()){
		fs<<"camera coeffs"<<distCoeffs;
		fs<<"camera matrix"<<cameraMatrix;
		fs<<"camera FOV"<<fov;
	}
	else{
		std::cerr<<"save camera parameter error"<<std::endl;
	}

    return 0;
}
