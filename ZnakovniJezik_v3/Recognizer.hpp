#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame, contouredFrame, contouredMaskedFrame, ROI;
	std::vector<std::vector<cv::Point>> contours;
	cv::Rect cropRect;
	bool started;
	int fps, nrObjects;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

