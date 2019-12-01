/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKFQuatAtt2.c
 *
 * Code generation for function 'updateEKFQuatAtt2'
 *
 */

/* Include files */
#include <string.h>
#include <math.h>
#include "updateEKFQuatAtt2.h"

/* Variable Definitions */
static double p;
static double q;
static double r;
static double ax;
static double ay;
static double az;
static double mx;
static double my;
static double mz;
static double Va;
static double lpfGyr;
static double lpfAcc;
static double lpfMag;
static double lpfVa;
static double NdivT;
static double x[7];
static double P[49];
static double Q[49];
static double R[9];
static double g;

/* Function Declarations */
static void updateEKFQuatAtt2_init(void);

/* Function Definitions */
static void updateEKFQuatAtt2_init(void)
{
  int i;
  static const double dv2[7] = { 1.0E-6, 1.0E-6, 1.0E-6, 1.0E-6, 1.0E-8, 1.0E-8,
    1.0E-8 };

  static const double dv3[7] = { 2.5E-9, 2.5E-9, 2.5E-9, 2.5E-9, 1.0E-12,
    1.0E-12, 1.0E-12 };

  g = 9.81;

  /*  Low-pass filtered measurements */
  p = 0.0;
  q = 0.0;
  r = 0.0;
  ax = 0.0;
  ay = 0.0;
  az = 0.0;
  mx = 0.0;
  my = 0.0;
  mz = 0.0;
  Va = 0.0;

  /*  Low-pass filter coefficients */
  lpfGyr = 0.7;
  lpfAcc = 0.9;
  lpfMag = 0.4;
  lpfVa = 0.7;
  NdivT = 10.0;

  /*  Initialise state estimate vector */
  for (i = 0; i < 7; i++) {
    x[i] = 0.0;
  }

  x[0] = 1.0;

  /*  Initialise covariance matrix */
  memset(&P[0], 0, 49U * sizeof(double));
  for (i = 0; i < 7; i++) {
    P[i + 7 * i] = dv2[i];
  }

  /*  Process noise matrix */
  memset(&Q[0], 0, 49U * sizeof(double));
  for (i = 0; i < 7; i++) {
    Q[i + 7 * i] = dv3[i];
  }

  /*  Measurement noise matrix */
  memset(&R[0], 0, 9U * sizeof(double));
  R[0] = 0.0025000000000000005;
  R[4] = 0.0025000000000000005;
  R[8] = 0.0025000000000000005;
}

