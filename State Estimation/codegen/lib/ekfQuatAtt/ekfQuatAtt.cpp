/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * ekfQuatAtt.cpp
 *
 * Code generation for function 'ekfQuatAtt'
 *
 */

/* Include files */
#include <string.h>
#include <cmath>
#include <math.h>
#include "ekfQuatAtt.h"

/* Variable Definitions */
static double lpfGyrCoeff;
static double lpfAccCoeff;
static double lpfVaCoeff;
static double P[100];
static double Q[100];
static double R[9];
static double xhat[10];
static double g;
static double DT_STEPS;
static double p;
static double q;
static double r;
static double ax;
static double ay;
static double az;
static double V;

/* Function Declarations */
static void ekfQuatAtt_init();

/* Function Definitions */
static void ekfQuatAtt_init()
{
  int i;
  double v[10];
  lpfGyrCoeff = 0.8;
  lpfAccCoeff = 0.9;
  lpfVaCoeff = 0.75;
  for (i = 0; i < 4; i++) {
    v[i] = 0.001;
  }

  for (i = 0; i < 3; i++) {
    v[i + 4] = 1.0E-6;
    v[i + 7] = 1.0E-6;
  }

  memset(&P[0], 0, 100U * sizeof(double));
  for (i = 0; i < 10; i++) {
    P[i + 10 * i] = v[i];
  }

  for (i = 0; i < 4; i++) {
    v[i] = 0.1;
  }

  for (i = 0; i < 3; i++) {
    v[i + 4] = 1.0E-6;
    v[i + 7] = 1.0E-6;
  }

  memset(&Q[0], 0, 100U * sizeof(double));
  for (i = 0; i < 10; i++) {
    Q[i + 10 * i] = v[i];
  }

  memset(&R[0], 0, 9U * sizeof(double));
  for (i = 0; i < 3; i++) {
    R[i + 3 * i] = 1.0;
  }

  memset(&xhat[0], 0, 10U * sizeof(double));
  xhat[0] = 1.0;
  p = 0.0;
  q = 0.0;
  r = 0.0;
  ax = 0.0;
  ay = 0.0;
  az = 0.0;
  V = 0.0;
  DT_STEPS = 10.0;
  g = 9.81;
}

