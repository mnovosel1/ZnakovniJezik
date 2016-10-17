#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame, contouredFrame, contouredMaskedFrame, ROI;
	std::vector<std::vector<cv::Point>> contours;
	cv::Rect cropRect;
	cv::Point P1, P2;
	bool started, recognizeOn;
	int fps, nrObjects;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

