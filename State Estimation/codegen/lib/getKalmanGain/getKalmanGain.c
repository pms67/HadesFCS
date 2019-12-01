/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * getKalmanGain.c
 *
 * Code generation for function 'getKalmanGain'
 *
 */

/* Include files */
#include <math.h>
#include "getKalmanGain.h"

/* Function Definitions */
void getKalmanGain(const double P[16], const double C[12], double r, double K[12])
{
  int K_tmp;
  int rtemp;
  double b_tmp[12];
  int r1;
  int b_K_tmp;
  int r2;
  double y[12];
  int r3;
  double maxval;
  double B[9];
  double b_C[12];
  double a21;
  int B_tmp;
  static const signed char a[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

  double c_K_tmp;
  double d_K_tmp;
  double e_K_tmp;
  double f_K_tmp;
  int g_K_tmp;
  int h_K_tmp;
  for (K_tmp = 0; K_tmp < 3; K_tmp++) {
    rtemp = K_tmp << 2;
    b_tmp[rtemp] = C[K_tmp];
    b_tmp[1 + rtemp] = C[K_tmp + 3];
    b_tmp[2 + rtemp] = C[K_tmp + 6];
    b_tmp[3 + rtemp] = C[K_tmp + 9];
  }

  for (K_tmp = 0; K_tmp < 4; K_tmp++) {
    for (rtemp = 0; rtemp < 3; rtemp++) {
      b_K_tmp = rtemp << 2;
      y[K_tmp + b_K_tmp] = ((P[K_tmp] * b_tmp[b_K_tmp] + P[K_tmp + 4] * b_tmp[1
        + b_K_tmp]) + P[K_tmp + 8] * b_tmp[2 + b_K_tmp]) + P[K_tmp + 12] *
        b_tmp[3 + b_K_tmp];
    }
  }

  for (K_tmp = 0; K_tmp < 3; K_tmp++) {
    for (rtemp = 0; rtemp < 4; rtemp++) {
      b_K_tmp = rtemp << 2;
      b_C[K_tmp + 3 * rtemp] = ((C[K_tmp] * P[b_K_tmp] + C[K_tmp + 3] * P[1 +
        b_K_tmp]) + C[K_tmp + 6] * P[2 + b_K_tmp]) + C[K_tmp + 9] * P[3 +
        b_K_tmp];
    }

    for (rtemp = 0; rtemp < 3; rtemp++) {
      b_K_tmp = rtemp << 2;
      B_tmp = K_tmp + 3 * rtemp;
      B[B_tmp] = (((b_C[K_tmp] * b_tmp[b_K_tmp] + b_C[K_tmp + 3] * b_tmp[1 +
                    b_K_tmp]) + b_C[K_tmp + 6] * b_tmp[2 + b_K_tmp]) + b_C[K_tmp
                  + 9] * b_tmp[3 + b_K_tmp]) + (double)a[B_tmp] * r;
    }
  }

  r1 = 0;
  r2 = 1;
  r3 = 2;
  maxval = fabs(B[0]);
  a21 = fabs(B[1]);
  if (a21 > maxval) {
    maxval = a21;
    r1 = 1;
    r2 = 0;
  }

  if (fabs(B[2]) > maxval) {
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
  b_K_tmp = r1 << 2;
  K[b_K_tmp] = y[0] / B[r1];
  B_tmp = r2 << 2;
  maxval = B[3 + r1];
  K[B_tmp] = y[4] - K[b_K_tmp] * maxval;
  rtemp = r3 << 2;
  a21 = B[6 + r1];
  K[rtemp] = y[8] - K[b_K_tmp] * a21;
  c_K_tmp = B[3 + r2];
  K[B_tmp] /= c_K_tmp;
  d_K_tmp = B[6 + r2];
  K[rtemp] -= K[B_tmp] * d_K_tmp;
  e_K_tmp = B[6 + r3];
  K[rtemp] /= e_K_tmp;
  f_K_tmp = B[3 + r3];
  K[B_tmp] -= K[rtemp] * f_K_tmp;
  K[b_K_tmp] -= K[rtemp] * B[r3];
  K[b_K_tmp] -= K[B_tmp] * B[r2];
  K_tmp = 1 + b_K_tmp;
  K[K_tmp] = y[1] / B[r1];
  g_K_tmp = 1 + B_tmp;
  K[g_K_tmp] = y[5] - K[K_tmp] * maxval;
  h_K_tmp = 1 + rtemp;
  K[h_K_tmp] = y[9] - K[K_tmp] * a21;
  K[g_K_tmp] /= c_K_tmp;
  K[h_K_tmp] -= K[g_K_tmp] * d_K_tmp;
  K[h_K_tmp] /= e_K_tmp;
  K[g_K_tmp] -= K[h_K_tmp] * f_K_tmp;
  K[K_tmp] -= K[h_K_tmp] * B[r3];
  K[K_tmp] -= K[g_K_tmp] * B[r2];
  K_tmp = 2 + b_K_tmp;
  K[K_tmp] = y[2] / B[r1];
  g_K_tmp = 2 + B_tmp;
  K[g_K_tmp] = y[6] - K[K_tmp] * maxval;
  h_K_tmp = 2 + rtemp;
  K[h_K_tmp] = y[10] - K[K_tmp] * a21;
  K[g_K_tmp] /= c_K_tmp;
  K[h_K_tmp] -= K[g_K_tmp] * d_K_tmp;
  K[h_K_tmp] /= e_K_tmp;
  K[g_K_tmp] -= K[h_K_tmp] * f_K_tmp;
  K[K_tmp] -= K[h_K_tmp] * B[r3];
  K[K_tmp] -= K[g_K_tmp] * B[r2];
  b_K_tmp += 3;
  K[b_K_tmp] = y[3] / B[r1];
  B_tmp += 3;
  K[B_tmp] = y[7] - K[b_K_tmp] * maxval;
  rtemp += 3;
  K[rtemp] = y[11] - K[b_K_tmp] * a21;
  K[B_tmp] /= c_K_tmp;
  K[rtemp] -= K[B_tmp] * d_K_tmp;
  K[rtemp] /= e_K_tmp;
  K[B_tmp] -= K[rtemp] * f_K_tmp;
  K[b_K_tmp] -= K[rtemp] * B[r3];
  K[b_K_tmp] -= K[B_tmp] * B[r2];
}

void getKalmanGain_initialize(void)
{
}

void getKalmanGain_terminate(void)
{
  /* (no terminate code required) */
}

/* End of code generation (getKalmanGain.c) */
