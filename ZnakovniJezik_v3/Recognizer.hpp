#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame, contourFrame;
	bool started;
	int fps, nrContours, nrObjects;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

