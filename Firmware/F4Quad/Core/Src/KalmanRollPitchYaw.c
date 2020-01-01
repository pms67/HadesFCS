#include "KalmanRollPitchYaw.h"

void KalmanRollPitchYaw_Init(KalmanRollPitchYaw *kal, float Pinit, float *Q, float Racc, float Rpsi) {
	kal->phi   = 0.0f;
	kal->theta = 0.0f;
	kal->P[0] = Pinit; kal->P[1] = 0.0f;  kal->P[2] = 0.0f;
	kal->P[3] = 0.0f;  kal->P[4] = Pinit; kal->P[5] = 0.0f;
	kal->P[6] = 0.0f;  kal->P[7] = 0.0f;  kal->P[8] = Pinit;
	kal->Q[0] = Q[0];  kal->Q[1] = Q[1];  kal->Q[2] = Q[2];
	kal->Racc = Racc;
	kal->Rpsi = Rpsi;
	kal->gyr[0] = 0.0f;
	kal->gyr[1] = 0.0f;
	kal->gyr[2] = 0.0f;
}

void KalmanRollPitchYaw_Predict(KalmanRollPitchYaw *kal, float *gyr, float T) {
	/* Extract measurements */
	float p = gyr[0];
	float q = gyr[1];
	float r = gyr[2];

	kal->gyr[0] = gyr[0];
	kal->gyr[1] = gyr[1];
	kal->gyr[2] = gyr[2];

	/* Compute common trig terms */
	float sp = sin(kal->phi);   float cp = cos(kal->phi);

	/* tan(theta) is undefined for theta=90deg */
	if (fabs(kal->theta) > 1.57952297305f || fabs(kal->theta) < 1.56206968053f) {
		return;
	}

	/* x+ = x- + T * f(x,u) */
	kal->phi   = kal->phi   + T * (p + tan(kal->theta) * (q * sp + r * cp));
	kal->theta = kal->theta + T * (          q * cp - r * sp);
	kal->psi   = kal->psi   + T * (			 q * sp + r * cp) / cos(kal->theta);

	if (fabs(kal->theta) > 1.57952297305f || fabs(kal->theta) < 1.56206968053f) {
		return;
	}

	/* Recompute common trig terms using new state estimates */
          sp = sin(kal->phi);		  cp = cos(kal->phi);
    float st = sin(kal->theta); float ct = cos(kal->theta); float tt = st / ct;

	/* Jacobian of f(x,u) */
	float A[9] = { tt * (q * cp - r * sp), (r * cp + q * sp) * (tt * tt + 1.0f), 0.0f
				  -(r * cp + q * sp),      0.0f,								 0.0f,
				  (q * cp - r * sp) / ct,  (q * sp + r * cp) * st / (ct * ct),	 0.0f};

	/* Update covariance matrix P+ = P- + T * (A*P- + P-*A' + Q) */
	float Ptmp[9] = {kal->Q[0] + 2*A[0]*kal->P[0] + A[1]*kal->P[1] + A[1]*kal->P[3], A[0]*kal->P[1] + A[3]*kal->P[0] + A[1]*kal->P[4], A[0]*kal->P[2] + A[1]*kal->P[5] + A[6]*kal->P[0] + A[7]*kal->P[1],
			         A[0]*kal->P[3] + A[3]*kal->P[0] + A[1]*kal->P[4], kal->Q[1] + A[3]*kal->P[1] + A[3]*kal->P[3], A[3]*kal->P[2] + A[6]*kal->P[3] + A[7]*kal->P[4],
					 A[0]*kal->P[6] + A[6]*kal->P[0] + A[1]*kal->P[7] + A[7]*kal->P[3], A[6]*kal->P[1] + A[3]*kal->P[6] + A[7]*kal->P[4], kal->Q[2] + A[6]*kal->P[2] + A[6]*kal->P[6] + A[7]*kal->P[5] + A[7]*kal->P[7]};

	kal->P[0] += T * Ptmp[0]; kal->P[1] += T * Ptmp[1]; kal->P[2] += T * Ptmp[2];
	kal->P[3] += T * Ptmp[3]; kal->P[4] += T * Ptmp[4]; kal->P[5] += T * Ptmp[5];
	kal->P[6] += T * Ptmp[6]; kal->P[7] += T * Ptmp[7]; kal->P[8] += T * Ptmp[8];
}

