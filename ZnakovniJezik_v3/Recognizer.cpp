#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

Recognizer::Recognizer()
{
	fps = 0;
	nrContours = 0;
	nrObjects = 0;
	started = false;
	cropRect = cv::Rect(0, 0, 0, 0);
	hand = cv::Rect(0, 0, 0, 0);
	P1 = cv::Point(0, 0);
	P2 = cv::Point(0, 0);
	slovo = "";
	recCounter = 0;
	start();
}

void Recognizer::start()
{
	frame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	maskedFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	contouredFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	overlyFrame = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	ROI = cv::imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);
}

void Recognizer::stop()
{
	started = false;
}

Recognizer::~Recognizer()
{
}

void Recognizer::updateLetters(std::string lName, int lVote)
{
	bool ltrExist = false;

	for (std::vector<Letter>::size_type i = 0; i != letters.size(); i++)
	{
		if (letters[i].name == lName)
		{
			ltrExist = true;
			letters[i].votes = (letters[i].votes + lVote) < 0 ? 0 : letters[i].votes + lVote;
		}
	}

	if (!ltrExist)
	{
		letters.push_back(Letter(lName, lVote));
	}

	//sort(letters.begin(), letters.end(), &Recognizer::sortLetters);
}

bool Recognizer::sortLetters(const Letter &lhs, const Letter &rhs)
{
	return lhs.votes > rhs.votes;
}
