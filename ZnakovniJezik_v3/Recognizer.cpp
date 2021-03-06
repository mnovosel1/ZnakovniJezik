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
	currSlovo = "";
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

void Recognizer::updateLetters()
{
	int lVote = 5;

	for (std::vector<Letter>::size_type i = 0; i != letters.size(); i++)
	{
		letters[i].votes = (letters[i].votes - lVote) < 0 ? 0 : (letters[i].votes - lVote) > 99 ? 99 : letters[i].votes - lVote;
	}
}

void Recognizer::updateLetters(std::string xmlName, std::string lName, int lVote)
{
	bool ltrExist = false;

	for (std::vector<Letter>::size_type i = 0; i != letters.size(); i++)
	{
		if (letters[i].name == lName)
		{
			ltrExist = true;
			currSlovo = lName;
			lVote = lVote < 75 ? lVote : pow(1 + lVote, letters[i].votes/10);
			letters[i].votes = (letters[i].votes + lVote) < 0 ? 0 : (letters[i].votes + lVote) > 99 ? 99 : letters[i].votes + lVote;
			break;
		}		
	}

	if (!ltrExist)
	{
		letters.push_back(Letter(xmlName, lName, lVote));
	}

	std::sort(letters.begin(), letters.end());

	if (letters[0].votes > 50)
		slovo = letters[0].name;
}
