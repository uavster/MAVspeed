/*
 * MyDrone.h
 *
 *  Created on: 05/09/2011
 *      Author: Ignacio Mellado
 */

#ifndef MYDRONE_H_
#define MYDRONE_H_

#include <droneproxy.h>
#include "PID.h"
#include <atlante.h>
#include "VideoUI.h"

class MyDrone : public virtual DroneProxy::LoggerDrone {
private:
	PID pidYaw, pidForwardSpeed, pidDrift, pidHeight;

	cvgMutex refMutex;

	VideoUI videoUI;

public:
	MyDrone();
	~MyDrone();

	void setYaw(cvg_double yaw);
	void setForwardSpeed(cvg_double speed);
	void setLateralSpeed(cvg_double speed);
	void setAltitude(cvg_double altitude);

	void incrementYaw(cvg_double inc);
	void incrementAltitude(cvg_double inc);

protected:
	cvg_double mapAngleToMinusPlusPi(cvg_double angleRads);
	void resetPIDs();
protected:
	virtual void processVideoFrame(cvg_int cameraId, cvg_ulong timeCode, VideoFormat format, cvg_uint width, cvg_uint height, cvg_char *frameData);
	virtual void processFeedback(FeedbackData *feebackData);
	virtual void setConfigParams();
};

#endif /* MYDRONE_H_ */
