#pragma once

struct Letter
{
	std::string name;
	int votes;

	Letter(std::string n, int v)
	{
		name = n;
		votes = v;
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
	void updateLetters(std::string lName, int lVote);
	bool sortLetters(const Letter &lhs, const Letter &rhs);
	~Recognizer();
};
