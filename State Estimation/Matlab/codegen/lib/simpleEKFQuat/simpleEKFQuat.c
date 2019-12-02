/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * simpleEKFQuat.c
 *
 * Code generation for function 'simpleEKFQuat'
 *
 */

/* Include files */
#include <string.h>
#include <math.h>
#include "simpleEKFQuat.h"

/* Variable Definitions */
static double p;
static double q;
static double r;
static double ax;
static double ay;
static double az;
static double Va;
static double lpfGyr;
static double lpfAcc;
static double lpfVa;
static double x[4];
static double P[16];
static double Q[16];
static double R[9];
static double g;

/* Function Declarations */
static void simpleEKFQuat_init(void);

/* Function Definitions */
static void simpleEKFQuat_init(void)
{
  g = 9.81;

  /*  Low-pass filtered measurements */
  p = 0.0;
  q = 0.0;
  r = 0.0;
  ax = 0.0;
  ay = 0.0;
  az = 0.0;
  Va = 0.0;

  /*  Low-pass filter coefficients */
  lpfGyr = 0.7;
  lpfAcc = 0.9;
  lpfVa = 0.7;

  /*  Initialise state estimate vector */
  x[0] = 0.0;
  x[1] = 0.0;
  x[2] = 0.0;
  x[3] = 0.0;
  x[0] = 1.0;

  /*  Initialise covariance matrix */
  memset(&P[0], 0, sizeof(double) << 4);
  P[0] = 1.0E-6;
  P[5] = 1.0E-6;
  P[10] = 1.0E-6;
  P[15] = 1.0E-6;

  /*  Process noise matrix */
  memset(&Q[0], 0, sizeof(double) << 4);
  Q[0] = 2.5E-9;
  Q[5] = 2.5E-9;
  Q[10] = 2.5E-9;
  Q[15] = 2.5E-9;

  /*  Measurement noise matrix */
  memset(&R[0], 0, 9U * sizeof(double));
  R[0] = 0.0025000000000000005;
  R[4] = 0.0025000000000000005;
  R[8] = 0.0025000000000000005;
}

