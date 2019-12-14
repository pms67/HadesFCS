/*
*
* Kalman Filter - Two States (Roll and Pitch)
*
* Implements a continuous-discrete Kalman filter for estimation of roll and pitch angles.
* Sensors: gyro (p, q, r), accelerometer (ax, ay, az), pitot tube and differential pressure sensor (Va)
*
* Written by: Philip M. Salmony @ philsal.co.uk
* Last changed: 02 Dec 2019
*
*/

#ifndef KALMAN_ROLL_PITCH_H
#define KALMAN_ROLL_PITCH_H

#include <math.h>

#define g ((float) 9.81f)

typedef struct {
	float phi;
	float theta;
	float P[4];
	float Q[2];
	float R[3];
} KalmanRollPitch;

void KalmanRollPitch_Init(KalmanRollPitch *kal, float Pinit, float *Q, float *R);
void KalmanRollPitch_Update(KalmanRollPitch *kal, float *gyr, float T);
void KalmanRollPitch_UpdateMeasurement(KalmanRollPitch *kal, float *gyr, float *acc, float Va);

#endif
