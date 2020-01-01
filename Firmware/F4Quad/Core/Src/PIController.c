/*
*
* PI Controller
*
* Implements a discrete-time PI controller with optional feedforward term.
* Controller was derived using an analog PI prototype and converted to the discrete-time domain
* using the Tustin transform.
* 
* Written by: Philip M. Salmony @ philsal.co.uk
* Last changed: 01 Dec 2019
*
*/

#include "PIController.h"

/* Initialise PI controller with gains and output limits */
void PI_Init(PIController *ctrl, float Kp, float Ki, float limMin, float limMax) {
	ctrl->Kp = Kp;
	ctrl->Ki = Ki;	
	ctrl->limMin = limMin;
	ctrl->limMax = limMax;
	ctrl->integrator = 0.0f;
	ctrl->prevError  = 0.0f;
	ctrl->output     = 0.0f;
	
	/* Disable feedforward control */
	ctrl->limMinFF = 0.0f;
	ctrl->limMaxFF = 0.0f;
}

/* Reset persistent variables and set output to zero */
void PI_Reset(PIController *ctrl) {
	ctrl->integrator = 0.0f;
	ctrl->prevError  = 0.0f;
	ctrl->output     = 0.0f;
}

/* Enable feedforward control */
void PI_SetFF(PIController *ctrl, float Kff, float limMinFF, float limMaxFF) {
	ctrl->Kff = Kff;
	ctrl->limMinFF = limMinFF;
	ctrl->limMaxFF = limMaxFF;
}

/* Main controller update routine, requires sample time T (in seconds) */
float PI_Update(PIController *ctrl, float setpoint, float measurement, float T) {
	/* Error signal */
	float error = setpoint - measurement;
	
	/* Proportional */
	float proportional = ctrl->Kp * error;
	
	/* Feedforward */
	float ff = ctrl->Kff * setpoint;
	
	if (ff > ctrl->limMaxFF) {
		ff = ctrl->limMaxFF;
	} else if (ff < ctrl->limMinFF) {
		ff = ctrl->limMinFF;
	}

	/* Integrator */
	ctrl->integrator = ctrl->integrator + 0.5f * T * ctrl->Ki * (error + ctrl->prevError);

	/* Dynamic integrator clamping */
	float limMinInt, limMaxInt;
	
	if (ctrl->limMax - proportional - ff > 0.0f) {
		limMaxInt = ctrl->limMax - proportional;
	} else {
		limMaxInt = 0.0f;
	}
	
	if (ctrl->limMin - proportional - ff < 0.0f) {
		limMinInt = ctrl->limMin - proportional;
	} else {
		limMinInt = 0.0f;
	}
	
	if (ctrl->integrator > limMaxInt) {
		ctrl->integrator = limMaxInt;
	} else if (ctrl->integrator < limMinInt) {
		ctrl->integrator = limMinInt;
	}
	
	/* Set output and clamp */
	ctrl->output = proportional + ff + ctrl->integrator;
	
	if (ctrl->output > ctrl->limMax) {
		ctrl->output = ctrl->limMax;
	} else if (ctrl->output < ctrl->limMin) {
		ctrl->output = ctrl->limMin;
	}
	
	/* Store error signal */
	ctrl->prevError = error;
	
	/* Return controller output */
	return ctrl->output;
}
