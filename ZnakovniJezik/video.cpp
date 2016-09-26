#include <ctime>
#include <iostream>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "video.h"

using namespace std;
using namespace cv;

Video::Video()
{
	frmWidth = 45;
	frmColor = Scalar(164, 164, 164);
	txtColor = Scalar(255, 255, 255);
	detColor = Scalar(128, 255, 128);
	frmAlpha = 0.3;
	tlText = "[ESC-izlaz] [O-overlay] [M-mask]";
	fpsText = "fps: /";
	btmText = "Startam..";
	frameCount = 1000;
	start = clock();
	bcgSet = false;
	started = false;
	masked = false;
	overlayed = true;
	haarXML = "";
	vWidth = 640;
	vHeight = 480;
	setInfo("Startam..");
	thresh = 25;
	rng(12345);
}

Mat Video::overlay()
{
	Mat overlayFrm, newFrm;
	vWidth = frame.cols, vHeight = frame.rows;
	//Mat overlayFrm(h, w, 16, Scalar::all(64));

	if (frameCount > 6000)
	{
		fps = (double)frameCount / (double)((clock() - start));
		fpsText = "fps: " + to_string((int)fps);
		frameCount = 1000;
		start = clock();
	}
	

	if (bcgSet)
	{
		Mat dest, bcgMask, frmHSV, hsvMask;
		absdiff(frame, bcgFrame, bcgMask);
		cvtColor(bcgMask, bcgMask, CV_BGR2GRAY);
		bcgMask = bcgMask > 0;

		/*
		cv::subtract(bcgFrame, newFrm, dest);
		cvtColor(dest, dest, CV_BGR2GRAY);
		cv::threshold(dest, dest, 50, 255, CV_THRESH_BINARY);
		*/

		frame.copyTo(frame, bcgMask);

		imshow("mask", bcgMask);
		//imshow("out", dest);
	}

	if (masked) GetSkin().copyTo(newFrm);
	else frame.copyTo(newFrm);

	for (auto const& hand : hands) {
		ellipse(newFrm, hand.center, hand.size, 0, 0, 360, detColor, 2, 8, 0);
	}
	newFrm.copyTo(overlayFrm);

	if (overlayed)
	{
		rectangle(overlayFrm, Point((0 - frmWidth * 2), (0 - frmWidth)), Point((vWidth + frmWidth * 2), vHeight), frmColor, frmWidth * 3, 8);
		addWeighted(newFrm, frmAlpha, overlayFrm, 1 - frmAlpha, 0, newFrm);
		putText(newFrm, tlText, Point(5, frmWidth / 3), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(newFrm, fpsText, Point(vWidth - 70, frmWidth / 3), FONT_HERSHEY_PLAIN, 0.9, txtColor);
		putText(newFrm, btmText, Point(5, vHeight - frmWidth / 2), FONT_HERSHEY_TRIPLEX, 1.7, txtColor, 1);
	}
	
	return newFrm;
}

void Video::detect()
{
	vector<Rect> hands;
	Mat frame_gray;
	int w = frame.cols, h = frame.rows;
	int handX, handY, handW, handH;
	double scaleFactor = 2;
	cascade.load("haarcascades/" + haarXML);
	return;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	cascade.detectMultiScale(frame_gray, hands, 1.2, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(50, 50));

	for (size_t i = 0; i < hands.size(); i++)
	{
		handX = (int)(hands[i].x*scaleFactor);
		handY = (int)(hands[i].y*scaleFactor);
		handW = (int)(hands[i].width*scaleFactor);
		handH = (int)(hands[i].height*scaleFactor);

		Point center(handX + handW*0.5, handY + handH*0.5);
		Size size(handW*0.5, handH*0.5);
		ellipse(frame, center, size, 0, 0, 360, txtColor, 4, 8, 0);

		btmText = haarXML;
		size_t lastindex = haarXML.find_last_of(".");
		cout << haarXML.substr(0, lastindex) << endl;

		break;
	}
	return;
}

bool Video::R1(int R, int G, int B) {
	bool e1 = (R>95) && (G>40) && (B>20) && ((max(R, max(G, B)) - min(R, min(G, B)))>15) && (abs(R - G)>15) && (R>G) && (R>B);
	bool e2 = (R>220) && (G>210) && (B>170) && (abs(R - G) <= 15) && (R>B) && (G>B);
	return (e1 || e2);
}

bool Video::R2(float Y, float Cr, float Cb) {
	bool e3 = Cr <= 1.5862*Cb + 20;
	bool e4 = Cr >= 0.3448*Cb + 76.2069;
	bool e5 = Cr >= -4.5652*Cb + 234.5652;
	bool e6 = Cr <= -1.15*Cb + 301.75;
	bool e7 = Cr <= -2.2857*Cb + 432.85;
	return e3 && e4 && e5 && e6 && e7;
}

bool Video::R3(float H, float S, float V) {
	//return (H<25) || (H > 230);
	return (H<25) || (H > 230);
}

Mat Video::GetSkin() {
	// allocate the result matrix
	Mat dst = frame.clone();

	Vec3b cwhite = Vec3b::all(255);
	Vec3b cblack = Vec3b::all(0);

	Mat src_ycrcb, src_hsv;
	// OpenCV scales the YCrCb components, so that they
	// cover the whole value range of [0,255], so there's
	// no need to scale the values:
	cvtColor(frame, src_ycrcb, CV_BGR2YCrCb);
	// OpenCV scales the Hue Channel to [0,180] for
	// 8bit images, so make sure we are operating on
	// the full spectrum from [0,360] by using floating
	// point precision:
	frame.convertTo(src_hsv, CV_32FC3);
	cvtColor(src_hsv, src_hsv, CV_BGR2HSV);
	// Now scale the values between [0,255]:
	normalize(src_hsv, src_hsv, 0.0, 255.0, NORM_MINMAX, CV_32FC3);

	for (int i = 0; i < frame.rows; i++) {
		for (int j = 0; j < frame.cols; j++) {

			Vec3b pix_bgr = frame.ptr<Vec3b>(i)[j];
			int B = pix_bgr.val[0];
			int G = pix_bgr.val[1];
			int R = pix_bgr.val[2];
			// apply rgb rule
			bool a = R1(R, G, B);

			Vec3b pix_ycrcb = src_ycrcb.ptr<Vec3b>(i)[j];
			int Y = pix_ycrcb.val[0];
			int Cr = pix_ycrcb.val[1];
			int Cb = pix_ycrcb.val[2];
			// apply ycrcb rule
			bool b = R2(Y, Cr, Cb);

			Vec3f pix_hsv = src_hsv.ptr<Vec3f>(i)[j];
			float H = pix_hsv.val[0];
			float S = pix_hsv.val[1];
			float V = pix_hsv.val[2];
			// apply hsv rule
			bool c = R3(H, S, V);

			if (!(a&&b&&c))
				dst.ptr<Vec3b>(i)[j] = cblack;
		}
	}
	return dst;
}

void Video::contours(double thresh)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(frame, canny_output, thresh, thresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	/// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);
}

void Video::setInfo(cv::string info)
{
	btmText = info;
	cout << info << endl;
}

void Video::setBcgFrame(cv::Mat frame)
{
	bcgSubtractor = BackgroundSubtractorMOG2();
	frame.copyTo(bcgFrame);
	bcgSet = true;
}
void Video::unSetBcgFrame()
{
	bcgSet = false;
}

Video::~Video()
{
}
