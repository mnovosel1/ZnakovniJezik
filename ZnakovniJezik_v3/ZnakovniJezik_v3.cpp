#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

using namespace cv;
using namespace std;

string infoText, oldinfoText, appNom = "Znakovni jezik v0.3.95";
string topText = "[ESC-izlaz] [O-overlay] [M-mask]";
bool started = false, overlayed = true, masked = false;
int ovrlyThick = 45;
double ovrlyAlpha = 0.3;
Scalar ovrlyColor = cv::Scalar(60, 60, 0);
Scalar txtColor = cv::Scalar(255, 255, 255);

Recognizer rc;
Mat displayFrame = imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);

mutex m;
thread strmThread, overlayThread, maskThread;

void _stream(Recognizer *obj);
void _overlay(Recognizer *obj);
void  _mask(Recognizer *obj);

void setInfo(string info);

int main(int, char**)
{
	strmThread = thread(_stream, &rc);
	overlayThread = thread(_overlay, &rc);
	maskThread = thread(_mask, &rc);

	setInfo("Startam..");
	Sleep(3000);

	imshow(appNom, displayFrame);

	setInfo("");
	do
	{
		m.lock();
			if (overlayed)
				rc.overlyFrame.copyTo(displayFrame);
			else if (masked)
				rc.maskedFrame.copyTo(displayFrame);
			else
				rc.frame.copyTo(displayFrame);
		m.unlock();

		imshow(appNom, displayFrame);

		switch (waitKey(10))
		{
			// ESC
			case 27:
				rc.stop();
				break;

			// O
			case 79:
			case 111:
				overlayed = !overlayed;
			break;

			// M
			case 77:
			case 109:
				masked = !masked;
			break;
		}

	} while (rc.started);	

	maskThread.join();
	overlayThread.join();
	strmThread.join();

	return 0;
}

void setInfo(string info)
{
	infoText = info;

	if ( infoText != oldinfoText )
		cout << infoText << endl;

	oldinfoText = infoText;
}

void _stream(Recognizer *obj)
{
	Mat frame;
	bool started = true;
	VideoCapture cap(0);

	m.lock();
		Sleep(2000);
		if (!cap.isOpened()) return;
		obj->started = true;
		obj->frame = imread("webcamfail.jpg", CV_LOAD_IMAGE_ANYCOLOR);
	m.unlock();

	setInfo("Streaming..");
	while (started)
	{
		cap.read(frame);

		if (!frame.empty())
		{
			m.lock();
				obj->frame = frame;
				started = obj->started;
			m.unlock();
		}
	}
	return;
}

void _overlay(Recognizer *obj)
{
	Mat frame, overlayFrame;
	bool started = true;
	int frameWidth;
	int frameHeight;
	
	Sleep(3000);

	m.lock();
		obj->frame.copyTo(frame);
	m.unlock();

	setInfo("Overlayed..");
	while (started)
	{
		frameWidth = frame.cols;
		frameHeight = frame.rows;

		frame.copyTo(overlayFrame);

		rectangle(overlayFrame, Point((0 - ovrlyThick * 2), (0 - ovrlyThick)), Point((frameWidth + ovrlyThick * 2), frameHeight), ovrlyColor, ovrlyThick * 3, 8);
		putText(overlayFrame, topText, Point(5, 15), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, infoText, Point(5, frameHeight - ovrlyThick / 2), FONT_HERSHEY_TRIPLEX, 1.7, txtColor, 1);
		addWeighted(frame, ovrlyAlpha, overlayFrame, 1 - ovrlyAlpha, 0, overlayFrame);

		m.lock();
			overlayFrame.copyTo(obj->overlyFrame);
			if (masked)
				obj->maskedFrame.copyTo(frame);
			else
				obj->frame.copyTo(frame);
			started = obj->started;
		m.unlock();
	}
	return;
}

void  _mask(Recognizer *obj)
{
	Mat frame, maskedFrame;
	bool started = true;

	Sleep(3000);

	m.lock();
		obj->frame.copyTo(frame);
	m.unlock();

	setInfo("Masked..");
	while (started)
	{
		cvtColor(frame, maskedFrame, COLOR_BGR2GRAY);
		GaussianBlur(maskedFrame, maskedFrame, Size(7, 7), 1.5, 1.5);
		Canny(maskedFrame, maskedFrame, 0, 30, 3);
		cvtColor(maskedFrame, maskedFrame, COLOR_GRAY2BGR);

		m.lock();
			maskedFrame.copyTo(obj->maskedFrame);
			obj->frame.copyTo(frame);
			started = obj->started;
		m.unlock();
	}
}