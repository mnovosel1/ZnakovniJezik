#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

using namespace cv;
using namespace std;

string infoText, oldinfoText, appNom = "Znakovni jezik v0.3.95";
string topText = "[ESC-izlaz] [O-overlay] [M-mask]", fpsText = "fps: /";
bool started = false, overlayed = true, masked = false;
int ovrlyThick = 45;
double fps, ovrlyAlpha = 0.5;
Scalar ovrlyColor = cv::Scalar(60, 60, 0);
Scalar txtColor = cv::Scalar(255, 255, 255);

Recognizer rc;
Mat displayFrame = imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);

mutex m;
thread strmThread, overlayThread, maskThread, recognizeThread;

void _stream(Recognizer *obj);
void _overlay(Recognizer *obj);
void  _mask(Recognizer *obj);
void  _recognize(Recognizer *obj);

void setInfo(string info);

int main(int, char**)
{
	strmThread = thread(_stream, &rc);
	Sleep(500);

	do 	{
		Sleep(50);
		m.lock();
			started = rc.started;
		m.unlock();
	} while (!started);
	setInfo("Streamer pokrenut..");

	Sleep(200);
	maskThread = thread(_mask, &rc);
	setInfo("Masker pokrenut..");

	Sleep(200);
	recognizeThread = thread(_recognize, &rc);
	setInfo("Recognizer pokrenut..");

	Sleep(400);
	overlayThread = thread(_overlay, &rc);
	setInfo("Overlayer pokrenut..");

	Sleep(1000);
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

		switch (waitKey(5))
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

	destroyAllWindows();

	setInfo("Gasim recognizer..");
	recognizeThread.join();
	Sleep(100);

	setInfo("Gasim masker..");
	maskThread.join();
	Sleep(100);

	setInfo("Gasim overlayer..");
	overlayThread.join();
	Sleep(100);

	setInfo("Gasim streamer..");
	strmThread.join();
	Sleep(100);


	setInfo("Gasim se..");
	Sleep(500);

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
		if (!cap.isOpened()) return;
		obj->frame = imread("webcamfail.jpg", CV_LOAD_IMAGE_ANYCOLOR);
		obj->started = true;
	m.unlock();

	Sleep(3000);

	while (started)
	{
		cap.read(frame);

		if (!frame.empty())
		{
			m.lock();
				frame.copyTo(obj->frame);
				started = obj->started;				
			m.unlock();
		}
	}
	return;
}

void _overlay(Recognizer *obj)
{
	Mat frame, overlayFrame;
	bool started = false;
	int frameWidth;
	int frameHeight;

	m.lock();
		obj->frame.copyTo(frame);
		started = obj->started;
	m.unlock();

	Sleep(4000);

	while (started)
	{
		frameWidth = frame.cols;
		frameHeight = frame.rows;

		frame.copyTo(overlayFrame);

		rectangle(overlayFrame, Point((0 - ovrlyThick * 2), (0 - ovrlyThick)), Point((frameWidth + ovrlyThick * 2), frameHeight), ovrlyColor, ovrlyThick * 3, 8);
		addWeighted(frame, ovrlyAlpha, overlayFrame, 1 - ovrlyAlpha, 0, overlayFrame);
		putText(overlayFrame, topText, Point(5, 15), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, fpsText, Point(frameWidth - 70, ovrlyThick / 3), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, infoText, Point(5, frameHeight - ovrlyThick / 2), FONT_HERSHEY_TRIPLEX, 1.7, txtColor, 1);

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
	Mat frame, frameHSV, maskedFrame;

	bool started = false;
	double frameCount = 0;
	clock_t start = clock();

	Vec3b cwhite = Vec3b::all(255);
	Vec3b cblack = Vec3b::all(0);

	m.lock();
		obj->frame.copyTo(frame);
		started = obj->started;
	m.unlock();


	while (started)
	{
		maskedFrame = frame.clone();
		GaussianBlur(maskedFrame, maskedFrame, Size(7, 7), 1.5, 1.5);

		maskedFrame.convertTo(frameHSV, CV_32FC3);
		cvtColor(frameHSV, frameHSV, CV_BGR2HSV);
		normalize(frameHSV, frameHSV, 0.0, 255.0, NORM_MINMAX, CV_32FC3);

		for (int i = 0; i < frame.rows; i++) {
			for (int j = 0; j < frame.cols; j++) {
				Vec3f pix_hsv = frameHSV.ptr<Vec3f>(i)[j];

				if ((pix_hsv.val[0] < 30) || (pix_hsv.val[0] > 200))
					maskedFrame.ptr<Vec3b>(i)[j] = cwhite;
				else
					maskedFrame.ptr<Vec3b>(i)[j] = cblack;
			}
		}

		frame.copyTo(maskedFrame, maskedFrame);
		//cvtColor(maskedFrame, maskedFrame, COLOR_BGR2GRAY);
		//Canny(maskedFrame, maskedFrame, 0, 30, 3);
		//cvtColor(maskedFrame, maskedFrame, COLOR_GRAY2BGR);

		m.lock();
			maskedFrame.copyTo(obj->maskedFrame);
			frame = obj->frame.clone();
			started = obj->started;
		m.unlock();

		frameCount += 1000;
		if (frameCount > 10000)
		{
			fps = (double)frameCount / (double)((clock() - start));
			fpsText = "fps: " + to_string((int)fps);
			frameCount = 0;
			start = clock();
		}
	}
	return;
}

void  _recognize(Recognizer *obj)
{
	Mat frame;
	string haarXML, haarName;
	CascadeClassifier cascade;
	vector<Rect> hands;

	bool started = false;

	WIN32_FIND_DATA data;
	HANDLE hFind;

	Sleep(5000);

	do {
		m.lock();
			obj->maskedFrame.copyTo(frame);
			started = obj->started;
		m.unlock();

		hFind = FindFirstFile("haarcascades/*.*", &data);
		do {
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				haarXML = data.cFileName;
				size_t lastindex = haarXML.find_last_of(".");
				haarName = haarXML.substr(0, lastindex);

				cascade.load("haarcascades/" + haarXML);
				cascade.detectMultiScale(frame, hands, 1.2, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(100, 100), Size(300, 300));

				if (hands.size() > 0)
				{
					setInfo( to_string(hands.size()) + " : " + haarName);
				}
			}
		} while (FindNextFile(hFind, &data));
	} while (started);

	FindClose(hFind);
	return;
}