void ekfQuatAtt(const double gyr[3], const double acc[3], const double [3],
                double airspeed, double dt, double *roll, double *pitch, double *
                yaw)
{
  double dtfrac;
  int k;
  double C[30];
  double A[100];
  int rtemp;
  int r1;
  double y[30];
  int r2;
  int r3;
  double K[30];
  double B[9];
  double a21;
  double b_dtfrac[10];
  double b_A[100];
  double c_A[100];
  double dv0[3];
  double dv1[3];
  double dv2[3];

  /*  Sensor readings (+ low-pass filter) */
  p = lpfGyrCoeff * p + (1.0 - lpfGyrCoeff) * gyr[0];
  q = lpfGyrCoeff * q + (1.0 - lpfGyrCoeff) * gyr[1];
  r = lpfGyrCoeff * r + (1.0 - lpfGyrCoeff) * gyr[2];
  ax = lpfAccCoeff * ax + (1.0 - lpfAccCoeff) * acc[0];
  ay = lpfAccCoeff * ay + (1.0 - lpfAccCoeff) * acc[1];
  az = lpfAccCoeff * az + (1.0 - lpfAccCoeff) * acc[2];
  V = lpfVaCoeff * V + (1.0 - lpfVaCoeff) * airspeed;
  dtfrac = dt / DT_STEPS;
  for (k = 0; k < (int)DT_STEPS; k++) {
    /*  Extract states from vector */
    /*  State transition function */
    /*  Jacobian of f */
    A[0] = 0.0;
    A[10] = xhat[4] / 2.0 - p / 2.0;
    A[20] = xhat[5] / 2.0 - q / 2.0;
    A[30] = xhat[6] / 2.0 - r / 2.0;
    A[40] = xhat[1] / 2.0;
    A[50] = xhat[2] / 2.0;
    A[60] = xhat[3] / 2.0;
    A[70] = 0.0;
    A[80] = 0.0;
    A[90] = 0.0;
    A[1] = p / 2.0 - xhat[4] / 2.0;
    A[11] = 0.0;
    A[21] = r / 2.0 - xhat[6] / 2.0;
    A[31] = xhat[5] / 2.0 - q / 2.0;
    A[41] = -xhat[0] / 2.0;
    A[51] = xhat[3] / 2.0;
    A[61] = -xhat[2] / 2.0;
    A[71] = 0.0;
    A[81] = 0.0;
    A[91] = 0.0;
    A[2] = q / 2.0 - xhat[5] / 2.0;
    A[12] = xhat[6] / 2.0 - r / 2.0;
    A[22] = 0.0;
    A[32] = p / 2.0 - xhat[4] / 2.0;
    A[42] = -xhat[3] / 2.0;
    A[52] = -xhat[0] / 2.0;
    A[62] = xhat[1] / 2.0;
    A[72] = 0.0;
    A[82] = 0.0;
    A[92] = 0.0;
    A[3] = r / 2.0 - xhat[6] / 2.0;
    A[13] = q / 2.0 - xhat[5] / 2.0;
    A[23] = xhat[4] / 2.0 - p / 2.0;
    A[33] = 0.0;
    A[43] = xhat[2] / 2.0;
    A[53] = -xhat[1] / 2.0;
    A[63] = -xhat[0] / 2.0;
    A[73] = 0.0;
    A[83] = 0.0;
    A[93] = 0.0;
    b_dtfrac[0] = dtfrac * ((xhat[1] * (xhat[4] / 2.0 - p / 2.0) + xhat[2] *
      (xhat[5] / 2.0 - q / 2.0)) + xhat[3] * (xhat[6] / 2.0 - r / 2.0));
    b_dtfrac[1] = dtfrac * ((xhat[3] * (xhat[5] / 2.0 - q / 2.0) - xhat[0] *
      (xhat[4] / 2.0 - p / 2.0)) - xhat[2] * (xhat[6] / 2.0 - r / 2.0));
    b_dtfrac[2] = dtfrac * ((xhat[1] * (xhat[6] / 2.0 - r / 2.0) - xhat[0] *
      (xhat[5] / 2.0 - q / 2.0)) - xhat[3] * (xhat[4] / 2.0 - p / 2.0));
    b_dtfrac[3] = dtfrac * ((xhat[2] * (xhat[4] / 2.0 - p / 2.0) - xhat[1] *
      (xhat[5] / 2.0 - q / 2.0)) - xhat[0] * (xhat[6] / 2.0 - r / 2.0));
    b_dtfrac[4] = 0.0;
    b_dtfrac[5] = 0.0;
    b_dtfrac[6] = 0.0;
    b_dtfrac[7] = 0.0;
    b_dtfrac[8] = 0.0;
    b_dtfrac[9] = 0.0;
    for (rtemp = 0; rtemp < 10; rtemp++) {
      A[4 + 10 * rtemp] = 0.0;
      A[5 + 10 * rtemp] = 0.0;
      A[6 + 10 * rtemp] = 0.0;
      A[7 + 10 * rtemp] = 0.0;
      A[8 + 10 * rtemp] = 0.0;
      A[9 + 10 * rtemp] = 0.0;
      xhat[rtemp] += b_dtfrac[rtemp];
    }

    for (rtemp = 0; rtemp < 10; rtemp++) {
      for (r1 = 0; r1 < 10; r1++) {
        b_A[rtemp + 10 * r1] = 0.0;
        c_A[rtemp + 10 * r1] = 0.0;
        for (r2 = 0; r2 < 10; r2++) {
          b_A[rtemp + 10 * r1] += A[rtemp + 10 * r2] * P[r2 + 10 * r1];
          c_A[rtemp + 10 * r1] += P[rtemp + 10 * r2] * A[r1 + 10 * r2];
        }
      }
    }

    for (rtemp = 0; rtemp < 10; rtemp++) {
      for (r1 = 0; r1 < 10; r1++) {
        P[r1 + 10 * rtemp] += dtfrac * ((b_A[r1 + 10 * rtemp] + c_A[r1 + 10 *
          rtemp]) + Q[r1 + 10 * rtemp]);
      }
    }
  }

  /*  Again, extract updated states from vector */
  /*  Output function */
  /*  Jacobian of h */
  C[0] = 2.0 * xhat[2] * g;
  C[3] = -2.0 * xhat[3] * g;
  C[6] = 2.0 * xhat[0] * g;
  C[9] = -2.0 * xhat[1] * g;
  C[12] = 0.0;
  C[15] = 0.0;
  C[18] = 0.0;
  C[21] = -1.0;
  C[24] = 0.0;
  C[27] = 0.0;
  C[1] = -2.0 * xhat[1] * g;
  C[4] = -2.0 * xhat[0] * g;
  C[7] = -2.0 * xhat[3] * g;
  C[10] = -2.0 * xhat[2] * g;
  C[13] = 0.0;
  C[16] = 0.0;
  C[19] = -V;
  C[22] = 0.0;
  C[25] = -1.0;
  C[28] = 0.0;
  C[2] = -2.0 * xhat[0] * g;
  C[5] = 2.0 * xhat[1] * g;
  C[8] = 2.0 * xhat[2] * g;
  C[11] = -2.0 * xhat[3] * g;
  C[14] = 0.0;
  C[17] = V;
  C[20] = 0.0;
  C[23] = 0.0;
  C[26] = 0.0;
  C[29] = -1.0;
  for (rtemp = 0; rtemp < 10; rtemp++) {
    for (r1 = 0; r1 < 3; r1++) {
      y[rtemp + 10 * r1] = 0.0;
      for (r2 = 0; r2 < 10; r2++) {
        y[rtemp + 10 * r1] += P[rtemp + 10 * r2] * C[r1 + 3 * r2];
      }
    }
  }

  for (rtemp = 0; rtemp < 3; rtemp++) {
    for (r1 = 0; r1 < 10; r1++) {
      K[rtemp + 3 * r1] = 0.0;
      for (r2 = 0; r2 < 10; r2++) {
        K[rtemp + 3 * r1] += C[rtemp + 3 * r2] * P[r2 + 10 * r1];
      }
    }

    for (r1 = 0; r1 < 3; r1++) {
      dtfrac = 0.0;
      for (r2 = 0; r2 < 10; r2++) {
        dtfrac += K[rtemp + 3 * r2] * C[r1 + 3 * r2];
      }

      B[rtemp + 3 * r1] = dtfrac + R[rtemp + 3 * r1];
    }
  }

  r1 = 0;
  r2 = 1;
  r3 = 2;
  dtfrac = std::abs(B[0]);
  a21 = std::abs(B[1]);
  if (a21 > dtfrac) {
    dtfrac = a21;
    r1 = 1;
    r2 = 0;
  }

  if (std::abs(B[2]) > dtfrac) {
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
  if (std::abs(B[3 + r3]) > std::abs(B[3 + r2])) {
    rtemp = r2;
    r2 = r3;
    r3 = rtemp;
  }

  B[3 + r3] /= B[3 + r2];
  B[6 + r3] -= B[3 + r3] * B[6 + r2];
  for (k = 0; k < 10; k++) {
    K[k + 10 * r1] = y[k] / B[r1];
    K[k + 10 * r2] = y[10 + k] - K[k + 10 * r1] * B[3 + r1];
    K[k + 10 * r3] = y[20 + k] - K[k + 10 * r1] * B[6 + r1];
    K[k + 10 * r2] /= B[3 + r2];
    K[k + 10 * r3] -= K[k + 10 * r2] * B[6 + r2];
    K[k + 10 * r3] /= B[6 + r3];
    K[k + 10 * r2] -= K[k + 10 * r3] * B[3 + r3];
    K[k + 10 * r1] -= K[k + 10 * r3] * B[r3];
    K[k + 10 * r1] -= K[k + 10 * r2] * B[r2];
  }

  memset(&A[0], 0, 100U * sizeof(double));
  for (k = 0; k < 10; k++) {
    A[k + 10 * k] = 1.0;
  }

  for (rtemp = 0; rtemp < 10; rtemp++) {
    for (r1 = 0; r1 < 10; r1++) {
      dtfrac = 0.0;
      for (r2 = 0; r2 < 3; r2++) {
        dtfrac += K[rtemp + 10 * r2] * C[r2 + 3 * r1];
      }

      b_A[rtemp + 10 * r1] = A[rtemp + 10 * r1] - dtfrac;
    }

    for (r1 = 0; r1 < 10; r1++) {
      c_A[rtemp + 10 * r1] = 0.0;
      for (r2 = 0; r2 < 10; r2++) {
        c_A[rtemp + 10 * r1] += b_A[rtemp + 10 * r2] * P[r2 + 10 * r1];
      }
    }
  }

  for (rtemp = 0; rtemp < 10; rtemp++) {
    memcpy(&P[rtemp * 10], &c_A[rtemp * 10], 10U * sizeof(double));
  }

  dv0[0] = ax - xhat[7];
  dv0[1] = ay - xhat[8];
  dv0[2] = az - xhat[9];
  dv1[0] = g * (2.0 * xhat[0] * xhat[2] - 2.0 * xhat[1] * xhat[3]) - xhat[7];
  dv1[1] = (-V * (xhat[6] - r) - g * (2.0 * xhat[0] * xhat[1] + 2.0 * xhat[2] *
             xhat[3])) - xhat[8];
  dv1[2] = (V * (xhat[5] - q) - g * (((xhat[0] * xhat[0] - xhat[1] * xhat[1]) -
              xhat[2] * xhat[2]) + xhat[3] * xhat[3])) - xhat[9];
  for (rtemp = 0; rtemp < 3; rtemp++) {
    dv2[rtemp] = dv0[rtemp] - dv1[rtemp];
  }

  for (rtemp = 0; rtemp < 10; rtemp++) {
    dtfrac = 0.0;
    for (r1 = 0; r1 < 3; r1++) {
      dtfrac += K[rtemp + 10 * r1] * dv2[r1];
    }

    xhat[rtemp] += dtfrac;
  }

  /*  Again, extract updated states from vector */
  *roll = atan2(2.0 * (xhat[0] * xhat[1] + xhat[2] * xhat[3]), ((xhat[0] * xhat
    [0] + xhat[3] * xhat[3]) - xhat[1] * xhat[1]) - xhat[2] * xhat[2]) * 180.0 /
    3.1415926535897931;
  *pitch = std::asin(2.0 * (xhat[0] * xhat[2] - xhat[1] * xhat[3])) * 180.0 /
    3.1415926535897931;
  *yaw = atan2(2.0 * (xhat[0] * xhat[3] + xhat[1] * xhat[2]), ((xhat[0] * xhat[0]
    + xhat[1] * xhat[1]) - xhat[2] * xhat[2]) - xhat[3] * xhat[3]) * 180.0 /
    3.1415926535897931;
}

void ekfQuatAtt_initialize()
{
  ekfQuatAtt_init();
}

void ekfQuatAtt_terminate()
{
  /* (no terminate code required) */
}

/* End of code generation (ekfQuatAtt.cpp) */