uint8_t KalmanRollPitchYaw_UpdateAcc(KalmanRollPitchYaw *kal, float *acc, float Va) {
	/* Extract measurements */
	float p = kal->gyr[0];
	float q = kal->gyr[1];
	float r = kal->gyr[2];

	/* Compute common trig terms */
	float sp = sin(kal->phi);   float cp = cos(kal->phi);
	float st = sin(kal->theta); float ct = cos(kal->theta);

	/* Output function h(x,u) */
	float h[3] = { q * Va * st               + g * st,
				   r * Va * ct - p * Va * st - g * ct * sp,
				  -q * Va * ct               - g * ct * cp };

	/* Jacobian of h(x,u) */
	float C[9] = { 0.0f,         q * Va * ct + g * ct, 0.0f
				  -g * cp * ct, -r * Va * st - p * Va * ct + g * sp * st, 0.0f,
				   g * sp * ct, (q * Va + g * cp) * st, 0.0f };

	float Racc = kal->Racc;
	float P[9] = {kal->P[0], kal->P[1], kal->P[2],
				  kal->P[3], kal->P[4], kal->P[5],
				  kal->P[6], kal->P[7], kal->P[8]};

	float det = (Racc*Racc + C[3]*C[3]*P[0]*Racc + C[1]*C[1]*P[4]*Racc + C[6]*C[6]*P[0]*Racc + C[4]*C[4]*P[4]*Racc + C[7]*C[7]*P[4]*Racc + C[1]*C[1]*C[3]*C[3]*P[0]*P[4] - C[1]*C[1]*C[3]*C[3]*P[1]*P[3] + C[1]*C[1]*C[6]*C[6]*P[0]*P[4] - C[1]*C[1]*C[6]*C[6]*P[1]*P[3] + C[3]*C[3]*C[7]*C[7]*P[0]*P[4] - C[3]*C[3]*C[7]*C[7]*P[1]*P[3] + C[4]*C[4]*C[6]*C[6]*P[0]*P[4] - C[4]*C[4]*C[6]*C[6]*P[1]*P[3] + C[3]*C[4]*P[1]*Racc + C[3]*C[4]*P[3]*Racc + C[6]*C[7]*P[1]*Racc + C[6]*C[7]*P[3]*Racc - 2*C[3]*C[4]*C[6]*C[7]*P[0]*P[4] + 2*C[3]*C[4]*C[6]*C[7]*P[1]*P[3]);

	/* Ensure matrix is not singular */
	if (det < -0.00001f || det > 0.00001f) {
		float idet = 1.0f / det;


		/* Kalman gain K = P * C' / (C * P * C' + R) */
		float K[3][3] = { { (C[1]*P[1]*Racc - C[1]*C[3]*C[4]*P[0]*P[4] + C[1]*C[3]*C[4]*P[1]*P[3] - C[1]*C[6]*C[7]*P[0]*P[4] + C[1]*C[6]*C[7]*P[1]*P[3]) * idet, (C[3]*P[0]*Racc + C[4]*P[1]*Racc + C[1]*C[1]*C[3]*P[0]*P[4] - C[1]*C[1]*C[3]*P[1]*P[3] + C[3]*C[7]*C[7]*P[0]*P[4] - C[3]*C[7]*C[7]*P[1]*P[3] - C[4]*C[6]*C[7]*P[0]*P[4] + C[4]*C[6]*C[7]*P[1]*P[3])* idet, (C[6]*P[0]*Racc + C[7]*P[1]*Racc + C[1]*C[1]*C[6]*P[0]*P[4] - C[1]*C[1]*C[6]*P[1]*P[3] + C[4]*C[4]*C[6]*P[0]*P[4] - C[4]*C[4]*C[6]*P[1]*P[3] - C[3]*C[4]*C[7]*P[0]*P[4] + C[3]*C[4]*C[7]*P[1]*P[3])* idet},
					      { (C[1]*P[4]*Racc + C[1]*C[3]*C[3]*P[0]*P[4] - C[1]*C[3]*C[3]*P[1]*P[3] + C[1]*C[6]*C[6]*P[0]*P[4] - C[1]*C[6]*C[6]*P[1]*P[3]) * idet, (C[3]*P[3]*Racc + C[4]*P[4]*Racc + C[4]*C[6]*C[6]*P[0]*P[4] - C[4]*C[6]*C[6]*P[1]*P[3] - C[3]*C[6]*C[7]*P[0]*P[4] + C[3]*C[6]*C[7]*P[1]*P[3])* idet, (C[6]*P[3]*Racc + C[7]*P[4]*Racc + C[3]*C[3]*C[7]*P[0]*P[4] - C[3]*C[3]*C[7]*P[1]*P[3] - C[3]*C[4]*C[6]*P[0]*P[4] + C[3]*C[4]*C[6]*P[1]*P[3])* idet},
					      { (C[1]*P[7]*Racc + C[1]*C[3]*C[3]*P[0]*P[7] - C[1]*C[3]*C[3]*P[1]*P[6] + C[1]*C[6]*C[6]*P[0]*P[7] - C[1]*C[6]*C[6]*P[1]*P[6] + C[1]*C[3]*C[4]*P[3]*P[7] - C[1]*C[3]*C[4]*P[4]*P[6] + C[1]*C[6]*C[7]*P[3]*P[7] - C[1]*C[6]*C[7]*P[4]*P[6])* idet, (C[3]*P[6]*Racc + C[4]*P[7]*Racc - C[1]*C[1]*C[3]*P[3]*P[7] + C[1]*C[1]*C[3]*P[4]*P[6] + C[4]*C[6]*C[6]*P[0]*P[7] - C[4]*C[6]*C[6]*P[1]*P[6] - C[3]*C[7]*C[7]*P[3]*P[7] + C[3]*C[7]*C[7]*P[4]*P[6] - C[3]*C[6]*C[7]*P[0]*P[7] + C[3]*C[6]*C[7]*P[1]*P[6] + C[4]*C[6]*C[7]*P[3]*P[7] - C[4]*C[6]*C[7]*P[4]*P[6])* idet, (C[6]*P[6]*Racc + C[7]*P[7]*Racc - C[1]*C[1]*C[6]*P[3]*P[7] + C[1]*C[1]*C[6]*P[4]*P[6] + C[3]*C[3]*C[7]*P[0]*P[7] - C[3]*C[3]*C[7]*P[1]*P[6] - C[4]*C[4]*C[6]*P[3]*P[7] + C[4]*C[4]*C[6]*P[4]*P[6] - C[3]*C[4]*C[6]*P[0]*P[7] + C[3]*C[4]*C[6]*P[1]*P[6] + C[3]*C[4]*C[7]*P[3]*P[7] - C[3]*C[4]*C[7]*P[4]*P[6])* idet} };

		/* Update state estimates */
		for (int i = 0; i < 3; i++) {
			kal->phi   += K[0][i] * (acc[i] - h[i]);
			kal->theta += K[1][i] * (acc[i] - h[i]);
			kal->psi   += K[2][i] * (acc[i] - h[i]);
		}

		/* Update error covariance matrix */
		kal->P[0] = -P[3]*(C[1]*K[0][0] + C[4]*K[0][1] + C[7]*K[0][2]) - P[0]*(C[3]*K[0][1] + C[6]*K[0][2]- 1.0f);
		kal->P[1] = -P[4]*(C[1]*K[0][0] + C[4]*K[0][1] + C[7]*K[0][2]) - P[1]*(C[3]*K[0][1] + C[6]*K[0][2] - 1.0f);
		kal->P[2] = -P[5]*(C[1]*K[0][0] + C[4]*K[0][1] + C[7]*K[0][2]) - P[2]*(C[3]*K[0][1] + C[6]*K[0][2] - 1.0f);

		kal->P[3] = -P[3]*(C[1]*K[1][0] + C[4]*K[1][1] + C[7]*K[1][2] - 1.0f) - P[0]*(C[3]*K[1][1] + C[6]*K[1][2]);
		kal->P[4] = -P[4]*(C[1]*K[1][0] + C[4]*K[1][1] + C[7]*K[1][2] - 1.0f) - P[1]*(C[3]*K[1][1] + C[6]*K[1][2]);
		kal->P[5] = -P[5]*(C[1]*K[1][0] + C[4]*K[1][1] + C[7]*K[1][2] - 1.0f) - P[2]*(C[3]*K[1][1] + C[6]*K[1][2]);

		kal->P[6] = P[6] - P[3]*(C[1]*K[2][0] + C[4]*K[2][1] + C[7]*K[2][2]) - P[0]*(C[3]*K[2][1] + C[6]*K[2][2]);
		kal->P[7] = P[7] - P[4]*(C[1]*K[2][0] + C[4]*K[2][1] + C[7]*K[2][2]) - P[1]*(C[3]*K[2][1] + C[6]*K[2][2]);
		kal->P[8] = P[8] - P[5]*(C[1]*K[2][0] + C[4]*K[2][1] + C[7]*K[2][2]) - P[2]*(C[3]*K[2][1] + C[6]*K[2][2]);

		return 1;
	}

	return 0;
}

