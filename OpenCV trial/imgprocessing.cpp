/*****************************************************************
 * imgprocessing.cpp
 *
 * Created: 10-04-2015 09:22:43
 *  Author: SHALAKA
 *****************************************************************/ 


#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include <iostream>
using namespace std;
using namespace cv;

Mat img;

Mat getRoute()
{
    //cvtColor(img,img,CV_BGR2HSV);

    Mat lowerb(img.rows, img.cols, CV_8UC3, Scalar(200,0,0));
    Mat upperb(img.rows, img,cols, CV_8UC3, Scalar(255,50,50));

    Mat blu  = (img<upperb) & (img>lowerb);

    return blu;
}

int main()
{
    img = imread("Capture.jpg");

    namedWindow("Map");
    imshow("Map", img);

    Mat newimg = getRoute();

    namedWindow("Route");
    imshow("Route", newimg);

    waitKey(0);
}
