/*
 * MyDrone.cpp
 *
 *  Created on: 05/09/2011
 *      Author: Ignacio Mellado
 */

#include <MyDrone.h>
#include <cv.h>
#include <highgui.h>

using namespace DroneProxy;

// Maximum allowed pitch and roll degrees
// This parameter will be set in the drone configuration. However, some drones don't support it yet, 
// so make sure you set this to the maximum angle defined in your drone's proxy.
#define MAX_QUAD_EULER_ANGLE		27.0

// Horizontal speed controller gains
#define SPEED_P				(9.0 / MAX_QUAD_EULER_ANGLE)		// [[0..1] / (m/s)]
#define SPEED_I				(SPEED_P / 5.0)						// Ti = 1s
#define SPEED_D				(0.5 / MAX_QUAD_EULER_ANGLE) // (SPEED_P * (2.0/9.0))			// Td = 0.04s

// Altitude controller gains
#define ALT_SPEED_P			1.0
#define ALT_SPEED_I			0.0
#define ALT_SPEED_D			0.0

// Yaw controller gains
#define YAW_P				1.0
#define YAW_I				0.0
#define YAW_D				0.0

// Comment this line to disable. If enabled, a window will be open for every video source.
// Under slow communication links or with large resolutions, the image refresh rate may be slow.
#define ENABLE_VIDEO_UI

MyDrone::MyDrone() {
#ifdef ENABLE_VIDEO_UI
	videoUI.start();
#endif

	pidYaw.enableMaxOutput(true, 1.0);
	pidForwardSpeed.enableMaxOutput(true, 1.0);
	pidHeight.enableMaxOutput(true, 1.0);
	pidDrift.enableMaxOutput(true, 1.0);

	// Kp = 1/Ts, Normalized Kp = 1/Ts / (MAX_YAW_SPEED * M_PI / 180)
	pidYaw.setGains(YAW_P, YAW_I, YAW_D);

	// Kp = 1/Ts, Normalized Kp = 1/Ts / (MAX_HEIGHT_SPEED)
	pidHeight.setGains(ALT_SPEED_P, ALT_SPEED_I, ALT_SPEED_D);

	pidDrift.setGains(SPEED_P, SPEED_I, SPEED_D);
	pidDrift.enableAntiWindup(true, 1.0);

	pidForwardSpeed.setGains(SPEED_P, SPEED_I, SPEED_D);
	pidForwardSpeed.enableAntiWindup(true, 1.0);

	pidForwardSpeed.setReference(0.0);
	pidDrift.setReference(0.0);
}

MyDrone::~MyDrone() {
	close();
}

void MyDrone::processVideoFrame(cvg_int cameraId, cvg_ulong timeCode, VideoFormat format, cvg_uint width, cvg_uint height, cvg_char *frameData) {
	if (format == RAW_BGR24 || format == RAW_RGB24) {
#ifdef ENABLE_VIDEO_UI
		videoUI.updateVideoSource(cameraId, width, height, frameData);
#endif
	}
	// Save data to log
	LoggerDrone::processVideoFrame(cameraId, timeCode, format, width, height, frameData);
}

void MyDrone::processFeedback(FeedbackData *feedbackData) {
	// Save data to log (including Vicon)
	LoggerDrone::processFeedback(feedbackData);
#ifdef ENABLE_VIDEO_UI
	videoUI.updateFeedbackData(*feedbackData);
#endif
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");

	if (feedbackData->droneMode != FLYING) {

		// After starting to fly, the PIDs will be reset and the yaw and altitude will be the current ones
		resetPIDs();
		pidYaw.setReference(feedbackData->yaw * M_PI / 180.0);
		pidHeight.setReference(-feedbackData->altitude);

	} else {

		// Feed measures to PIDs
		pidYaw.setFeedback(feedbackData->yaw * M_PI / 180.0);
		pidForwardSpeed.setFeedback(feedbackData->speedX);
		pidDrift.setFeedback(feedbackData->speedY);
		pidHeight.setFeedback(-feedbackData->altitude);

//		log(cvgString("ref. vx: ") + pidForwardSpeed.getReference() + ", cmd. vx: " + pidForwardSpeed.getOutput());

		// Send PIDs' outputs to the drone
		setControlData(	Comm::ControlChannel::THETA | Comm::ControlChannel::PHI | Comm::ControlChannel::YAW | Comm::ControlChannel::GAZ,
						pidDrift.getOutput(),
						-pidForwardSpeed.getOutput(),
						pidHeight.getOutput(),
						pidYaw.getOutput());

	}

	refMutex.unlock();
}

void MyDrone::setYaw(cvg_double yawRads) {
	cvg_double yawMapped = mapAngleToMinusPlusPi(yawRads);
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidYaw.setReference(yawMapped);
	refMutex.unlock();
}

void MyDrone::setForwardSpeed(cvg_double speed) {
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidForwardSpeed.setReference(speed);
	refMutex.unlock();
}

void MyDrone::setAltitude(cvg_double altitude) {
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidHeight.setReference(altitude);
	refMutex.unlock();
}

void MyDrone::setLateralSpeed(cvg_double speed) {
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidDrift.setReference(speed);
	refMutex.unlock();
}

cvg_double MyDrone::mapAngleToMinusPlusPi(cvg_double angleRads) {
	cvg_double mapped = fmod(angleRads, M_PI);
	cvg_int turns = (cvg_int)floor(angleRads / M_PI);
	if (turns & 1) {
		if (turns > 0)
			mapped -= M_PI;
		else
			mapped += M_PI;
	}
	return mapped;
}

void MyDrone::resetPIDs() {
	pidYaw.reset();
	pidForwardSpeed.reset();
	pidDrift.reset();
	pidHeight.reset();
}

void MyDrone::setConfigParams() {
	LedsAnimation anim;
	anim.typeId = LEDS_RED_SNAKE;
	anim.frequencyHz = 2.0f;
	anim.durationSec = 0;
	writeParam(CONFIGPARAM_LEDS_ANIMATION, (cvg_char *)&anim, sizeof(LedsAnimation));
	writeParam(CONFIGPARAM_MAX_EULER_ANGLES, (cvg_float)MAX_QUAD_EULER_ANGLE);
	writeParam(CONFIGPARAM_VIDEO_ENCODING_TYPE, JPEG);
	writeParam(CONFIGPARAM_VIDEO_ENCODING_QUALITY, 1.0f);
}


void MyDrone::incrementYaw(cvg_double inc) {
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidYaw.setReference(pidYaw.getReference() + inc);
	refMutex.unlock();
}

void MyDrone::incrementAltitude(cvg_double inc) {
	if (!refMutex.lock()) throw cvgException("Cannot lock ref mutex");
	pidHeight.setReference(pidHeight.getReference() + inc);
	refMutex.unlock();
}
