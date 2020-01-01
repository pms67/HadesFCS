#include "KalmanAHRSQuat.h"

void EKF_Init(EKF *ekf, float initErrorCovariance, float noiseGyr, float noiseAcc, float noiseHeading) {
	
	int i, j;
	
	for (i = 0; i < 4; i++) {
		
		ekf->x[i] = 0.0f;
		
		for (j = 0; j < 4; j++) {
			
			if (i == j) {
				ekf->P[i][j] = initErrorCovariance;
			} else {
				ekf->P[i][j] = 0.0f;
			}
			
		}
		
	}

	/* Assume initially no roll, pitch, or yaw */
	ekf->x[0] = 1.0f;

	ekf->Q		  = 3.0f * noiseGyr * noiseGyr;
	ekf->Racc     = noiseAcc * noiseAcc;
	ekf->Rheading = noiseHeading * noiseHeading;
		
	ekf->V = 0.0f;
}

void EKF_Predict(EKF *ekf, float *gyr_radps, float T) {
		
	/* Extract measurements */
	float p = gyr_radps[0];
	float q = gyr_radps[1];
	float r = gyr_radps[2];
	
	/* Store measurements for later use in output functions */
	ekf->p = p;
	ekf->q = q;
	ekf->r = r;
	
	/* Extract states */
	float q0 = ekf->x[0];
	float q1 = ekf->x[1];
	float q2 = ekf->x[2];
	float q3 = ekf->x[3];
	
	/* Common term */
	float T2 = 0.5f * T;
	
	/* State transition function f(x,u) */
	ekf->x[0] += T2 * (-p * q1 - q * q2 - r * q3);
	ekf->x[1] += T2 * ( p * q0 - q * q3 + r * q2);
	ekf->x[2] += T2 * ( p * q3 + q * q0 - r * q1);
	ekf->x[3] += T2 * (-p * q2 + q * q1 + r * q0);
	
	/* Normalise quaternion */
	float inorm = 1.0f / sqrt(ekf->x[0] * ekf->x[0] + ekf->x[1] * ekf->x[1]
								+ ekf->x[2] * ekf->x[2] + ekf->x[3] * ekf->x[3]);
								
	ekf->x[0] *= inorm;
	ekf->x[1] *= inorm;
	ekf->x[2] *= inorm;
	ekf->x[3] *= inorm;
	
	/* Re-extract states */
	q0 = ekf->x[0];
	q1 = ekf->x[1];
	q2 = ekf->x[2];
	q3 = ekf->x[3];
	
	/* Update error covariance matrix */
	float P00 = ekf->P[0][0]; float P01 = ekf->P[0][1]; float P02 = ekf->P[0][2]; float P03 = ekf->P[0][3];
	float P10 = ekf->P[1][0]; float P11 = ekf->P[1][1]; float P12 = ekf->P[1][2]; float P13 = ekf->P[1][3];
	float P20 = ekf->P[2][0]; float P21 = ekf->P[2][1]; float P22 = ekf->P[2][2]; float P23 = ekf->P[2][3];
	float P30 = ekf->P[3][0]; float P31 = ekf->P[3][1]; float P32 = ekf->P[3][2]; float P33 = ekf->P[3][3];
	
	ekf->P[0][0] += T2 * (2*ekf->Q - P01*p - P10*p - P02*q - P20*q - P03*r - P30*r);
	ekf->P[0][1] += T2 * (P00*p - P11*p - P03*q - P21*q + P02*r - P31*r);
	ekf->P[0][2] += T2 * (P03*p - P12*p + P00*q - P22*q - P01*r - P32*r);
	ekf->P[0][3] += T2 * (P01*q - P13*p - P02*p - P23*q + P00*r - P33*r);
	
    ekf->P[1][0] += T2 * (P00*p - P11*p - P12*q - P30*q - P13*r + P20*r);
	ekf->P[1][1] += T2 * (2*ekf->Q + P01*p + P10*p - P13*q - P31*q + P12*r + P21*r);
	ekf->P[1][2] += T2 * (P02*p + P13*p + P10*q - P32*q - P11*r + P22*r);
	ekf->P[1][3] += T2 * (P03*p - P12*p + P11*q - P33*q + P10*r + P23*r);

	ekf->P[2][0] += T2 * (P30*p - P21*p + P00*q - P22*q - P10*r - P23*r);
	ekf->P[2][1] += T2 * (P20*p + P31*p + P01*q - P23*q - P11*r + P22*r);
	ekf->P[2][2] += T2 * (2*ekf->Q + P23*p + P32*p + P02*q + P20*q - P12*r - P21*r);
	ekf->P[2][3] += T2 * (P33*p - P22*p + P03*q + P21*q - P13*r + P20*r);
	
	ekf->P[3][0] += T2 * (P10*q - P31*p - P20*p - P32*q + P00*r - P33*r);
	ekf->P[3][1] += T2 * (P30*p - P21*p + P11*q - P33*q + P01*r + P32*r);
	ekf->P[3][2] += T2 * (P33*p - P22*p + P12*q + P30*q + P02*r - P31*r);
	ekf->P[3][3] += T2 * (2*ekf->Q - P23*p - P32*p + P13*q + P31*q + P03*r + P30*r);
	
}