uint8_t KalmanRollPitchYaw_UpdatePsi(KalmanRollPitchYaw *kal, float psi) {

	float det = kal->P[8] + kal->Rpsi;

	/* Ensure matrix is not singular */
	if (det < -0.00001f || det > 0.00001f) {
		float idet = 1.0f / det;

		/* Kalman gain K = P * C' / (C * P * C' + R) */
		float K[] = {kal->P[2] * idet, kal->P[5] * idet, kal->P[8] * idet};

		/* Update state estimates */
		kal->phi   += K[0] * (psi - kal->psi);
		kal->theta += K[1] * (psi - kal->psi);
		kal->psi   += K[2] * (psi - kal->psi);

		/* Update error covariance matrix */
		kal->P[0] =  kal->P[0] - K[0]*kal->P[6]; kal->P[1] =  kal->P[1] - K[0]*kal->P[7]; kal->P[2] =  kal->P[2] - K[0]*kal->P[8];
		kal->P[3] =  kal->P[3] - K[1]*kal->P[6]; kal->P[4] =  kal->P[4] - K[1]*kal->P[7]; kal->P[5] =  kal->P[5] - K[1]*kal->P[8];
		kal->P[6] = -kal->P[6]*(K[2] - 1.0f);    kal->P[7] = -kal->P[7]*(K[2] - 1.0f);    kal->P[8] = -kal->P[8]*(K[2] - 1.0f);

		return 1;
	}

	return 0;
}
