#pragma once

struct Letter
{
	std::string name, xmlName;
	int votes;

	Letter(std::string& xn, std::string& n, int v) : xmlName(xn), name(n), votes(v) {};

	bool operator < (const Letter& ltr) const
	{
		return votes > ltr.votes;
	}
};

class Recognizer
{
public:
	cv::Mat frame, overlyFrame, maskedFrame, contouredFrame, contouredMaskedFrame, ROI;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<Letter> letters;
	cv::Rect cropRect, hand;
	cv::Point P1, P2;
	bool started, recognizeOn;
	int fps, nrContours, nrObjects, recCounter;
	std::string slovo;

public:
	Recognizer();
	void start();
	void stop();
	void updateLetters();
	void updateLetters(std::string xmlName, std::string lName, int lVote);
	~Recognizer();
};
