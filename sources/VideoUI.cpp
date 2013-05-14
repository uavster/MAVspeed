/*
 * VideoUI.cpp
 *
 *  Created on: 14/11/2012
 *      Author: Ignacio Mellado-Bataller
 */

#include "VideoUI.h"

cvg_bool VideoUI::windowThreadStarted = false;
cvgMutex VideoUI::wtsMutex;

VideoUI::VideoUI() : cvgThread("VideoUI") {
	if (!wtsMutex.lock()) throw cvgException("[VideoUI] Unable to lock wtsMutex");
	if (!windowThreadStarted) {
		cvStartWindowThread();
		windowThreadStarted = true;
	}
	wtsMutex.unlock();

	for (cvg_int i = 0; i < VIDEOUI_MAX_SOURCES; i++)
		frame[i] = NULL;

#ifdef ENABLE_SPEED_MONITOR
	lastFeedback.userData[0] = 1e6;
	speedMonitor.setParent(this);
#endif
}

VideoUI::~VideoUI() {
	closeWindows();
	stop();
}

void VideoUI::updateVideoSource(cvg_int id, cvg_int width, cvg_int height, cvg_char *frameData) {
	cvgString camWindowName = cvgString("Camera ") + id;
	if (windowMutex[id].lock()) {
		if (frame[id] == NULL) { // && getChannelManager().getVideoChannel(cameraId).getCurrentState() == Comm::Channel::CONNECTED) {
			frame[id] = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
		}
		cvSetData(frame[id], frameData, width * 3);
		windowMutex[id].unlock();
	}
}

void VideoUI::closeWindow(cvg_int id) {
	if (windowMutex[id].lock()) {
		if (frame[id] != NULL) {
			cvDestroyWindow((cvgString("Camera ") + id).c_str());
			cvReleaseImageHeader(&frame[id]);
			frame[id] = NULL;
		}
		windowMutex[id].unlock();
	}
}

void VideoUI::closeWindows() {
	for (cvg_int i = 0; i < VIDEOUI_MAX_SOURCES; i++)
		closeWindow(i);
#ifdef ENABLE_SPEED_MONITOR
	cvDestroyWindow("Speed");
	cvReleaseImage(&speedMonitor.speedImg);
#endif
}

void VideoUI::run() {
#ifdef ENABLE_SPEED_MONITOR
	speedMonitor.run();
#endif
	for (cvg_int i = 0; i < VIDEOUI_MAX_SOURCES; i++) {
		if (windowMutex[i].lock()) {
			if (frame[i] != NULL) {
				cvgString camWindowName = cvgString("Camera ") + i;
				cvNamedWindow(camWindowName.c_str());
				cvShowImage(camWindowName.c_str(), frame[i]);
			}
			windowMutex[i].unlock();
		}
	}

	cvWaitKey(33);
}

#ifdef ENABLE_SPEED_MONITOR
void VideoUI::SpeedMonitor::run() {
	if (speedImg == NULL) {
		cvNamedWindow("Speed");
		speedImg = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	} else {
		cvFillImage(speedImg, 0.0);
		FeedbackData fd;
		fd.userData[0] = 1e6;
		if (parent->lastFeedbackMutex.lock()) {
			fd = parent->lastFeedback;
			parent->lastFeedbackMutex.unlock();
		}

		if (fd.userData[0] < 1e6) {
			cvLine(speedImg, cvPoint(160 + 160 * fd.userData[0], 120 + 120 * fd.userData[1]), cvPoint(160, 120), CV_RGB(255, 0, 0), 1, CV_AA);
			cvLine(speedImg, cvPoint(160 + 160 * fd.speedX, 120 + 120 * fd.speedY), cvPoint(160, 120), CV_RGB(128, 128, 0), 1, CV_AA);
		}
		cvShowImage("Speed", speedImg);
	}
}
#endif

void VideoUI::updateFeedbackData(const FeedbackData &lastFeedback) {
#ifdef ENABLE_SPEED_MONITOR
	if (!lastFeedbackMutex.lock()) throw cvgException("[VideoUI] Unable to lock lastFeedbackMutex");
	this->lastFeedback = lastFeedback;
	lastFeedbackMutex.unlock();
#endif
}
