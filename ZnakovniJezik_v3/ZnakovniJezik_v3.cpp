#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "Recognizer.hpp"

using namespace cv;
using namespace std;

const char* appNom = "Znakovni jezik v1.0.153";

string infoText, oldinfoText;
string slikaIme, topText = "[ESC-izlaz]  [P-postavke]  [C-slikaj]";

bool started = false, overlayed = true, masked = false, postavke = false, clicked = false, capture = false;

unsigned long capTime = clock();

int voteStep = 15, voteHyst = 3, brSlike = 1000000, ovrlyThick = 45, contourThresh = 10, minContourArea = 10000, maxNrContours = 1, minHsv = 60, maxHsv = 240, blurKernel = 10, brLetersa=4;
double ovrlyAlpha = 0.6;
Scalar ovrlyColor = Scalar(60, 60, 0);
Scalar txtColor = Scalar(255, 255, 255);
Scalar contourColor = Scalar(0, 255, 150);

Recognizer rc;
Mat displayFrame = imread("starting.jpg", CV_LOAD_IMAGE_ANYCOLOR), saveFrame, ROIframe;

mutex m;
thread strmThread, maskThread, overlayThread, contourThread, recognizeThread;

void _stream(Recognizer *obj);
void _mask(Recognizer *obj);
void _overlay(Recognizer *obj);
void _contours(Recognizer *obj);
void _recognize(Recognizer *obj);

void onMouse(int event, int x, int y, int f, void*);

void setInfo(string info, int what = 0);

