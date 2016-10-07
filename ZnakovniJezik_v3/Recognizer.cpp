#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

Recognizer::Recognizer()
{
	fps = 0;
	nrObjects = 0;
	started = false;
	start();
}

void Recognizer::start()
{
	frame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	maskedFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	overlyFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
}

void Recognizer::stop()
{
	started = false;
}

Recognizer::~Recognizer()
{
}
