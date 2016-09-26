#pragma once
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"

struct Hand
{
	cv::Point center;
	cv::Size size;
};

class Video
{
public:
	Video();
	void setInfo(cv::string);
	cv::Mat overlay();
	void detect();
	void setBcgFrame(cv::Mat);
	void unSetBcgFrame();
	cv::Mat GetSkin();
	void contours(double thresh);
	~Video();

	cv::Mat frame;
	std::vector<Hand> hands;
	cv::CascadeClassifier cascade;
	int vWidth, vHeight, frmWidth, frameCount, thresh;
	double frmAlpha, fps;
	clock_t start;
	cv::Scalar frmColor, txtColor, detColor;
	cv::string tlText, fpsText, btmText, haarXML;
	bool bcgSet, masked, overlayed, started;

private:
	cv::Mat bcgFrame;
	cv::BackgroundSubtractorMOG2 bcgSubtractor;
	bool R1(int, int, int);
	bool R2(float, float, float);
	bool R3(float, float, float);
	cv::RNG rng;
};