int main(int, char**)
{
	strmThread = thread(_stream, &rc);

	setInfo("\n", 1);
	Sleep(400);

	do 	
	{
		Sleep(500);

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
			{
				rc.overlyFrame.copyTo(displayFrame);
			}
			else
				rc.frame.copyTo(displayFrame);

			rc.maskedFrame.copyTo(ROIframe);
		m.unlock();

		imshow(appNom, displayFrame);
		//imshow("ROI", ROIframe);
		setMouseCallback(appNom, onMouse, NULL);

		if ( capture && ((unsigned long)clock() - capTime) >= 1500)
		{
			slikaIme = to_string(++brSlike);
			slikaIme = slikaIme.substr(1, 6);
			slikaIme = "img\\" + slikaIme + ".jpg";

			m.lock();
			saveFrame = rc.maskedFrame;
			m.unlock();

			//resize(saveFrame, saveFrame, Size((int)saveFrame.cols/8, (int)saveFrame.rows/8));

			cvtColor(saveFrame, saveFrame, CV_BGR2GRAY);
			imwrite(slikaIme, saveFrame, vector<int>({ CV_IMWRITE_JPEG_QUALITY, 100 }));

			setInfo(slikaIme + "\n", 1);

			capTime = clock();
		}

		switch (waitKey(5))
		{
			// ESC
			case 27:				
				rc.stop();
			break;

			// C capture
			case 67:
			case 99:				
				capture = !capture;

				if (capture)
					topText = "[ESC-izlaz]  [P-postavke]  [C-ne slikaj]";
				else
					topText = "[ESC-izlaz]  [P-postavke]  [C-slikaj]";
			break;

			// O
			//case 79:
			//case 111:
				//overlayed = !overlayed;
			//break;

			// P
			case 80:
			case 112:
				postavke = !postavke;

				if (postavke)
				{
					namedWindow("Postavke", CV_WINDOW_AUTOSIZE);
					createTrackbar("Blur", "Postavke", &blurKernel, 150, NULL);
					createTrackbar("Hmin.", "Postavke", &minHsv, 255, NULL);
					createTrackbar("Hmax.", "Postavke", &maxHsv, 255, NULL);
					createTrackbar("Vstep.", "Postavke", &voteStep, 40, NULL);
					createTrackbar("Vhyst.", "Postavke", &voteHyst, 30, NULL);
					createTrackbar("Cthr.", "Postavke", &contourThresh, 200, NULL);
					createTrackbar("Carea", "Postavke", &minContourArea, 50000, NULL);
					createTrackbar("Cmin.", "Postavke", &maxNrContours, 20, NULL);
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

				if (obj->cropRect.width > 0 && obj->cropRect.height > 0)
				{
					obj->ROI = frame(obj->cropRect);
				}
				else
				{
					frame.copyTo(obj->ROI);
				}

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
		obj->ROI.copyTo(frame);
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
			obj->ROI.copyTo(frame);
			obj->fps = fps;
			contours = obj->contours;
			started = obj->started;
		m.unlock();
	}
	return;
}

void _overlay(Recognizer *obj)
{
	Mat frame, overlayFrame, contouredMaskedFrame;
	bool started = false, recognizeOn = false;
	int frameWidth, frameHeight, fps = 0, nrObjects = 0, nrContours = 0, recCounter;
	Rect cropRect, hand, rLabel;
	string recognizeOnText, slovo, curLetter="", lastLetter="";
	unsigned long begTime = clock();

	m.lock();
		obj->frame.copyTo(frame);
		obj->contouredMaskedFrame.copyTo(contouredMaskedFrame);
		cropRect = obj->cropRect;
		hand = obj->hand;
		slovo = obj->currSlovo;
		recCounter = obj->recCounter;
		started = obj->started;
	m.unlock();

	Sleep(4000);

	while (started)
	{
		frameWidth = frame.cols;
		frameHeight = frame.rows;

		frame.copyTo(overlayFrame);

		if (cropRect.width > 0 && cropRect.height > 0)
		{
			rectangle(overlayFrame, cropRect, Scalar(0,0,255), 4);

			Mat destROI = overlayFrame(cropRect);
			contouredMaskedFrame.copyTo(destROI);
		}


		rectangle(overlayFrame, Point((0 - ovrlyThick * 2), (0 - ovrlyThick)), Point((frameWidth + ovrlyThick * 2), frameHeight), ovrlyColor, ovrlyThick * 3, 8);
		addWeighted(frame, ovrlyAlpha, overlayFrame, 1 - ovrlyAlpha, 0, overlayFrame);
		putText(overlayFrame, topText, Point(5, 15), FONT_HERSHEY_PLAIN, 0.9, txtColor);

		if (recognizeOn) recognizeOnText = "Prepoznavanje u tijeku..";
		else recognizeOnText = "ROI nije odabran.";
		putText(overlayFrame, recognizeOnText, Point(frameWidth - 190, ovrlyThick / 3), FONT_HERSHEY_PLAIN, 0.9, txtColor);


		if (cropRect.width > 0 && cropRect.height > 0)
			putText(overlayFrame, "ROI : " + to_string(abs(rc.P1.x - rc.P2.x)) + " x " + to_string(abs(rc.P1.y - rc.P2.y)), Point(frameWidth - 210, frameHeight - 5), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(overlayFrame, "fps : " + to_string(fps), Point(frameWidth - 80, frameHeight - 5), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		//putText(overlayFrame, "objekata : " + to_string(nrObjects), Point(frameWidth - 191, frameHeight - 20), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		//putText(overlayFrame, "kontura : " + to_string(nrContours), Point(frameWidth - 186, frameHeight - 5), FONT_HERSHEY_PLAIN, 0.9, txtColor);

		hand.x += cropRect.x;
		hand.y += cropRect.y;

		if (recCounter > 0 && hand.height > 40 && hand.x + hand.width <= cropRect.x + cropRect.width && hand.y + hand.height <= cropRect.y + cropRect.height)
		{
			rectangle(overlayFrame, hand, Scalar(255, 255, 255), 1);
			putText(overlayFrame, slovo, Point(hand.x + 5, hand.y + hand.height - 7), FONT_HERSHEY_DUPLEX, 1.1, txtColor);
		}

		putText(overlayFrame, infoText, Point(8, frameHeight + 5 - ovrlyThick / 2), FONT_HERSHEY_TRIPLEX, 1.7, txtColor, 1);

		m.lock();
			
			int imax = obj->letters.size() > brLetersa ? brLetersa : obj->letters.size();
			for (std::vector<Letter>::size_type i = 0; i < imax; i++)
			{
				if (obj->letters[i].votes > 0)
				{
					if (i == 0) curLetter = obj->letters[i].name;
					putText(overlayFrame, to_string(i + 1) + ". " + obj->letters[i].xmlName + " " + to_string(obj->letters[i].votes) + "", Point(frameWidth - 210, frameHeight - 55 + (i * 10)), FONT_HERSHEY_PLAIN, 0.7, txtColor);
				}
				else if (i == 0) curLetter = " ";
			}
			
			if (lastLetter != curLetter)
			{
				setInfo(curLetter);
				lastLetter = curLetter;
			}

			overlayFrame.copyTo(obj->overlyFrame);

			obj->frame.copyTo(frame);
			obj->contouredMaskedFrame.copyTo(contouredMaskedFrame);

			cropRect = obj->cropRect;
			hand = obj->hand;
			slovo = obj->currSlovo;
			recCounter = obj->recCounter;
			started = obj->started;
			nrObjects = obj->nrObjects;
			nrContours = obj->nrContours;
			fps = obj->fps;
			recognizeOn = obj->recognizeOn;

			if (((unsigned long)clock() - begTime) >= voteHyst*50)
			{
				obj->updateLetters();
				begTime = clock();
			}
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

		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) > minContourArea)
			{
				m.lock();					
					obj->nrContours += 40;
					drawContours(obj->contouredMaskedFrame, contours, i, contourColor);
				m.unlock();
			}
			else
			{
				m.lock();
					obj->nrContours = obj->nrContours <= 0 ? 0 : obj->nrContours-1;
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
	Rect cropRect;
	int sizeFactor, nrObjects = 0;

	bool started = false, recognizeOn = false;

	WIN32_FIND_DATA data;
	HANDLE hFind;

	Sleep(5000);

	do {
		nrObjects = 0;

		m.lock();
			obj->maskedFrame.copyTo(frame);
			started = obj->started;
			obj->nrObjects = nrObjects;

			cropRect = obj->cropRect;
			obj->recognizeOn = recognizeOn;
		m.unlock();


		if (cropRect.width < 40 || cropRect.height < 40)
		{
			recognizeOn = false;
			continue;
		}

		recognizeOn = true;
		sizeFactor = (cropRect.width < cropRect.height) ? cropRect.width : cropRect.height;

		hFind = FindFirstFile("haarcascades/*.*", &data);
		do {
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				haarXML = data.cFileName;
				size_t index = haarXML.find_first_of("_");
				haarName = haarXML.substr(0, index);

				cascade.load("haarcascades/" + haarXML);
				cascade.detectMultiScale(frame, hands, 1.3, 3, 0 | CV_HAAR_SCALE_IMAGE, Size((int)sizeFactor / 2 + 20, (int)sizeFactor / 2 + 20), Size((int)sizeFactor + 20, (int)sizeFactor + 20));

				m.lock();
				if (obj->nrContours > 0 && hands.size() > 0)
				{
					obj->nrObjects++;
					for each (Rect hand in hands)
					{
						obj->hand = hand;
						obj->recCounter = 10;
						obj->updateLetters(data.cFileName, haarName, voteStep);
						//break;
					}
				}
				else
				{
					obj->recCounter = obj->recCounter <= 0 ? 0 : obj->recCounter-1;
					obj->nrObjects = obj->nrObjects <= 0 ? 0 : obj->nrObjects-1;
				}
				m.unlock();
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);

	} while (started);

	return;
}

void onMouse(int event, int x, int y, int f, void*)
{
	m.lock();

		x = x < 0 ? 0 : x;
		x = x > rc.frame.cols ? rc.frame.cols : x;
		y = y < 0 ? 0 : y;
		y = y > rc.frame.rows ? rc.frame.rows : y;

		switch (event)
		{
			case  CV_EVENT_LBUTTONDOWN:
				clicked = true;


				rc.P1.x = x;
				rc.P1.y = y;
				rc.P2.x = x;
				rc.P2.y = y;
			break;

			case  CV_EVENT_LBUTTONUP:
				rc.P2.x = x;
				rc.P2.y = y;
				clicked = false;
			break;

			case  CV_EVENT_MOUSEMOVE:
				if (clicked)
				{
					rc.P2.x = x;
					rc.P2.y = y;
				}
			break;

			default:   break;
		}


		if (clicked)
		{

				if (rc.P1.x>rc.P2.x)
					rc.cropRect.x = rc.P2.x;
				else
					rc.cropRect.x = rc.P1.x;

				rc.cropRect.width = abs(rc.P1.x - rc.P2.x);

				if (rc.P1.y>rc.P2.y)
					rc.cropRect.y = rc.P2.y;
				else
					rc.cropRect.y = rc.P1.y;

				rc.cropRect.height = abs(rc.P1.y - rc.P2.y);
		}

	m.unlock();
}