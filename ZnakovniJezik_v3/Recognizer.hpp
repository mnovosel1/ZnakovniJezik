#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame;
	bool started;
	int ovrlyThick;
	double ovrlyAlpha;
	cv::Scalar ovrlyColor, txtColor;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

