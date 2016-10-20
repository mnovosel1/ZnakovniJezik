#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

Recognizer::Recognizer()
{
	fps = 0;
	nrContours = 0;
	nrObjects = 0;
	started = false;
	cropRect = cv::Rect(0, 0, 0, 0);
	hand = cv::Rect(0, 0, 0, 0);
	P1 = cv::Point(0, 0);
	P2 = cv::Point(0, 0);
	start();
}

void Recognizer::start()
{
	frame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	maskedFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	contouredFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	overlyFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	ROI = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
}

void Recognizer::stop()
{
	started = false;
}

Recognizer::~Recognizer()
{
}
