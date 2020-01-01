#ifndef KALMAN_AHRS_QUAT_H
#define KALMAN_AHRS_QUAT_H

#include <stdint.h>
#include <math.h>

#define g ((float) 9.81f)

typedef struct {
	/* State estimates */
	float x[4];	
	
	/* Covariances */
	float P[4][4];	
	float Q;
	float Racc;
	float Rheading;
	
	/* Measurements */
	float p, q, r;
	float V;
} EKF;

void EKF_Init(EKF *ekf, float initErrorCovariance, float noiseGyr, float noiseAcc, float noiseHeading);
void EKF_Predict(EKF *ekf, float *gyr_radps, float T);
void EKF_UpdateAirspeed(EKF *ekf, float V_mps);
uint8_t EKF_UpdateAccelerometer(EKF *ekf, float *acc_mps2);
uint8_t EKF_UpdateHeading(EKF *ekf, float psi_rad);

#endif
