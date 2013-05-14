/*
 * VideoUI.h
 *
 *  Created on: 14/11/2012
 *      Author: Ignacio Mellado-Bataller
 */

#ifndef VIDEOUI_H_
#define VIDEOUI_H_


#define ENABLE_SPEED_MONITOR

#include <atlante.h>
#include <highgui.h>
#include <cv.h>
#include <droneproxy.h>

#define VIDEOUI_MAX_SOURCES		3

class VideoUI : public virtual cvgThread {

public:
	VideoUI();
	virtual ~VideoUI();

	void updateVideoSource(cvg_int id, cvg_int width, cvg_int height, cvg_char *frameData);
	void closeWindows();
	void updateFeedbackData(const FeedbackData &lastFeedback);

private:
	static cvgMutex wtsMutex;
	static cvg_bool windowThreadStarted;
	cvgMutex windowMutex[VIDEOUI_MAX_SOURCES];
	IplImage *frame[VIDEOUI_MAX_SOURCES];
#ifdef ENABLE_SPEED_MONITOR
	cvgMutex lastFeedbackMutex;
	FeedbackData lastFeedback;
#endif

protected:
	void run();
	void closeWindow(cvg_int id);

private:
#ifdef ENABLE_SPEED_MONITOR
	class SpeedMonitor {
	public:
		inline SpeedMonitor() { speedImg = NULL; }
		inline virtual ~SpeedMonitor() {}
		void run();

		inline void setParent(VideoUI *md) { parent = md; }
		IplImage *speedImg;

	private:
		VideoUI *parent;
	};

	SpeedMonitor speedMonitor;
#endif

};

#endif /* VIDEOUI_H_ */
