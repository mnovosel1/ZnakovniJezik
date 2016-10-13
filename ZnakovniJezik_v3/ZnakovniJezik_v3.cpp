#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

using namespace cv;
using namespace std;

string infoText, oldinfoText, appNom = "Znakovni jezik v0.3.98";
string slikaIme, topText = "[ESC-izlaz] [O-overlay] [M-mask] [P-postavke] [C-capture]";

bool started = false, overlayed = true, masked = false, postavke = false;

int brSlike = 1000000, ovrlyThick = 45, contourThresh = 20, minContourArea = 20000, maxNrContours = 1, minHsv = 40, maxHsv = 180, blurKernel = 15;
double ovrlyAlpha = 0.5;
Scalar ovrlyColor = Scalar(60, 60, 0);
Scalar txtColor = Scalar(255, 255, 255);
Scalar contourColor = Scalar(0, 255, 150);

Recognizer rc;
Mat displayFrame = imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR);

mutex m;
thread strmThread, maskThread, overlayThread, contourThread, recognizeThread;

void _stream(Recognizer *obj);
void _mask(Recognizer *obj);
void _overlay(Recognizer *obj);
void _contours(Recognizer *obj);
void _recognize(Recognizer *obj);

string ExePath();
void setInfo(string info, int what = 0);

int main(int, char**)
{
	strmThread = thread(_stream, &rc);
	setInfo("\n", 1);
	Sleep(400);

	do 	{
		Sleep(50);
		m.lock();
			started = rc.started;
		m.unlock();
	} while (!started);
	setInfo("Streamer pokrenut..\n", 1);

	Sleep(200);
	maskThread = thread(_mask, &rc);
	setInfo("Masker pokrenut..\n", 1);

	Sleep(300);
	overlayThread = thread(_overlay, &rc);
	setInfo("Overlayer pokrenut..\n", 1);

	Sleep(400);
	contourThread = thread(_contours, &rc);
	setInfo("Contourer pokrenut..\n", 1);

	Sleep(500);
	recognizeThread = thread(_recognize, &rc);
	setInfo("Recognizer pokrenut..\n", 1);

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
				rc.contouredFrame.copyTo(displayFrame);
		m.unlock();

		imshow(appNom, displayFrame);

		switch (waitKey(5))
		{
			// ESC
			case 27:				
				rc.stop();
			break;

			// C capture
			case 67:
			case 99:
				slikaIme = to_string(++brSlike);
				slikaIme = slikaIme.substr(1, 6);
				slikaIme = ExePath() + "\\img\\" + slikaIme + ".jpg";

				imwrite(slikaIme, rc.frame);

				setInfo(slikaIme + "\n", 1);
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

			// P
			case 80:
			case 112:
				postavke = !postavke;

				if (postavke)
				{
					namedWindow("Postavke", CV_WINDOW_AUTOSIZE);
					createTrackbar("Blur", "Postavke", &blurKernel, 150, NULL);
					createTrackbar("Hsv min.", "Postavke", &minHsv, 255, NULL);
					createTrackbar("Hsv max.", "Postavke", &maxHsv, 255, NULL);
					createTrackbar("C. thr.", "Postavke", &contourThresh, 200, NULL);
					createTrackbar("C. min. ar.", "Postavke", &minContourArea, 50000, NULL);
					createTrackbar("C. min.", "Postavke", &maxNrContours, 20, NULL);
				}
				else
				{
					destroyWindow("Postavke");
				}

			break;
		}


	} while (rc.started);	

	setInfo("\n\n\n");
	destroyAllWindows();

	setInfo("Gasim recognizer..\n", 1);
	recognizeThread.join();
	Sleep(100);

	setInfo("Gasim contourer..\n", 1);
	contourThread.join();
	Sleep(100);

	setInfo("Gasim overlayer..\n", 1);
	overlayThread.join();
	Sleep(100);

	setInfo("Gasim masker..\n", 1);
	maskThread.join();
	Sleep(100);

	setInfo("Gasim streamer..\n", 1);
	strmThread.join();
	Sleep(100);

	return EXIT_SUCCESS;
}

void setInfo(string info, int what)
{

	if (what == 0 || what == 1)
	{
		if (info != "" && info != oldinfoText)
			cout << info;
	}

	if (what == 0 || what == 2)
	{
		infoText = info;
	}

	oldinfoText = info;
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
				frame.copyTo(obj->contouredFrame);
				started = obj->started;				
			m.unlock();
		}
	}
	cap.release();
	return;
}