void simpleEKFQuat(const double gyr_rps[3], const double acc_mps2[3], double
                   Va_mps, double T, double *rollDeg, double *pitchDeg)
{
  double qNorm;
  double K[12];
  double a21;
  double K_tmp;
  int b_K_tmp;
  double A[16];
  int c_K_tmp;
  double b_A[16];
  double dv0[16];
  int d_K_tmp;
  int rtemp;
  double C[12];
  int e_K_tmp;
  int r1;
  int r2;
  double y[12];
  int r3;
  double B[9];
  double b_C[12];
  double f_K_tmp;
  double g_K_tmp;
  double h_K_tmp;
  int i_K_tmp;

  /*  Get measurements and low-pass filter them */
  p = lpfGyr * p + (1.0 - lpfGyr) * gyr_rps[0];
  q = lpfGyr * q + (1.0 - lpfGyr) * gyr_rps[1];
  r = lpfGyr * r + (1.0 - lpfGyr) * gyr_rps[2];
  ax = lpfAcc * ax + (1.0 - lpfAcc) * acc_mps2[0];
  ay = lpfAcc * ay + (1.0 - lpfAcc) * acc_mps2[1];
  az = lpfAcc * az + (1.0 - lpfAcc) * acc_mps2[2];
  Va = lpfVa * Va + (1.0 - lpfVa) * Va_mps;

  /*  Extract states */
  /*  State transition function, xdot = f(x, u)                          */
  /*  Update state estimate */
  qNorm = 0.5 * -x[1];
  K[0] = qNorm;
  a21 = 0.5 * -x[2];
  K[4] = a21;
  K_tmp = 0.5 * -x[3];
  K[8] = K_tmp;
  K[1] = 0.5 * x[0];
  K[5] = K_tmp;
  K[9] = 0.5 * x[2];
  K[2] = 0.5 * x[3];
  K[6] = 0.5 * x[0];
  K[10] = qNorm;
  K[3] = a21;
  K[7] = 0.5 * x[1];
  K[11] = 0.5 * x[0];
  for (b_K_tmp = 0; b_K_tmp < 4; b_K_tmp++) {
    x[b_K_tmp] += T * ((K[b_K_tmp] * p + K[b_K_tmp + 4] * q) + K[b_K_tmp + 8] *
                       r);
  }

  /*  Normalise quaternion */
  qNorm = sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] /= qNorm;
  x[1] /= qNorm;
  x[2] /= qNorm;
  x[3] /= qNorm;

  /*  Re-extract states */
  /*  Compute Jacobian of f, A(x, u) */
  A[0] = 0.0;
  A[4] = -0.5 * p;
  A[8] = -0.5 * q;
  A[12] = -0.5 * r;
  A[1] = 0.5 * p;
  A[5] = 0.0;
  A[9] = 0.5 * r;
  A[13] = -0.5 * q;
  A[2] = 0.5 * q;
  A[6] = -0.5 * r;
  A[10] = 0.0;
  A[14] = 0.5 * p;
  A[3] = 0.5 * r;
  A[7] = 0.5 * q;
  A[11] = -0.5 * p;
  A[15] = 0.0;

  /*  Update error covariance matrix */
  for (b_K_tmp = 0; b_K_tmp < 4; b_K_tmp++) {
    for (c_K_tmp = 0; c_K_tmp < 4; c_K_tmp++) {
      d_K_tmp = c_K_tmp << 2;
      rtemp = b_K_tmp + d_K_tmp;
      dv0[rtemp] = 0.0;
      dv0[rtemp] = ((P[b_K_tmp] * A[c_K_tmp] + P[b_K_tmp + 4] * A[c_K_tmp + 4])
                    + P[b_K_tmp + 8] * A[c_K_tmp + 8]) + P[b_K_tmp + 12] *
        A[c_K_tmp + 12];
      b_A[rtemp] = ((A[b_K_tmp] * P[d_K_tmp] + A[b_K_tmp + 4] * P[1 + d_K_tmp])
                    + A[b_K_tmp + 8] * P[2 + d_K_tmp]) + A[b_K_tmp + 12] * P[3 +
        d_K_tmp];
    }
  }

  for (b_K_tmp = 0; b_K_tmp < 16; b_K_tmp++) {
    P[b_K_tmp] += T * ((b_A[b_K_tmp] + dv0[b_K_tmp]) + Q[b_K_tmp]);
  }

  /*  Compute accelerometer output estimates */
  /*  Note: assuming here that u = Va, v = 0, w = 0 */
  /*  Would be good to set u = Va * cos(theta), v = 0, w = Va * sin(theta) */
  /*  But need expressions for cos(theta) as quaternions... */
  /*  Jacobian of z, C(x, u)     */
  C[0] = 2.0 * g * x[2];
  qNorm = -2.0 * g * x[3];
  C[3] = qNorm;
  C[6] = 2.0 * g * x[0];
  a21 = -2.0 * g * x[1];
  C[9] = a21;
  C[1] = a21;
  C[4] = -2.0 * g * x[0];
  C[7] = qNorm;
  C[10] = -2.0 * g * x[2];
  C[2] = 0.0;
  C[5] = 4.0 * g * x[1];
  C[8] = 4.0 * g * x[2];
  C[11] = 0.0;

  /*  Kalman gain */
  for (b_K_tmp = 0; b_K_tmp < 3; b_K_tmp++) {
    e_K_tmp = b_K_tmp << 2;
    K[e_K_tmp] = C[b_K_tmp];
    K[1 + e_K_tmp] = C[b_K_tmp + 3];
    K[2 + e_K_tmp] = C[b_K_tmp + 6];
    K[3 + e_K_tmp] = C[b_K_tmp + 9];
  }

  for (b_K_tmp = 0; b_K_tmp < 4; b_K_tmp++) {
    for (c_K_tmp = 0; c_K_tmp < 3; c_K_tmp++) {
      d_K_tmp = c_K_tmp << 2;
      y[b_K_tmp + d_K_tmp] = ((P[b_K_tmp] * K[d_K_tmp] + P[b_K_tmp + 4] * K[1 +
        d_K_tmp]) + P[b_K_tmp + 8] * K[2 + d_K_tmp]) + P[b_K_tmp + 12] * K[3 +
        d_K_tmp];
    }
  }

  for (b_K_tmp = 0; b_K_tmp < 3; b_K_tmp++) {
    for (c_K_tmp = 0; c_K_tmp < 4; c_K_tmp++) {
      d_K_tmp = c_K_tmp << 2;
      b_C[b_K_tmp + 3 * c_K_tmp] = ((C[b_K_tmp] * P[d_K_tmp] + C[b_K_tmp + 3] *
        P[1 + d_K_tmp]) + C[b_K_tmp + 6] * P[2 + d_K_tmp]) + C[b_K_tmp + 9] * P
        [3 + d_K_tmp];
    }

    for (c_K_tmp = 0; c_K_tmp < 3; c_K_tmp++) {
      d_K_tmp = c_K_tmp << 2;
      rtemp = b_K_tmp + 3 * c_K_tmp;
      B[rtemp] = (((b_C[b_K_tmp] * K[d_K_tmp] + b_C[b_K_tmp + 3] * K[1 + d_K_tmp])
                   + b_C[b_K_tmp + 6] * K[2 + d_K_tmp]) + b_C[b_K_tmp + 9] * K[3
                  + d_K_tmp]) + R[rtemp];
    }
  }

  r1 = 0;
  r2 = 1;
  r3 = 2;
  qNorm = fabs(B[0]);
  a21 = fabs(B[1]);
  if (a21 > qNorm) {
    qNorm = a21;
    r1 = 1;
    r2 = 0;
  }

  if (fabs(B[2]) > qNorm) {
    r1 = 2;
    r2 = 1;
    r3 = 0;
  }

  B[r2] /= B[r1];
  B[r3] /= B[r1];
  B[3 + r2] -= B[r2] * B[3 + r1];
  B[3 + r3] -= B[r3] * B[3 + r1];
  B[6 + r2] -= B[r2] * B[6 + r1];
  B[6 + r3] -= B[r3] * B[6 + r1];
  if (fabs(B[3 + r3]) > fabs(B[3 + r2])) {
    rtemp = r2;
    r2 = r3;
    r3 = rtemp;
  }

  B[3 + r3] /= B[3 + r2];
  B[6 + r3] -= B[3 + r3] * B[6 + r2];
  e_K_tmp = r1 << 2;
  K[e_K_tmp] = y[0] / B[r1];
  rtemp = r2 << 2;
  qNorm = B[3 + r1];
  K[rtemp] = y[4] - K[e_K_tmp] * qNorm;
  d_K_tmp = r3 << 2;
  a21 = B[6 + r1];
  K[d_K_tmp] = y[8] - K[e_K_tmp] * a21;
  K_tmp = B[3 + r2];
  K[rtemp] /= K_tmp;
  f_K_tmp = B[6 + r2];
  K[d_K_tmp] -= K[rtemp] * f_K_tmp;
  g_K_tmp = B[6 + r3];
  K[d_K_tmp] /= g_K_tmp;
  h_K_tmp = B[3 + r3];
  K[rtemp] -= K[d_K_tmp] * h_K_tmp;
  K[e_K_tmp] -= K[d_K_tmp] * B[r3];
  K[e_K_tmp] -= K[rtemp] * B[r2];
  c_K_tmp = 1 + e_K_tmp;
  K[c_K_tmp] = y[1] / B[r1];
  b_K_tmp = 1 + rtemp;
  K[b_K_tmp] = y[5] - K[c_K_tmp] * qNorm;
  i_K_tmp = 1 + d_K_tmp;
  K[i_K_tmp] = y[9] - K[c_K_tmp] * a21;
  K[b_K_tmp] /= K_tmp;
  K[i_K_tmp] -= K[b_K_tmp] * f_K_tmp;
  K[i_K_tmp] /= g_K_tmp;
  K[b_K_tmp] -= K[i_K_tmp] * h_K_tmp;
  K[c_K_tmp] -= K[i_K_tmp] * B[r3];
  K[c_K_tmp] -= K[b_K_tmp] * B[r2];
  c_K_tmp = 2 + e_K_tmp;
  K[c_K_tmp] = y[2] / B[r1];
  b_K_tmp = 2 + rtemp;
  K[b_K_tmp] = y[6] - K[c_K_tmp] * qNorm;
  i_K_tmp = 2 + d_K_tmp;
  K[i_K_tmp] = y[10] - K[c_K_tmp] * a21;
  K[b_K_tmp] /= K_tmp;
  K[i_K_tmp] -= K[b_K_tmp] * f_K_tmp;
  K[i_K_tmp] /= g_K_tmp;
  K[b_K_tmp] -= K[i_K_tmp] * h_K_tmp;
  K[c_K_tmp] -= K[i_K_tmp] * B[r3];
  K[c_K_tmp] -= K[b_K_tmp] * B[r2];
  e_K_tmp += 3;
  K[e_K_tmp] = y[3] / B[r1];
  rtemp += 3;
  K[rtemp] = y[7] - K[e_K_tmp] * qNorm;
  d_K_tmp += 3;
  K[d_K_tmp] = y[11] - K[e_K_tmp] * a21;
  K[rtemp] /= K_tmp;
  K[d_K_tmp] -= K[rtemp] * f_K_tmp;
  K[d_K_tmp] /= g_K_tmp;
  K[rtemp] -= K[d_K_tmp] * h_K_tmp;
  K[e_K_tmp] -= K[d_K_tmp] * B[r3];
  K[e_K_tmp] -= K[rtemp] * B[r2];

  /*  Update error covariance matrix */
  memset(&A[0], 0, sizeof(double) << 4);
  A[0] = 1.0;
  A[5] = 1.0;
  A[10] = 1.0;
  A[15] = 1.0;
  for (b_K_tmp = 0; b_K_tmp < 4; b_K_tmp++) {
    for (c_K_tmp = 0; c_K_tmp < 4; c_K_tmp++) {
      rtemp = b_K_tmp + (c_K_tmp << 2);
      b_A[rtemp] = A[rtemp] - ((K[b_K_tmp] * C[3 * c_K_tmp] + K[b_K_tmp + 4] *
        C[1 + 3 * c_K_tmp]) + K[b_K_tmp + 8] * C[2 + 3 * c_K_tmp]);
    }

    for (c_K_tmp = 0; c_K_tmp < 4; c_K_tmp++) {
      d_K_tmp = c_K_tmp << 2;
      A[b_K_tmp + d_K_tmp] = ((b_A[b_K_tmp] * P[d_K_tmp] + b_A[b_K_tmp + 4] * P
        [1 + d_K_tmp]) + b_A[b_K_tmp + 8] * P[2 + d_K_tmp]) + b_A[b_K_tmp + 12] *
        P[3 + d_K_tmp];
    }
  }

  memcpy(&P[0], &A[0], sizeof(double) << 4);

  /*  Update state estimate using accelerometer measurements */
  qNorm = ax - -2.0 * g * (x[1] * x[3] - x[2] * x[0]);
  a21 = ay - (Va * r - 2.0 * g * (x[2] * x[3] + x[1] * x[0]));
  K_tmp = az - (-Va * q - g * (1.0 - 2.0 * (x[1] * x[1] + x[2] * x[2])));
  for (b_K_tmp = 0; b_K_tmp < 4; b_K_tmp++) {
    x[b_K_tmp] += (K[b_K_tmp] * qNorm + K[b_K_tmp + 4] * a21) + K[b_K_tmp + 8] *
      K_tmp;
  }

  /*  Normalise quaternion */
  qNorm = sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] /= qNorm;
  x[1] /= qNorm;
  x[2] /= qNorm;
  x[3] /= qNorm;

  /*  Re-extract states */
  /*  Store state estimates */
  *rollDeg = atan2(2.0 * (x[0] * x[1] + x[2] * x[3]), ((x[0] * x[0] + x[3] * x[3])
    - x[1] * x[1]) - x[2] * x[2]) * 180.0 / 3.1415926535897931;
  *pitchDeg = asin(2.0 * (x[0] * x[2] - x[1] * x[3])) * 180.0 /
    3.1415926535897931;
}

void simpleEKFQuat_initialize(void)
{
  simpleEKFQuat_init();
}

void simpleEKFQuat_terminate(void)
{
  /* (no terminate code required) */
}

/* End of code generation (simpleEKFQuat.c) */
