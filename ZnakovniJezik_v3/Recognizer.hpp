#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame;
	std::vector<std::vector<cv::Point>> contours;
	bool started;
	int fps, nrObjects;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