void EKF_UpdateAirspeed(EKF *ekf, float V_mps) {
	ekf->V = V_mps;
}

uint8_t EKF_UpdateAccelerometer(EKF *ekf, float *acc_mps2) {
	
	/* Extract states */
	float q0 = ekf->x[0];
	float q1 = ekf->x[1];
	float q2 = ekf->x[2];
	float q3 = ekf->x[3];
	
	/* Calculate Kalman gain */
	float P00 = ekf->P[0][0]; float P01 = ekf->P[0][1]; float P02 = ekf->P[0][2]; float P03 = ekf->P[0][3];
	float P10 = ekf->P[1][0]; float P11 = ekf->P[1][1]; float P12 = ekf->P[1][2]; float P13 = ekf->P[1][3];
	float P20 = ekf->P[2][0]; float P21 = ekf->P[2][1]; float P22 = ekf->P[2][2]; float P23 = ekf->P[2][3];
	float P30 = ekf->P[3][0]; float P31 = ekf->P[3][1]; float P32 = ekf->P[3][2]; float P33 = ekf->P[3][3];
	
	float G00 = ekf->Racc + 2*g*q2*(2*P00*g*q2 - 2*P10*g*q3 + 2*P20*g*q0 - 2*P30*g*q1) - 2*g*q3*(2*P01*g*q2 - 2*P11*g*q3 + 2*P21*g*q0 - 2*P31*g*q1) + 2*g*q0*(2*P02*g*q2 - 2*P12*g*q3 + 2*P22*g*q0 - 2*P32*g*q1) - 2*g*q1*(2*P03*g*q2 - 2*P13*g*q3 + 2*P23*g*q0 - 2*P33*g*q1);
	float G01 = -2*g*q1*(2*P00*g*q2 - 2*P10*g*q3 + 2*P20*g*q0 - 2*P30*g*q1) - 2*g*q0*(2*P01*g*q2 - 2*P11*g*q3 + 2*P21*g*q0 - 2*P31*g*q1) - 2*g*q3*(2*P02*g*q2 - 2*P12*g*q3 + 2*P22*g*q0 - 2*P32*g*q1) - 2*g*q2*(2*P03*g*q2 - 2*P13*g*q3 + 2*P23*g*q0 - 2*P33*g*q1);
	float G02 = 4*g*q1*(2*P01*g*q2 - 2*P11*g*q3 + 2*P21*g*q0 - 2*P31*g*q1) + 4*g*q2*(2*P02*g*q2 - 2*P12*g*q3 + 2*P22*g*q0 - 2*P32*g*q1);
	
	float G10 = 2*g*q3*(2*P01*g*q1 + 2*P11*g*q0 + 2*P21*g*q3 + 2*P31*g*q2) - 2*g*q2*(2*P00*g*q1 + 2*P10*g*q0 + 2*P20*g*q3 + 2*P30*g*q2) - 2*g*q0*(2*P02*g*q1 + 2*P12*g*q0 + 2*P22*g*q3 + 2*P32*g*q2) + 2*g*q1*(2*P03*g*q1 + 2*P13*g*q0 + 2*P23*g*q3 + 2*P33*g*q2);
	float G11 = ekf->Racc + 2*g*q1*(2*P00*g*q1 + 2*P10*g*q0 + 2*P20*g*q3 + 2*P30*g*q2) + 2*g*q0*(2*P01*g*q1 + 2*P11*g*q0 + 2*P21*g*q3 + 2*P31*g*q2) + 2*g*q3*(2*P02*g*q1 + 2*P12*g*q0 + 2*P22*g*q3 + 2*P32*g*q2) + 2*g*q2*(2*P03*g*q1 + 2*P13*g*q0 + 2*P23*g*q3 + 2*P33*g*q2);
	float G12 = -4*g*q1*(2*P01*g*q1 + 2*P11*g*q0 + 2*P21*g*q3 + 2*P31*g*q2) - 4*g*q2*(2*P02*g*q1 + 2*P12*g*q0 + 2*P22*g*q3 + 2*P32*g*q2);
	
	float G20 = 2*g*q2*(4*P10*g*q1 + 4*P20*g*q2) + 2*g*q0*(4*P12*g*q1 + 4*P22*g*q2) - 2*g*q3*(4*P11*g*q1 + 4*P21*g*q2) - 2*g*q1*(4*P13*g*q1 + 4*P23*g*q2);
	float G21 = -2*g*q1*(4*P10*g*q1 + 4*P20*g*q2) - 2*g*q0*(4*P11*g*q1 + 4*P21*g*q2) - 2*g*q3*(4*P12*g*q1 + 4*P22*g*q2) - 2*g*q2*(4*P13*g*q1 + 4*P23*g*q2);
	float G22 = ekf->Racc + 4*g*q1*(4*P11*g*q1 + 4*P21*g*q2) + 4*g*q2*(4*P12*g*q1 + 4*P22*g*q2);

	float det = (G00*G11*G22 - G00*G12*G21 - G01*G10*G22 + G01*G12*G20 + G02*G10*G21 - G02*G11*G20);

	/* Make sure matrix is not singular */
	if (det <= -1e-6 || det >= 1e-6) {

		float idet = 1.0f / det;
		float K[4][3] = {{(2*(2*G10*G21*P01*g*q1 + G10*G22*P00*g*q1 + G10*G22*P01*g*q0 - 2*G11*G20*P01*g*q1 - G12*G20*P00*g*q1 - G12*G20*P01*g*q0 + 2*G10*G21*P02*g*q2 - 2*G11*G20*P02*g*q2 + G11*G22*P00*g*q2 + G11*G22*P02*g*q0 - G12*G21*P00*g*q2 - G12*G21*P02*g*q0 + G10*G22*P02*g*q3 + G10*G22*P03*g*q2 - G11*G22*P01*g*q3 - G11*G22*P03*g*q1 - G12*G20*P02*g*q3 - G12*G20*P03*g*q2 + G12*G21*P01*g*q3 + G12*G21*P03*g*q1)) * idet, -(2*(2*G00*G21*P01*g*q1 + G00*G22*P00*g*q1 + G00*G22*P01*g*q0 - 2*G01*G20*P01*g*q1 - G02*G20*P00*g*q1 - G02*G20*P01*g*q0 + 2*G00*G21*P02*g*q2 - 2*G01*G20*P02*g*q2 + G01*G22*P00*g*q2 + G01*G22*P02*g*q0 - G02*G21*P00*g*q2 - G02*G21*P02*g*q0 + G00*G22*P02*g*q3 + G00*G22*P03*g*q2 - G01*G22*P01*g*q3 - G01*G22*P03*g*q1 - G02*G20*P02*g*q3 - G02*G20*P03*g*q2 + G02*G21*P01*g*q3 + G02*G21*P03*g*q1)) * idet, (2*(2*G00*G11*P01*g*q1 + G00*G12*P00*g*q1 + G00*G12*P01*g*q0 - 2*G01*G10*P01*g*q1 - G02*G10*P00*g*q1 - G02*G10*P01*g*q0 + 2*G00*G11*P02*g*q2 - 2*G01*G10*P02*g*q2 + G01*G12*P00*g*q2 + G01*G12*P02*g*q0 - G02*G11*P00*g*q2 - G02*G11*P02*g*q0 + G00*G12*P02*g*q3 + G00*G12*P03*g*q2 - G01*G12*P01*g*q3 - G01*G12*P03*g*q1 - G02*G10*P02*g*q3 - G02*G10*P03*g*q2 + G02*G11*P01*g*q3 + G02*G11*P03*g*q1)) * idet},
						{(2*(2*G10*G21*P11*g*q1 + G10*G22*P10*g*q1 + G10*G22*P11*g*q0 - 2*G11*G20*P11*g*q1 - G12*G20*P10*g*q1 - G12*G20*P11*g*q0 + 2*G10*G21*P12*g*q2 - 2*G11*G20*P12*g*q2 + G11*G22*P10*g*q2 + G11*G22*P12*g*q0 - G12*G21*P10*g*q2 - G12*G21*P12*g*q0 + G10*G22*P12*g*q3 + G10*G22*P13*g*q2 - G11*G22*P11*g*q3 - G11*G22*P13*g*q1 - G12*G20*P12*g*q3 - G12*G20*P13*g*q2 + G12*G21*P11*g*q3 + G12*G21*P13*g*q1)) * idet, -(2*(2*G00*G21*P11*g*q1 + G00*G22*P10*g*q1 + G00*G22*P11*g*q0 - 2*G01*G20*P11*g*q1 - G02*G20*P10*g*q1 - G02*G20*P11*g*q0 + 2*G00*G21*P12*g*q2 - 2*G01*G20*P12*g*q2 + G01*G22*P10*g*q2 + G01*G22*P12*g*q0 - G02*G21*P10*g*q2 - G02*G21*P12*g*q0 + G00*G22*P12*g*q3 + G00*G22*P13*g*q2 - G01*G22*P11*g*q3 - G01*G22*P13*g*q1 - G02*G20*P12*g*q3 - G02*G20*P13*g*q2 + G02*G21*P11*g*q3 + G02*G21*P13*g*q1)) * idet, (2*(2*G00*G11*P11*g*q1 + G00*G12*P10*g*q1 + G00*G12*P11*g*q0 - 2*G01*G10*P11*g*q1 - G02*G10*P10*g*q1 - G02*G10*P11*g*q0 + 2*G00*G11*P12*g*q2 - 2*G01*G10*P12*g*q2 + G01*G12*P10*g*q2 + G01*G12*P12*g*q0 - G02*G11*P10*g*q2 - G02*G11*P12*g*q0 + G00*G12*P12*g*q3 + G00*G12*P13*g*q2 - G01*G12*P11*g*q3 - G01*G12*P13*g*q1 - G02*G10*P12*g*q3 - G02*G10*P13*g*q2 + G02*G11*P11*g*q3 + G02*G11*P13*g*q1)) * idet},
						{(2*(2*G10*G21*P21*g*q1 + G10*G22*P20*g*q1 + G10*G22*P21*g*q0 - 2*G11*G20*P21*g*q1 - G12*G20*P20*g*q1 - G12*G20*P21*g*q0 + 2*G10*G21*P22*g*q2 - 2*G11*G20*P22*g*q2 + G11*G22*P20*g*q2 + G11*G22*P22*g*q0 - G12*G21*P20*g*q2 - G12*G21*P22*g*q0 + G10*G22*P22*g*q3 + G10*G22*P23*g*q2 - G11*G22*P21*g*q3 - G11*G22*P23*g*q1 - G12*G20*P22*g*q3 - G12*G20*P23*g*q2 + G12*G21*P21*g*q3 + G12*G21*P23*g*q1)) * idet, -(2*(2*G00*G21*P21*g*q1 + G00*G22*P20*g*q1 + G00*G22*P21*g*q0 - 2*G01*G20*P21*g*q1 - G02*G20*P20*g*q1 - G02*G20*P21*g*q0 + 2*G00*G21*P22*g*q2 - 2*G01*G20*P22*g*q2 + G01*G22*P20*g*q2 + G01*G22*P22*g*q0 - G02*G21*P20*g*q2 - G02*G21*P22*g*q0 + G00*G22*P22*g*q3 + G00*G22*P23*g*q2 - G01*G22*P21*g*q3 - G01*G22*P23*g*q1 - G02*G20*P22*g*q3 - G02*G20*P23*g*q2 + G02*G21*P21*g*q3 + G02*G21*P23*g*q1)) * idet, (2*(2*G00*G11*P21*g*q1 + G00*G12*P20*g*q1 + G00*G12*P21*g*q0 - 2*G01*G10*P21*g*q1 - G02*G10*P20*g*q1 - G02*G10*P21*g*q0 + 2*G00*G11*P22*g*q2 - 2*G01*G10*P22*g*q2 + G01*G12*P20*g*q2 + G01*G12*P22*g*q0 - G02*G11*P20*g*q2 - G02*G11*P22*g*q0 + G00*G12*P22*g*q3 + G00*G12*P23*g*q2 - G01*G12*P21*g*q3 - G01*G12*P23*g*q1 - G02*G10*P22*g*q3 - G02*G10*P23*g*q2 + G02*G11*P21*g*q3 + G02*G11*P23*g*q1)) * idet},
						{(2*(2*G10*G21*P31*g*q1 + G10*G22*P30*g*q1 + G10*G22*P31*g*q0 - 2*G11*G20*P31*g*q1 - G12*G20*P30*g*q1 - G12*G20*P31*g*q0 + 2*G10*G21*P32*g*q2 - 2*G11*G20*P32*g*q2 + G11*G22*P30*g*q2 + G11*G22*P32*g*q0 - G12*G21*P30*g*q2 - G12*G21*P32*g*q0 + G10*G22*P32*g*q3 + G10*G22*P33*g*q2 - G11*G22*P31*g*q3 - G11*G22*P33*g*q1 - G12*G20*P32*g*q3 - G12*G20*P33*g*q2 + G12*G21*P31*g*q3 + G12*G21*P33*g*q1)) * idet, -(2*(2*G00*G21*P31*g*q1 + G00*G22*P30*g*q1 + G00*G22*P31*g*q0 - 2*G01*G20*P31*g*q1 - G02*G20*P30*g*q1 - G02*G20*P31*g*q0 + 2*G00*G21*P32*g*q2 - 2*G01*G20*P32*g*q2 + G01*G22*P30*g*q2 + G01*G22*P32*g*q0 - G02*G21*P30*g*q2 - G02*G21*P32*g*q0 + G00*G22*P32*g*q3 + G00*G22*P33*g*q2 - G01*G22*P31*g*q3 - G01*G22*P33*g*q1 - G02*G20*P32*g*q3 - G02*G20*P33*g*q2 + G02*G21*P31*g*q3 + G02*G21*P33*g*q1)) * idet, (2*(2*G00*G11*P31*g*q1 + G00*G12*P30*g*q1 + G00*G12*P31*g*q0 - 2*G01*G10*P31*g*q1 - G02*G10*P30*g*q1 - G02*G10*P31*g*q0 + 2*G00*G11*P32*g*q2 - 2*G01*G10*P32*g*q2 + G01*G12*P30*g*q2 + G01*G12*P32*g*q0 - G02*G11*P30*g*q2 - G02*G11*P32*g*q0 + G00*G12*P32*g*q3 + G00*G12*P33*g*q2 - G01*G12*P31*g*q3 - G01*G12*P33*g*q1 - G02*G10*P32*g*q3 - G02*G10*P33*g*q2 + G02*G11*P31*g*q3 + G02*G11*P33*g*q1)) * idet}};

		/* Output function h(x,u) */
		float hAcc[3] = {2.0f * g * (q0 * q2 - q1 * q3), 
						 ekf->V * ekf->r - 2.0f * g * (q0 * q1 + q2 * q3),
						-ekf->V * ekf->q + 2.0f * g * (q1 * q1 + q2 * q2 - 1.0f)};

		/* Update state estimate */
		int i, j;
		
		for (i = 0; i < 4; i++) {
			
			for (j = 0; j < 3; j++) {
			
				ekf->x[i] += K[i][j] * (acc_mps2[j] - hAcc[j]);
			
			}		
			
		}
		
		/* Normalise quaternion */
		float inorm = 1.0f / sqrt(ekf->x[0] * ekf->x[0] + ekf->x[1] * ekf->x[1]
									+ ekf->x[2] * ekf->x[2] + ekf->x[3] * ekf->x[3]);
								
		ekf->x[0] *= inorm;
		ekf->x[1] *= inorm;
		ekf->x[2] *= inorm;
		ekf->x[3] *= inorm;
		
		/* Re-extract states */
		q0 = ekf->x[0];
		q1 = ekf->x[1];
		q2 = ekf->x[2];
		q3 = ekf->x[3];

		/* Update error covariance matrix */
		ekf->P[0][0] = P00*(2*K[0][1]*g*q1 - 2*K[0][0]*g*q2 + 1) + P30*(2*K[0][0]*g*q1 + 2*K[0][1]*g*q2) + P10*(2*K[0][1]*g*q0 + 2*K[0][0]*g*q3 - 4*K[0][2]*g*q1) - P20*(2*K[0][0]*g*q0 - 2*K[0][1]*g*q3 + 4*K[0][2]*g*q2);
		ekf->P[0][1] = P01*(2*K[0][1]*g*q1 - 2*K[0][0]*g*q2 + 1) + P31*(2*K[0][0]*g*q1 + 2*K[0][1]*g*q2) + P11*(2*K[0][1]*g*q0 + 2*K[0][0]*g*q3 - 4*K[0][2]*g*q1) - P21*(2*K[0][0]*g*q0 - 2*K[0][1]*g*q3 + 4*K[0][2]*g*q2);
		ekf->P[0][2] = P02*(2*K[0][1]*g*q1 - 2*K[0][0]*g*q2 + 1) + P32*(2*K[0][0]*g*q1 + 2*K[0][1]*g*q2) + P12*(2*K[0][1]*g*q0 + 2*K[0][0]*g*q3 - 4*K[0][2]*g*q1) - P22*(2*K[0][0]*g*q0 - 2*K[0][1]*g*q3 + 4*K[0][2]*g*q2);
		ekf->P[0][3] = P03*(2*K[0][1]*g*q1 - 2*K[0][0]*g*q2 + 1) + P33*(2*K[0][0]*g*q1 + 2*K[0][1]*g*q2) + P13*(2*K[0][1]*g*q0 + 2*K[0][0]*g*q3 - 4*K[0][2]*g*q1) - P23*(2*K[0][0]*g*q0 - 2*K[0][1]*g*q3 + 4*K[0][2]*g*q2);
		
		ekf->P[1][0] = P30*(2*K[1][0]*g*q1 + 2*K[1][1]*g*q2) - P00*(2*K[1][0]*g*q2 - 2*K[1][1]*g*q1) + P10*(2*K[1][1]*g*q0 + 2*K[1][0]*g*q3 - 4*K[1][2]*g*q1 + 1) - P20*(2*K[1][0]*g*q0 - 2*K[1][1]*g*q3 + 4*K[1][2]*g*q2);
		ekf->P[1][1] = P31*(2*K[1][0]*g*q1 + 2*K[1][1]*g*q2) - P01*(2*K[1][0]*g*q2 - 2*K[1][1]*g*q1) + P11*(2*K[1][1]*g*q0 + 2*K[1][0]*g*q3 - 4*K[1][2]*g*q1 + 1) - P21*(2*K[1][0]*g*q0 - 2*K[1][1]*g*q3 + 4*K[1][2]*g*q2);
		ekf->P[1][2] = P32*(2*K[1][0]*g*q1 + 2*K[1][1]*g*q2) - P02*(2*K[1][0]*g*q2 - 2*K[1][1]*g*q1) + P12*(2*K[1][1]*g*q0 + 2*K[1][0]*g*q3 - 4*K[1][2]*g*q1 + 1) - P22*(2*K[1][0]*g*q0 - 2*K[1][1]*g*q3 + 4*K[1][2]*g*q2);
		ekf->P[1][3] = P33*(2*K[1][0]*g*q1 + 2*K[1][1]*g*q2) - P03*(2*K[1][0]*g*q2 - 2*K[1][1]*g*q1) + P13*(2*K[1][1]*g*q0 + 2*K[1][0]*g*q3 - 4*K[1][2]*g*q1 + 1) - P23*(2*K[1][0]*g*q0 - 2*K[1][1]*g*q3 + 4*K[1][2]*g*q2);
		
		ekf->P[2][0] = P30*(2*K[2][0]*g*q1 + 2*K[2][1]*g*q2) - P00*(2*K[2][0]*g*q2 - 2*K[2][1]*g*q1) + P10*(2*K[2][1]*g*q0 + 2*K[2][0]*g*q3 - 4*K[2][2]*g*q1) - P20*(2*K[2][0]*g*q0 - 2*K[2][1]*g*q3 + 4*K[2][2]*g*q2 - 1);
		ekf->P[2][1] = P31*(2*K[2][0]*g*q1 + 2*K[2][1]*g*q2) - P01*(2*K[2][0]*g*q2 - 2*K[2][1]*g*q1) + P11*(2*K[2][1]*g*q0 + 2*K[2][0]*g*q3 - 4*K[2][2]*g*q1) - P21*(2*K[2][0]*g*q0 - 2*K[2][1]*g*q3 + 4*K[2][2]*g*q2 - 1);
		ekf->P[2][2] = P32*(2*K[2][0]*g*q1 + 2*K[2][1]*g*q2) - P02*(2*K[2][0]*g*q2 - 2*K[2][1]*g*q1) + P12*(2*K[2][1]*g*q0 + 2*K[2][0]*g*q3 - 4*K[2][2]*g*q1) - P22*(2*K[2][0]*g*q0 - 2*K[2][1]*g*q3 + 4*K[2][2]*g*q2 - 1);
		ekf->P[2][3] = P33*(2*K[2][0]*g*q1 + 2*K[2][1]*g*q2) - P03*(2*K[2][0]*g*q2 - 2*K[2][1]*g*q1) + P13*(2*K[2][1]*g*q0 + 2*K[2][0]*g*q3 - 4*K[2][2]*g*q1) - P23*(2*K[2][0]*g*q0 - 2*K[2][1]*g*q3 + 4*K[2][2]*g*q2 - 1);
		
		ekf->P[3][0] = P30*(2*K[3][0]*g*q1 + 2*K[3][1]*g*q2 + 1) - P00*(2*K[3][0]*g*q2 - 2*K[3][1]*g*q1) + P10*(2*K[3][1]*g*q0 + 2*K[3][0]*g*q3 - 4*K[3][2]*g*q1) - P20*(2*K[3][0]*g*q0 - 2*K[3][1]*g*q3 + 4*K[3][2]*g*q2);
		ekf->P[3][1] = P31*(2*K[3][0]*g*q1 + 2*K[3][1]*g*q2 + 1) - P01*(2*K[3][0]*g*q2 - 2*K[3][1]*g*q1) + P11*(2*K[3][1]*g*q0 + 2*K[3][0]*g*q3 - 4*K[3][2]*g*q1) - P21*(2*K[3][0]*g*q0 - 2*K[3][1]*g*q3 + 4*K[3][2]*g*q2);
		ekf->P[3][2] = P32*(2*K[3][0]*g*q1 + 2*K[3][1]*g*q2 + 1) - P02*(2*K[3][0]*g*q2 - 2*K[3][1]*g*q1) + P12*(2*K[3][1]*g*q0 + 2*K[3][0]*g*q3 - 4*K[3][2]*g*q1) - P22*(2*K[3][0]*g*q0 - 2*K[3][1]*g*q3 + 4*K[3][2]*g*q2);
		ekf->P[3][3] = P33*(2*K[3][0]*g*q1 + 2*K[3][1]*g*q2 + 1) - P03*(2*K[3][0]*g*q2 - 2*K[3][1]*g*q1) + P13*(2*K[3][1]*g*q0 + 2*K[3][0]*g*q3 - 4*K[3][2]*g*q1) - P23*(2*K[3][0]*g*q0 - 2*K[3][1]*g*q3 + 4*K[3][2]*g*q2);
		
		return 1;
		
	} else {
		
		return 0;
		
	}

}

