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

#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

/* PI Controller struct */
typedef struct {
	float Kp;
	float Ki;
	float Kff;
	float limMin;
	float limMax;
	float limMinFF;
	float limMaxFF;
	float integrator;
	float prevError;
	float output;
} PIController;

/* Initialise PI controller with gains and output limits */
void PI_Init(PIController *ctrl, float Kp, float Ki, float limMin, float limMax);

/* Reset persistent variables and set output to zero */
void PI_Reset(PIController *ctrl);

/* Enable feedforward control */
void PI_SetFF(PIController *ctrl, float Kff, float limMinFF, float limMaxFF);

/* Main controller update routine, requires sample time T (in seconds) */
float PI_Update(PIController *ctrl, float setpoint, float measurement, float T);

#endif
