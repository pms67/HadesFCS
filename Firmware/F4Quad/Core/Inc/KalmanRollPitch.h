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
#include <stdint.h>

#define g ((float) 9.81f)

typedef struct {
	float phi;
	float theta;
	float P[4];
	float Q[2];
	float R[3];
	float gyr[3];
} KalmanRollPitch;

void KalmanRollPitch_Init(KalmanRollPitch *kal, float Pinit, float *Q, float *R);
void KalmanRollPitch_Predict(KalmanRollPitch *kal, float *gyr, float T);
uint8_t KalmanRollPitch_Update(KalmanRollPitch *kal, float *acc, float Va);

#endif