void updateEKFQuatAtt2(const double gyr_rps[3], const double acc_mps2[3], const
  double mag_unit[3], double Va_mps, double T, double *roll_deg, double
  *pitch_deg, double *yaw_deg, double *yaw_mag_deg)
{
  double mnorm;
  int K_tmp;
  int rtemp;
  double a;
  double dv0[12];
  double a21;
  double A[49];
  double unnamed_idx_2;
  double A_tmp;
  int r1;
  double b_a[7];
  double b_A[49];
  double dv1[49];
  int b_K_tmp;
  double C[21];
  int c_K_tmp;
  double K[21];
  int r2;
  int r3;
  double B[9];
  double y[21];
  double b_C[21];

  /*  Get measurements and low-pass filter them */
  p = lpfGyr * p + (1.0 - lpfGyr) * gyr_rps[0];
  q = lpfGyr * q + (1.0 - lpfGyr) * gyr_rps[1];
  r = lpfGyr * r + (1.0 - lpfGyr) * gyr_rps[2];
  ax = lpfAcc * ax + (1.0 - lpfAcc) * acc_mps2[0];
  ay = lpfAcc * ay + (1.0 - lpfAcc) * acc_mps2[1];
  az = lpfAcc * az + (1.0 - lpfAcc) * acc_mps2[2];
  mx = lpfMag * mx + (1.0 - lpfMag) * mag_unit[0];
  my = lpfMag * my + (1.0 - lpfMag) * mag_unit[1];
  mz = lpfMag * mz + (1.0 - lpfMag) * mag_unit[2];
  mnorm = sqrt((mx * mx + my * my) + mz * mz);
  mx /= mnorm;
  my /= mnorm;
  mz /= mnorm;
  Va = lpfVa * Va + (1.0 - lpfVa) * Va_mps;
  K_tmp = (int)NdivT;
  for (rtemp = 0; rtemp < K_tmp; rtemp++) {
    /*  Extract states */
    /*  State transition function, xdot = f(x, u) */
    /*  Update state estimate */
    a = T / NdivT;
    mnorm = 0.5 * -x[1];
    dv0[0] = mnorm;
    a21 = 0.5 * -x[2];
    dv0[4] = a21;
    unnamed_idx_2 = 0.5 * -x[3];
    dv0[8] = unnamed_idx_2;
    dv0[1] = 0.5 * x[0];
    dv0[5] = unnamed_idx_2;
    dv0[9] = 0.5 * x[2];
    dv0[2] = 0.5 * x[3];
    dv0[6] = 0.5 * x[0];
    dv0[10] = mnorm;
    dv0[3] = a21;
    dv0[7] = 0.5 * x[1];
    dv0[11] = 0.5 * x[0];
    mnorm = p - x[4];
    a21 = q - x[5];
    unnamed_idx_2 = r - x[6];
    for (r1 = 0; r1 < 4; r1++) {
      b_a[r1] = a * ((dv0[r1] * mnorm + dv0[r1 + 4] * a21) + dv0[r1 + 8] *
                     unnamed_idx_2);
    }

    b_a[4] = 0.0;
    b_a[5] = 0.0;
    b_a[6] = 0.0;
    for (r1 = 0; r1 < 7; r1++) {
      x[r1] += b_a[r1];
    }
  }

  /*  Normalise quaternion */
  mnorm = sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] /= mnorm;
  x[1] /= mnorm;
  x[2] /= mnorm;
  x[3] /= mnorm;

  /*  Re-extract states */
  /*  Compute Jacobian of f, A(x, u) */
  A[0] = 0.0;
  mnorm = p - x[4];
  A[7] = -0.5 * mnorm;
  a21 = q - x[5];
  unnamed_idx_2 = -0.5 * a21;
  A[14] = unnamed_idx_2;
  a = r - x[6];
  A_tmp = -0.5 * a;
  A[21] = A_tmp;
  A[28] = 0.5 * x[1];
  A[35] = 0.5 * x[2];
  A[42] = 0.5 * x[3];
  mnorm *= 0.5;
  A[1] = mnorm;
  A[8] = 0.0;
  a *= 0.5;
  A[15] = a;
  A[22] = unnamed_idx_2;
  A[29] = -0.5 * x[0];
  A[36] = 0.5 * x[3];
  A[43] = -0.5 * x[2];
  unnamed_idx_2 = 0.5 * a21;
  A[2] = unnamed_idx_2;
  A[9] = A_tmp;
  A[16] = 0.0;
  A[23] = mnorm;
  A[30] = -0.5 * x[3];
  A[37] = -0.5 * x[0];
  A[44] = 0.5 * x[1];
  A[3] = a;
  A[10] = unnamed_idx_2;
  A[17] = -0.5 * (p - x[4]);
  A[24] = 0.0;
  A[31] = 0.5 * x[2];
  A[38] = -0.5 * x[1];
  A[45] = -0.5 * x[0];
  for (K_tmp = 0; K_tmp < 7; K_tmp++) {
    A[4 + 7 * K_tmp] = 0.0;
    A[5 + 7 * K_tmp] = 0.0;
    A[6 + 7 * K_tmp] = 0.0;
  }

  /*  Update error covariance matrix */
  for (K_tmp = 0; K_tmp < 7; K_tmp++) {
    for (r1 = 0; r1 < 7; r1++) {
      rtemp = K_tmp + 7 * r1;
      dv1[rtemp] = 0.0;
      mnorm = 0.0;
      a21 = 0.0;
      for (b_K_tmp = 0; b_K_tmp < 7; b_K_tmp++) {
        c_K_tmp = K_tmp + 7 * b_K_tmp;
        mnorm += A[c_K_tmp] * P[b_K_tmp + 7 * r1];
        a21 += P[c_K_tmp] * A[r1 + 7 * b_K_tmp];
      }

      dv1[rtemp] = a21;
      b_A[rtemp] = mnorm;
    }
  }

  for (K_tmp = 0; K_tmp < 49; K_tmp++) {
    P[K_tmp] += T * ((b_A[K_tmp] + dv1[K_tmp]) + Q[K_tmp]);
  }

  /*  Output function z(x, u), i.e. compute accelerometer output estimates */
  /*  Note: assuming here that u = Va, v = 0, w = 0 */
  /*  Would be good to set u = Va * cos(theta), v = 0, w = Va * sin(theta) */
  /*  But need expressions for cos(theta) as quaternions... */
  /*  Jacobian of z, C(x, u)      */
  C[0] = 2.0 * g * x[2];
  mnorm = -2.0 * g * x[3];
  C[3] = mnorm;
  C[6] = 2.0 * g * x[0];
  a21 = -2.0 * g * x[1];
  C[9] = a21;
  C[12] = 0.0;
  C[15] = 0.0;
  C[18] = 0.0;
  C[1] = a21;
  C[4] = -2.0 * g * x[0];
  C[7] = mnorm;
  C[10] = -2.0 * g * x[2];
  C[13] = 0.0;
  C[16] = 0.0;
  C[19] = -Va;
  C[2] = 0.0;
  C[5] = 4.0 * g * x[1];
  C[8] = 4.0 * g * x[2];
  C[11] = 0.0;
  C[14] = 0.0;
  C[17] = Va;
  C[20] = 0.0;

  /*  Kalman gain */
  for (K_tmp = 0; K_tmp < 3; K_tmp++) {
    for (r1 = 0; r1 < 7; r1++) {
      K[r1 + 7 * K_tmp] = C[K_tmp + 3 * r1];
    }
  }

  for (K_tmp = 0; K_tmp < 7; K_tmp++) {
    for (r1 = 0; r1 < 3; r1++) {
      mnorm = 0.0;
      for (rtemp = 0; rtemp < 7; rtemp++) {
        mnorm += P[K_tmp + 7 * rtemp] * K[rtemp + 7 * r1];
      }

      y[K_tmp + 7 * r1] = mnorm;
    }
  }

  for (K_tmp = 0; K_tmp < 3; K_tmp++) {
    for (r1 = 0; r1 < 7; r1++) {
      mnorm = 0.0;
      for (rtemp = 0; rtemp < 7; rtemp++) {
        mnorm += C[K_tmp + 3 * rtemp] * P[rtemp + 7 * r1];
      }

      b_C[K_tmp + 3 * r1] = mnorm;
    }

    for (r1 = 0; r1 < 3; r1++) {
      mnorm = 0.0;
      for (rtemp = 0; rtemp < 7; rtemp++) {
        mnorm += b_C[K_tmp + 3 * rtemp] * K[rtemp + 7 * r1];
      }

      rtemp = K_tmp + 3 * r1;
      B[rtemp] = mnorm + R[rtemp];
    }
  }

  r1 = 0;
  r2 = 1;
  r3 = 2;
  mnorm = fabs(B[0]);
  a21 = fabs(B[1]);
  if (a21 > mnorm) {
    mnorm = a21;
    r1 = 1;
    r2 = 0;
  }

  if (fabs(B[2]) > mnorm) {
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
  for (rtemp = 0; rtemp < 7; rtemp++) {
    b_K_tmp = rtemp + 7 * r1;
    K[b_K_tmp] = y[rtemp] / B[r1];
    c_K_tmp = rtemp + 7 * r2;
    K[c_K_tmp] = y[7 + rtemp] - K[b_K_tmp] * B[3 + r1];
    K_tmp = rtemp + 7 * r3;
    K[K_tmp] = y[14 + rtemp] - K[b_K_tmp] * B[6 + r1];
    K[c_K_tmp] /= B[3 + r2];
    K[K_tmp] -= K[c_K_tmp] * B[6 + r2];
    K[K_tmp] /= B[6 + r3];
    K[c_K_tmp] -= K[K_tmp] * B[3 + r3];
    K[b_K_tmp] -= K[K_tmp] * B[r3];
    K[b_K_tmp] -= K[c_K_tmp] * B[r2];
  }

  /*  Update error covariance matrix */
  memset(&A[0], 0, 49U * sizeof(double));
  for (rtemp = 0; rtemp < 7; rtemp++) {
    A[rtemp + 7 * rtemp] = 1.0;
  }

  for (K_tmp = 0; K_tmp < 7; K_tmp++) {
    for (r1 = 0; r1 < 7; r1++) {
      rtemp = K_tmp + 7 * r1;
      b_A[rtemp] = A[rtemp] - ((K[K_tmp] * C[3 * r1] + K[K_tmp + 7] * C[1 + 3 *
        r1]) + K[K_tmp + 14] * C[2 + 3 * r1]);
    }

    for (r1 = 0; r1 < 7; r1++) {
      mnorm = 0.0;
      for (rtemp = 0; rtemp < 7; rtemp++) {
        mnorm += b_A[K_tmp + 7 * rtemp] * P[rtemp + 7 * r1];
      }

      A[K_tmp + 7 * r1] = mnorm;
    }
  }

  memcpy(&P[0], &A[0], 49U * sizeof(double));

  /*  Update state estimate using measurements (accelerometer and */
  /*  magnetometer) */
  mnorm = ax - -2.0 * g * (x[1] * x[3] - x[2] * x[0]);
  a21 = ay - (Va * (r - x[6]) - 2.0 * g * (x[2] * x[3] + x[1] * x[0]));
  unnamed_idx_2 = az - (-Va * (q - x[5]) - g * (1.0 - 2.0 * (x[1] * x[1] + x[2] *
    x[2])));
  for (K_tmp = 0; K_tmp < 7; K_tmp++) {
    x[K_tmp] += (K[K_tmp] * mnorm + K[K_tmp + 7] * a21) + K[K_tmp + 14] *
      unnamed_idx_2;
  }

  /*  Normalise quaternion */
  mnorm = sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] /= mnorm;
  x[1] /= mnorm;
  x[2] /= mnorm;
  x[3] /= mnorm;

  /*  Re-extract states */
  /*  Store state estimates */
  mnorm = x[0] * x[0];
  a21 = x[1] * x[1];
  unnamed_idx_2 = x[2] * x[2];
  a = x[3] * x[3];
  *roll_deg = atan2(2.0 * (x[0] * x[1] + x[2] * x[3]), ((mnorm + a) - a21) -
                    unnamed_idx_2) * 180.0 / 3.1415926535897931;
  *pitch_deg = asin(2.0 * (x[0] * x[2] - x[1] * x[3])) * 180.0 /
    3.1415926535897931;
  *yaw_deg = atan2(2.0 * (x[0] * x[3] + x[1] * x[2]), ((mnorm + a21) -
    unnamed_idx_2) - a) * 180.0 / 3.1415926535897931;

  /*  Compute tild-compensated estimate of yaw angle using filtered */
  /*  magnetometer measurements */
  mnorm = *roll_deg * 3.1415926535897931 / 180.0;
  a21 = sin(mnorm);
  mnorm = cos(mnorm);
  unnamed_idx_2 = *pitch_deg * 3.1415926535897931 / 180.0;
  *yaw_mag_deg = atan2(-my * mnorm + mz * a21, mx * cos(unnamed_idx_2) + (my *
    a21 + mz * mnorm) * sin(unnamed_idx_2)) * 180.0 / 3.1415926535897931;
}

void updateEKFQuatAtt2_initialize(void)
{
  updateEKFQuatAtt2_init();
}

void updateEKFQuatAtt2_terminate(void)
{
  /* (no terminate code required) */
}

/* End of code generation (updateEKFQuatAtt2.c) */
