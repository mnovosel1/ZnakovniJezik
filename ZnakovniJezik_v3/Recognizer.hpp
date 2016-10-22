#pragma once
class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame, contouredFrame, contouredMaskedFrame, ROI;
	std::vector<std::vector<cv::Point>> contours;
	cv::Rect cropRect, hand;
	cv::Point P1, P2;
	bool started, recognizeOn;
	int fps, nrContours, nrObjects, recCounter;
	std::string slovo;

public:
	Recognizer();
	void start();
	void stop();
	~Recognizer();
};