uint8_t EKF_UpdateHeading(EKF *ekf, float psi_rad) {
	
	/* Extract states */
	float q0 = ekf->x[0];
	float q1 = ekf->x[1];
	float q2 = ekf->x[2];
	float q3 = ekf->x[3];
	
	/* Output function h(x,u) */
	float hMag = atan2(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3));
	
	/* Jacobian of h(x,u) */
	float fact = (2*q2*q2 + 2*q3*q3 - 1);
	float iden = 1.0f / (fact*fact + (2*q0*q3 + 2*q1*q2)*(2*q0*q3 + 2*q1*q2));
	 
	float C[] = {-(2*q3*fact)*iden,
				 -(2*q2*fact)*iden,
				 -((2*q1)*fact - 4*q2*(2*q0*q3 + 2*q1*q2))*iden,
				 -((2*q0)*fact - 4*q3*(2*q0*q3 + 2*q1*q2))*iden};
				 
	/* Kalman gain */
	float P00 = ekf->P[0][0]; float P01 = ekf->P[0][1]; float P02 = ekf->P[0][2]; float P03 = ekf->P[0][3];
	float P10 = ekf->P[1][0]; float P11 = ekf->P[1][1]; float P12 = ekf->P[1][2]; float P13 = ekf->P[1][3];
	float P20 = ekf->P[2][0]; float P21 = ekf->P[2][1]; float P22 = ekf->P[2][2]; float P23 = ekf->P[2][3];
	float P30 = ekf->P[3][0]; float P31 = ekf->P[3][1]; float P32 = ekf->P[3][2]; float P33 = ekf->P[3][3];
	
	float det = ekf->Rheading 
				+ C[0]*(C[0]*P00 + C[1]*P10 + C[2]*P20 + C[3]*P30)
				+ C[1]*(C[0]*P01 + C[1]*P11 + C[2]*P21 + C[3]*P31)
				+ C[2]*(C[0]*P02 + C[1]*P12 + C[2]*P22 + C[3]*P32)
				+ C[3]*(C[0]*P03 + C[1]*P13 + C[2]*P23 + C[3]*P33);
	
	/* Make sure matrix is not singular */
	if (det <= -1e-6 || det >= 1e-6) {
	
		float idet = 1.0f / det;
		float K[] = {(C[0]*P00 + C[1]*P01 + C[2]*P02 + C[3]*P03) * idet,
					 (C[0]*P10 + C[1]*P11 + C[2]*P12 + C[3]*P13) * idet,
					 (C[0]*P20 + C[1]*P21 + C[2]*P22 + C[3]*P23) * idet,
					 (C[0]*P30 + C[1]*P31 + C[2]*P32 + C[3]*P33) * idet };
				
		/* Update state estimate */
		int i;
		
		for (i = 0; i < 4; i++) {
			
			ekf->x[i] += K[i] * (psi_rad - hMag);	
			
		}
		
		/* Normalise quaternion */
		float inorm = 1.0f / sqrt(ekf->x[0] * ekf->x[0] + ekf->x[1] * ekf->x[1]
									+ ekf->x[2] * ekf->x[2] + ekf->x[3] * ekf->x[3]);
								
		ekf->x[0] *= inorm;
		ekf->x[1] *= inorm;
		ekf->x[2] *= inorm;
		ekf->x[3] *= inorm;
		
		/* Re-extract states */
		q0 = ekf->x[0];
		q1 = ekf->x[1];
		q2 = ekf->x[2];
		q3 = ekf->x[3];

		/* Update error covariance matrix */
		ekf->P[0][0] = -P00*(C[0]*K[0] - 1) - C[1]*K[0]*P10 - C[2]*K[0]*P20 - C[3]*K[0]*P30;
		ekf->P[0][1] = -P01*(C[0]*K[0] - 1) - C[1]*K[0]*P11 - C[2]*K[0]*P21 - C[3]*K[0]*P31;
		ekf->P[0][2] = -P02*(C[0]*K[0] - 1) - C[1]*K[0]*P12 - C[2]*K[0]*P22 - C[3]*K[0]*P32;
		ekf->P[0][3] = -P03*(C[0]*K[0] - 1) - C[1]*K[0]*P13 - C[2]*K[0]*P23 - C[3]*K[0]*P33;
		
		ekf->P[1][0] = -P10*(C[1]*K[1] - 1) - C[0]*K[1]*P00 - C[2]*K[1]*P20 - C[3]*K[1]*P30;
		ekf->P[1][1] = -P11*(C[1]*K[1] - 1) - C[0]*K[1]*P01 - C[2]*K[1]*P21 - C[3]*K[1]*P31;
		ekf->P[1][2] = -P12*(C[1]*K[1] - 1) - C[0]*K[1]*P02 - C[2]*K[1]*P22 - C[3]*K[1]*P32;
		ekf->P[1][3] = -P13*(C[1]*K[1] - 1) - C[0]*K[1]*P03 - C[2]*K[1]*P23 - C[3]*K[1]*P33;
		
		ekf->P[2][0] = -P20*(C[2]*K[2] - 1) - C[0]*K[2]*P00 - C[1]*K[2]*P10 - C[3]*K[2]*P30;
		ekf->P[2][1] = -P21*(C[2]*K[2] - 1) - C[0]*K[2]*P01 - C[1]*K[2]*P11 - C[3]*K[2]*P31;
		ekf->P[2][2] = -P22*(C[2]*K[2] - 1) - C[0]*K[2]*P02 - C[1]*K[2]*P12 - C[3]*K[2]*P32;
		ekf->P[2][3] = -P23*(C[2]*K[2] - 1) - C[0]*K[2]*P03 - C[1]*K[2]*P13 - C[3]*K[2]*P33;

 		ekf->P[3][0] = -P30*(C[3]*K[3] - 1) - C[0]*K[3]*P00 - C[1]*K[3]*P10 - C[2]*K[3]*P20;
		ekf->P[3][1] = -P31*(C[3]*K[3] - 1) - C[0]*K[3]*P01 - C[1]*K[3]*P11 - C[2]*K[3]*P21;
		ekf->P[3][2] = -P32*(C[3]*K[3] - 1) - C[0]*K[3]*P02 - C[1]*K[3]*P12 - C[2]*K[3]*P22;
		ekf->P[3][3] = -P33*(C[3]*K[3] - 1) - C[0]*K[3]*P03 - C[1]*K[3]*P13 - C[2]*K[3]*P23;
				
		return 1;
				
	} else {
		
		return 0;
		
	}	
}
