#include <process.h>
#include <iostream>
#include <string>
#include <ctime>
#include <windows.h>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "video.h"

using namespace cv;
using namespace std;

string winName = "Znakovni jezik v.93";
HANDLE hContoursThread, hDetectThread, hStreamThread;
Video video;

void _contours(void *param)
{
	return;
	video.contours(30);
}

void _detect(void *param)
{
	Hand hand;
	vector<Rect> hands;
	int handX, handY, handW, handH;
	double scaleFactor = 1;
	CascadeClassifier cascade;
	size_t ind;
	string haarName, lastHaar;
	Mat frameGray;
	
	Sleep(1500);
	video.setInfo("Contours..");

	while (video.started)
	{
		ind = video.haarXML.find_last_of(".");
		haarName = video.haarXML.substr(0, ind);

		cvtColor(video.GetSkin(), frameGray, CV_BGR2GRAY);
		equalizeHist(frameGray, frameGray);
		resize(frameGray, frameGray, Size((int)(video.vWidth / scaleFactor), (int)(video.vHeight / scaleFactor)));

		if (video.haarXML != "" && haarName != lastHaar)
		{	
			cascade.load("haarcascades/" + video.haarXML);
			cascade.detectMultiScale(frameGray, hands, 1.1, 1, 0 | CV_HAAR_SCALE_IMAGE, Size(100, 100));

			for (size_t i = 0; i < hands.size(); i++)
			{
				handX = (int)(hands[i].x*scaleFactor);
				handY = (int)(hands[i].y*scaleFactor);
				handW = (int)(hands[i].width*scaleFactor);
				handH = (int)(hands[i].height*scaleFactor);

				hand.center = Point(handX + handW*0.5, handY + handH*0.5);
				hand.size = Size(handW*0.5, handH*0.5);
				video.hands.push_back(hand);
				if (video.hands.size() > 2) video.hands.erase(video.hands.begin());

				video.setInfo(haarName);
			}

			lastHaar = haarName;
			Sleep(125);
		}

	}
	video.setInfo("Adijo!");
	return;
}

void _stream(void *param)
{
	int key;
	Mat bgFrame;
	VideoCapture cap(0);
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	//createTrackbar("", winName, &video.thresh, 255, NULL);

	if (cap.isOpened())
	{
		video.started = true;
		hDetectThread = (HANDLE)_beginthread(_detect, 0, NULL);
		hContoursThread = (HANDLE)_beginthread(_contours, 0, NULL);

		while (cap.read(video.frame))
		{
			key = waitKey(5);
			if (key == 27) // ESCape
			{
				video.started = false;
				return;
			}
			else if (key == 109) video.masked = !video.masked; // M
			else if (key == 111) video.overlayed = !video.overlayed; // O
			/*
			else if (key == 112) // P
			{
				video.frame.copyTo(bgFrame);
				video.setBcgFrame(bgFrame);
			}
			else if (key == 114)  // R
			{
				video.unSetBcgFrame();
				destroyAllWindows();
			}
			video.setInfo(to_string(key));
			*/

			video.frameCount += 1000;
			imshow(winName, video.overlay());
		}
	}
	else return;
}

int main(int argc, char* argv[])
{
	hStreamThread = (HANDLE)_beginthread(_stream, 0, NULL);
	while (!video.started) Sleep(1500);

	HANDLE hFind;
	WIN32_FIND_DATA data;	

	while (video.started)
	{
		hFind = FindFirstFile("haarcascades/*.*", &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					video.haarXML = data.cFileName;
					Sleep(20);
				}
			} while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
	}

	WaitForSingleObject(hStreamThread, INFINITE);
	WaitForSingleObject(hDetectThread, INFINITE);

	return 0;
}