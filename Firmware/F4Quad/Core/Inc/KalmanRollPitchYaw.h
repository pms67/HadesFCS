/*
*
* Kalman Filter - Three States (Roll, Pitch, Yaw)
*
* Implements a continuous-discrete Kalman filter for estimation of roll and pitch angles.
* Sensors: gyro (p, q, r), accelerometer (ax, ay, az), pitot tube and differential pressure sensor (Va), and magnetometer or other heading reference
*
* Written by: Philip M. Salmony @ philsal.co.uk
* Last changed: 01 Jann 2020
*
*/

#ifndef KALMAN_ROLL_PITCH_H
#define KALMAN_ROLL_PITCH_H

#include <math.h>
#include <stdint.h>

#define g ((float) 9.80655f)

typedef struct {
	float phi;
	float theta;
	float psi;
	float P[9];
	float Q[3];
	float Racc;
	float Rpsi;
	float gyr[3];
} KalmanRollPitchYaw;

void KalmanRollPitchYaw_Init(KalmanRollPitchYaw *kal, float Pinit, float *Q, float Racc, float Rpsi);
void KalmanRollPitchYaw_Predict(KalmanRollPitchYaw *kal, float *gyr, float T);
uint8_t KalmanRollPitchYaw_UpdateAcc(KalmanRollPitchYaw *kal, float *acc, float Va);
uint8_t KalmanRollPitchYaw_UpdatePsi(KalmanRollPitchYaw *kal, float psi);

#endif
