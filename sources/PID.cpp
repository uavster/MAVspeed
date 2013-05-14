/*
 * PID.cpp
 *
 *  Created on: 19/09/2011
 *      Author: Ignacio Mellado
 */
#include <iostream>
#include "PID.h"

PID::PID() {
	Kp = 0.0;
	Ki = 0.0;
	Kd = 0.0;
	feedback = 0.0;
	reference = 0.0;
	output = 0.0;
	integrator = 0.0;
	lastError = 0.0;
	for (cvg_int i = 0; i < PID_D_FILTER_LENGTH; i++) dErrorHistory[i] = 0.0;
	started = false;
	enableAntiWindup(false, 0.0);
	enableMaxOutput(false, 0.0);
}

PID::~PID() {
}

cvg_double PID::getOutput() {
	cvg_double elapsed = timer.getElapsedSeconds();
	timer.restart(started);
	if (!started) {
		started = true;
		return output;
	}

	for (cvg_int i = 0; i < PID_D_FILTER_LENGTH - 1; i++)
		dErrorHistory[i] = dErrorHistory[i + 1];
	dErrorHistory[PID_D_FILTER_LENGTH - 1] = (error - lastError) / elapsed;
	lastError = error;

	cvg_double dError = 0.0;
	if (Kd != 0.0) {
		// Filter derivative
		cvg_double weight = 2.0 / (PID_D_FILTER_LENGTH + 1);
		for (cvg_int i = 0; i < PID_D_FILTER_LENGTH; i++) {
			dError += weight * dErrorHistory[PID_D_FILTER_LENGTH - i - 1];
	#if PID_D_FILTER_LENGTH > 1
			weight -= (2.0 / (PID_D_FILTER_LENGTH * (PID_D_FILTER_LENGTH + 1)));
	#endif
		}
		// Output with derivative action
		output = Kp * error + Ki * integrator + Kd * dError;
	} else {
		// If there's no derivative component, there's no need to filter the error derivative
		// Output without derivative action
		output = Kp * error + Ki * integrator;
	}

	// Anti-windup
	if (actuatorSaturation >= 0.0) {
		// If enabled, only update the integrator if the PID output is inside the actuator operation range
		// or it is higher than the upper bound but the error is negative
		// or it is lower than the lower bound but the error is positive
		cvg_double ki_int = Ki * integrator;
		if ((ki_int < actuatorSaturation && ki_int > -actuatorSaturation) ||
			(ki_int >= actuatorSaturation && error < 0) ||
			(ki_int <= -actuatorSaturation && error > 0)
			)
			integrator += error * elapsed;
	} else integrator += error * elapsed;

//std::cout << "PID int.: " << Ki*integrator;

	if (maxOutput >= 0.0) {
		if (output > maxOutput) output = maxOutput;
		else if (output < -maxOutput) output = -maxOutput;
	}

	return output;
}

void PID::enableAntiWindup(cvg_bool enable, cvg_double actuatorSaturation) {
	if (enable) {
		if (actuatorSaturation < 0) this->actuatorSaturation = -actuatorSaturation;
		else this->actuatorSaturation = actuatorSaturation;
	}
	else this->actuatorSaturation = -1.0;
}

void PID::enableMaxOutput(cvg_bool enable, cvg_double max) {
	if (enable) {
		if (max < 0) maxOutput = -max;
		else maxOutput = max;
	}
	else maxOutput = -1.0;
}

void PID::reset() {
	error = 0.0;
	integrator = 0.0;
	lastError = 0.0;
	for (cvg_int i = 0; i < PID_D_FILTER_LENGTH; i++) dErrorHistory[i] = 0.0;
	started = false;
}