void  _mask(Recognizer *obj)
{
	Mat frame, frameHSV, maskedFrame;

	bool started = false;
	double frameCount = 0;
	clock_t start = clock();
	int fps = 0;

	vector<vector<Point>> contours;

	Vec3b cwhite = Vec3b::all(255);
	Vec3b cblack = Vec3b::all(0);

	m.lock();
		obj->frame.copyTo(frame);
		contours = obj->contours;
		started = obj->started;
	m.unlock();


	while (started)
	{
		frame.copyTo(maskedFrame);
		//GaussianBlur(maskedFrame, maskedFrame, Size(blurKernel, blurKernel), 0, 0);
		blurKernel = blurKernel < 2 ? 2 : blurKernel;
		blur(maskedFrame, maskedFrame, Size(blurKernel, blurKernel));

		maskedFrame.convertTo(frameHSV, CV_32FC3);
		cvtColor(frameHSV, frameHSV, CV_BGR2HSV);
		normalize(frameHSV, frameHSV, 0.0, 255.0, NORM_MINMAX, CV_32FC3);

		for (int i = 0; i < frame.rows; i++) {
			for (int j = 0; j < frame.cols; j++) {
				Vec3f pix_hsv = frameHSV.ptr<Vec3f>(i)[j];

				if ((pix_hsv.val[0] < minHsv) || (pix_hsv.val[0] > maxHsv))
					maskedFrame.ptr<Vec3b>(i)[j] = cwhite;
				else
					maskedFrame.ptr<Vec3b>(i)[j] = cblack;

			}
		}

		frame.copyTo(maskedFrame, maskedFrame);
		/*
		for (int i = 0; i < contours.size(); i++)
		{
			drawContours(maskedFrame, contours, i, contourColor);
		}
		*/
		//Canny(maskedFrame, maskedFrame, 0, 30, 3);
		//cvtColor(subtractedFrame, subtractedFrame, COLOR_GRAY2BGR);

		frameCount += 1000;
		if (frameCount > 10000)
		{
			fps = (double)frameCount / (double)((clock() - start));
			frameCount = 0;
			start = clock();
		}

		m.lock();
			maskedFrame.copyTo(obj->maskedFrame);
			maskedFrame.copyTo(obj->contouredMaskedFrame);
			obj->frame.copyTo(frame);
			obj->fps = fps;
			contours = obj->contours;
			started = obj->started;
		m.unlock();
	}
	return;
}

void _overlay(Recognizer *obj)
{
	Mat frame, overlayFrame;
	bool started = false;
	int frameWidth, frameHeight, fps = 0, nrObjects = 0, nrContours = 0;

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
		putText(overlayFrame, "fps : " + to_string(fps), Point(frameWidth - 75, ovrlyThick / 3), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, "objekata : " + to_string(nrObjects), Point(frameWidth - 105, frameHeight - 50), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, "kontura : " + to_string(nrContours), Point(frameWidth - 100, frameHeight - 35), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, infoText, Point(5, frameHeight - ovrlyThick / 2), FONT_HERSHEY_TRIPLEX, 1.7, txtColor, 1);

		m.lock();
			overlayFrame.copyTo(obj->overlyFrame);

			if (masked)
				obj->contouredMaskedFrame.copyTo(frame);
			else
				obj->contouredFrame.copyTo(frame);

			started = obj->started;
			nrObjects = obj->nrObjects;
			nrContours = obj->contours.size();
			fps = obj->fps;
		m.unlock();
	}
	return;
}

void _contours(Recognizer *obj)
{
	Mat frame, canny_output;
	vector<vector<Point>> contours, bigContours;
	vector<Vec4i> hierarchy;

	bool started = true;

	m.lock();
		obj->maskedFrame.copyTo(frame);
	m.unlock();

	do
	{
		cvtColor(frame, frame, COLOR_BGR2GRAY);
		threshold(frame, canny_output, contourThresh, 50, 0);

		//Canny(canny_output, canny_output, contourThresh, contourThresh * 2, 3);
		findContours(canny_output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		bigContours.clear();
		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) > minContourArea)
			{
				//bigContours.push_back(contours[i]);
				m.lock();
					drawContours(obj->contouredMaskedFrame, contours, i, contourColor);
					drawContours(obj->contouredFrame, contours, i, contourColor);
				m.unlock();
			}
		}

		m.lock();
			obj->maskedFrame.copyTo(frame);
			started = obj->started;
		m.unlock();

	} while (started);
	return;
}

void  _recognize(Recognizer *obj)
{
	Mat frame;
	string haarXML, haarName;
	CascadeClassifier cascade;
	vector<Rect> hands;
	int nrObjects = 0;

	bool started = false;

	WIN32_FIND_DATA data;
	HANDLE hFind;

	Sleep(5000);

	do {
		nrObjects = 0;

		m.lock();
			obj->maskedFrame.copyTo(frame);
			started = obj->started;
			obj->nrObjects = nrObjects;
		m.unlock();

		hFind = FindFirstFile("haarcascades/*.*", &data);
		do {
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				haarXML = data.cFileName;
				size_t lastindex = haarXML.find_last_of(".");
				haarName = haarXML.substr(0, lastindex);

				cascade.load("haarcascades/" + haarXML);
				cascade.detectMultiScale(frame, hands, 1.2, 3, 0 | CV_HAAR_SCALE_IMAGE, Size(150, 150), Size(700, 700));

				if (hands.size() > 0)
				{
					nrObjects++;
					setInfo(haarName + " ");
					Sleep(50);
				}
			}
		} while (FindNextFile(hFind, &data));

		Sleep(2000);
		setInfo("");

	} while (started);

	FindClose(hFind);
	return;
}

string ExePath()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}