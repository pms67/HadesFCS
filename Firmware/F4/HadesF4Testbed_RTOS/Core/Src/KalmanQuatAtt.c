/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKFQuatAtt.c
 *
 * Code generation for function 'updateEKFQuatAtt'
 *
 */

/* Include files */
#include <string.h>
#include <math.h>
#include "KalmanQuatAtt.h"

/* Variable Definitions */
static float p;
static float q;
static float r;
static float ax;
static float ay;
static float az;
static float mx;
static float my;
static float mz;
static float Va;
static float lpfGyr;
static float lpfAcc;
static float lpfMag;
static float lpfVa;
static float x[7];
static float P[49];
static float Q[49];
static float R[36];
static float g;


/* Function Definitions */
void updateEKFQuatAtt_initialize(void)
{
  int i;
  static const float dv4[7] = { 1.0E-6, 1.0E-6, 1.0E-6, 1.0E-6, 1.0E-8, 1.0E-8,
    1.0E-8 };

  static const float dv5[7] = { 2.5E-9, 2.5E-9, 2.5E-9, 2.5E-9, 1.0E-12,
    1.0E-12, 1.0E-12 };

  static const float dv6[6] = { 0.0025000000000000005, 0.0025000000000000005,
    0.0025000000000000005, 0.0004, 0.0004, 0.0004 };

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

  /*  Initialise state estimate vector */
  for (i = 0; i < 7; i++) {
    x[i] = 0.0;
  }

  x[0] = 1.0;

  /*  Initialise covariance matrix */
  memset(&P[0], 0, 49U * sizeof(float));
  for (i = 0; i < 7; i++) {
    P[i + 7 * i] = dv4[i];
  }

  /*  Process noise matrix */
  memset(&Q[0], 0, 49U * sizeof(float));
  for (i = 0; i < 7; i++) {
    Q[i + 7 * i] = dv5[i];
  }

  /*  Measurement noise matrix */
  memset(&R[0], 0, 36U * sizeof(float));
  for (i = 0; i < 6; i++) {
    R[i + 6 * i] = dv6[i];
  }
}

void updateEKFQuatAtt(const float gyr_rps[3], const float acc_mps2[3], const
                      float mag_unit[3], float Va_mps, float magDecRad,
                      float T, float NdivT, float *roll_deg, float
                      *pitch_deg, float *yaw_deg)
{
  float mnorm;
  int i0;
  int iy;
  float a;
  float dv0[12];
  float unnamed_idx_2;
  float A[49];
  float s;
  float A_tmp;
  int k;
  float b_a[7];
  float b_A[49];
  float dv1[49];
  float smag;
  float cmag;
  int kBcol;
  int jA;
  float C[42];
  float C_tmp;
  float b_tmp[42];
  int j;
  int jj;
  float K[42];
  int jp1j;
  signed char ipiv[6];
  int n;
  float b_C[42];
  int ix;
  float B[36];
  float dv2[6];
  float dv3[6];

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
  i0 = (int)NdivT;
  for (iy = 0; iy < i0; iy++) {
    /*  Extract states */
    /*  State transition function, xdot = f(x, u) */
    /*  Update state estimate */
    a = T / NdivT;
    mnorm = 0.5 * -x[1];
    dv0[0] = mnorm;
    unnamed_idx_2 = 0.5 * -x[2];
    dv0[4] = unnamed_idx_2;
    s = 0.5 * -x[3];
    dv0[8] = s;
    dv0[1] = 0.5 * x[0];
    dv0[5] = s;
    dv0[9] = 0.5 * x[2];
    dv0[2] = 0.5 * x[3];
    dv0[6] = 0.5 * x[0];
    dv0[10] = mnorm;
    dv0[3] = unnamed_idx_2;
    dv0[7] = 0.5 * x[1];
    dv0[11] = 0.5 * x[0];
    mnorm = p - x[4];
    s = q - x[5];
    unnamed_idx_2 = r - x[6];
    for (k = 0; k < 4; k++) {
      b_a[k] = a * ((dv0[k] * mnorm + dv0[k + 4] * s) + dv0[k + 8] *
                    unnamed_idx_2);
    }

    b_a[4] = 0.0;
    b_a[5] = 0.0;
    b_a[6] = 0.0;
    for (k = 0; k < 7; k++) {
      x[k] += b_a[k];
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
  unnamed_idx_2 = q - x[5];
  s = -0.5 * unnamed_idx_2;
  A[14] = s;
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
  A[22] = s;
  A[29] = -0.5 * x[0];
  A[36] = 0.5 * x[3];
  A[43] = -0.5 * x[2];
  s = 0.5 * unnamed_idx_2;
  A[2] = s;
  A[9] = A_tmp;
  A[16] = 0.0;
  A[23] = mnorm;
  A[30] = -0.5 * x[3];
  A[37] = -0.5 * x[0];
  A[44] = 0.5 * x[1];
  A[3] = a;
  A[10] = s;
  A[17] = -0.5 * (p - x[4]);
  A[24] = 0.0;
  A[31] = 0.5 * x[2];
  A[38] = -0.5 * x[1];
  A[45] = -0.5 * x[0];
  for (i0 = 0; i0 < 7; i0++) {
    A[4 + 7 * i0] = 0.0;
    A[5 + 7 * i0] = 0.0;
    A[6 + 7 * i0] = 0.0;
  }

  /*  Update error covariance matrix */
  for (i0 = 0; i0 < 7; i0++) {
    for (k = 0; k < 7; k++) {
      iy = i0 + 7 * k;
      dv1[iy] = 0.0;
      mnorm = 0.0;
      unnamed_idx_2 = 0.0;
      for (kBcol = 0; kBcol < 7; kBcol++) {
        jA = i0 + 7 * kBcol;
        mnorm += A[jA] * P[kBcol + 7 * k];
        unnamed_idx_2 += P[jA] * A[k + 7 * kBcol];
      }

      dv1[iy] = unnamed_idx_2;
      b_A[iy] = mnorm;
    }
  }

  for (i0 = 0; i0 < 49; i0++) {
    P[i0] += T * ((b_A[i0] + dv1[i0]) + Q[i0]);
  }

  /*  Compute magnetic field unit vector estimate in body coordinates from */
  /*  quaternion estimates */
  smag = sin(magDecRad);
  cmag = cos(magDecRad);

  /*  Compute accelerometer output estimates */
  /*  Note: assuming here that u = Va, v = 0, w = 0 */
  /*  Would be good to set u = Va * cos(theta), v = 0, w = Va * sin(theta) */
  /*  But need expressions for cos(theta) as quaternions... */
  /*  Output function z(x, u) = [axhat, ayhat, azhat, mxhat, myhat, mzhat] */
  /*  Jacobian of z, C(x, u)  */
  C[0] = 2.0 * g * x[2];
  mnorm = -2.0 * g * x[3];
  C[6] = mnorm;
  C[12] = 2.0 * g * x[0];
  unnamed_idx_2 = -2.0 * g * x[1];
  C[18] = unnamed_idx_2;
  C[24] = 0.0;
  C[30] = 0.0;
  C[36] = 0.0;
  C[1] = unnamed_idx_2;
  C[7] = -2.0 * g * x[0];
  C[13] = mnorm;
  C[19] = -2.0 * g * x[2];
  C[25] = 0.0;
  C[31] = 0.0;
  C[37] = -Va;
  C[2] = 0.0;
  C[8] = 4.0 * g * x[1];
  C[14] = 4.0 * g * x[2];
  C[20] = 0.0;
  C[26] = 0.0;
  C[32] = Va;
  C[38] = 0.0;
  mnorm = 2.0 * x[3] * smag;
  C[3] = mnorm;
  unnamed_idx_2 = 2.0 * x[2] * smag;
  C[9] = unnamed_idx_2;
  s = 2.0 * x[1] * smag;
  C[15] = s - 4.0 * x[2] * cmag;
  A_tmp = 2.0 * x[0] * smag;
  C[21] = A_tmp - 4.0 * x[3] * cmag;
  C[27] = 0.0;
  C[33] = 0.0;
  C[39] = 0.0;
  C[4] = -2.0 * x[3] * cmag;
  a = 2.0 * x[2] * cmag;
  C[10] = a - 4.0 * x[1] * smag;
  C_tmp = 2.0 * x[1] * cmag;
  C[16] = C_tmp;
  C[22] = -2.0 * x[0] * cmag - 4.0 * x[3] * smag;
  C[28] = 0.0;
  C[34] = 0.0;
  C[40] = 0.0;
  C[5] = a - s;
  C[11] = 2.0 * x[3] * cmag - A_tmp;
  C[17] = 2.0 * x[0] * cmag + mnorm;
  C[23] = C_tmp + unnamed_idx_2;
  C[29] = 0.0;
  C[35] = 0.0;
  C[41] = 0.0;

  /*  Kalman gain */
  for (i0 = 0; i0 < 6; i0++) {
    for (k = 0; k < 7; k++) {
      b_tmp[k + 7 * i0] = C[i0 + 6 * k];
    }
  }

  for (i0 = 0; i0 < 7; i0++) {
    for (k = 0; k < 6; k++) {
      mnorm = 0.0;
      for (iy = 0; iy < 7; iy++) {
        mnorm += P[i0 + 7 * iy] * b_tmp[iy + 7 * k];
      }

      K[i0 + 7 * k] = mnorm;
    }
  }

  for (i0 = 0; i0 < 6; i0++) {
    for (k = 0; k < 7; k++) {
      mnorm = 0.0;
      for (iy = 0; iy < 7; iy++) {
        mnorm += C[i0 + 6 * iy] * P[iy + 7 * k];
      }

      b_C[i0 + 6 * k] = mnorm;
    }

    for (k = 0; k < 6; k++) {
      mnorm = 0.0;
      for (iy = 0; iy < 7; iy++) {
        mnorm += b_C[i0 + 6 * iy] * b_tmp[iy + 7 * k];
      }

      iy = i0 + 6 * k;
      B[iy] = mnorm + R[iy];
    }

    ipiv[i0] = (signed char)(1 + i0);
  }

  for (j = 0; j < 5; j++) {
    jA = j * 7;
    jj = j * 7;
    jp1j = jA + 2;
    n = 6 - j;
    kBcol = 0;
    ix = jA;
    mnorm = fabs(B[jA]);
    for (k = 2; k <= n; k++) {
      ix++;
      s = fabs(B[ix]);
      if (s > mnorm) {
        kBcol = k - 1;
        mnorm = s;
      }
    }

    if (B[jj + kBcol] != 0.0) {
      if (kBcol != 0) {
        iy = j + kBcol;
        ipiv[j] = (signed char)(iy + 1);
        ix = j;
        for (k = 0; k < 6; k++) {
          mnorm = B[ix];
          B[ix] = B[iy];
          B[iy] = mnorm;
          ix += 6;
          iy += 6;
        }
      }

      i0 = (jj - j) + 6;
      for (n = jp1j; n <= i0; n++) {
        B[n - 1] /= B[jj];
      }
    }

    n = 4 - j;
    iy = jA + 6;
    jA = jj;
    for (kBcol = 0; kBcol <= n; kBcol++) {
      mnorm = B[iy];
      if (B[iy] != 0.0) {
        ix = jj + 1;
        i0 = jA + 8;
        k = (jA - j) + 12;
        for (jp1j = i0; jp1j <= k; jp1j++) {
          B[jp1j - 1] += B[ix] * -mnorm;
          ix++;
        }
      }

      iy += 6;
      jA += 6;
    }
  }

  for (j = 0; j < 6; j++) {
    jA = 7 * j - 1;
    iy = 6 * j;
    for (k = 0; k < j; k++) {
      kBcol = 7 * k;
      mnorm = B[k + iy];
      if (mnorm != 0.0) {
        for (n = 0; n < 7; n++) {
          jp1j = (n + jA) + 1;
          K[jp1j] -= mnorm * K[n + kBcol];
        }
      }
    }

    mnorm = 1.0 / B[j + iy];
    for (n = 0; n < 7; n++) {
      jp1j = (n + jA) + 1;
      K[jp1j] *= mnorm;
    }
  }

  for (j = 5; j >= 0; j--) {
    jA = 7 * j - 1;
    iy = 6 * j - 1;
    i0 = j + 2;
    for (k = i0; k < 7; k++) {
      kBcol = 7 * (k - 1);
      mnorm = B[k + iy];
      if (mnorm != 0.0) {
        for (n = 0; n < 7; n++) {
          jp1j = (n + jA) + 1;
          K[jp1j] -= mnorm * K[n + kBcol];
        }
      }
    }
  }

  for (iy = 4; iy >= 0; iy--) {
    if (ipiv[iy] != iy + 1) {
      for (kBcol = 0; kBcol < 7; kBcol++) {
        jA = kBcol + 7 * iy;
        mnorm = K[jA];
        jp1j = kBcol + 7 * (ipiv[iy] - 1);
        K[jA] = K[jp1j];
        K[jp1j] = mnorm;
      }
    }
  }

  /*  Update error covariance matrix */
  memset(&A[0], 0, 49U * sizeof(float));
  for (k = 0; k < 7; k++) {
    A[k + 7 * k] = 1.0;
  }

  for (i0 = 0; i0 < 7; i0++) {
    for (k = 0; k < 7; k++) {
      mnorm = 0.0;
      for (iy = 0; iy < 6; iy++) {
        mnorm += K[i0 + 7 * iy] * C[iy + 6 * k];
      }

      iy = i0 + 7 * k;
      b_A[iy] = A[iy] - mnorm;
    }

    for (k = 0; k < 7; k++) {
      mnorm = 0.0;
      for (iy = 0; iy < 7; iy++) {
        mnorm += b_A[i0 + 7 * iy] * P[iy + 7 * k];
      }

      A[i0 + 7 * k] = mnorm;
    }
  }

  memcpy(&P[0], &A[0], 49U * sizeof(float));

  /*  Update state estimate using measurements (accelerometer and */
  /*  magnetometer) */
  dv2[0] = ax;
  dv2[1] = ay;
  dv2[2] = az;
  dv2[3] = mx;
  dv2[4] = my;
  dv2[5] = mz;
  dv3[0] = -2.0 * g * (x[1] * x[3] - x[2] * x[0]);
  dv3[1] = Va * (r - x[6]) - 2.0 * g * (x[2] * x[3] + x[1] * x[0]);
  dv3[2] = -Va * (q - x[5]) - g * (1.0 - 2.0 * (x[1] * x[1] + x[2] * x[2]));
  mnorm = 2.0 * x[0] * x[3];
  unnamed_idx_2 = 2.0 * x[1] * x[2];
  s = 2.0 * x[3] * x[3];
  dv3[3] = smag * (mnorm + unnamed_idx_2) - cmag * ((2.0 * x[2] * x[2] + s) -
    1.0);
  dv3[4] = -cmag * (mnorm - unnamed_idx_2) - smag * ((2.0 * x[1] * x[1] + s) -
    1.0);
  dv3[5] = cmag * (2.0 * x[0] * x[2] + 2.0 * x[1] * x[3]) - smag * (2.0 * x[0] *
    x[1] - 2.0 * x[2] * x[3]);
  for (i0 = 0; i0 < 6; i0++) {
    dv2[i0] -= dv3[i0];
  }

  for (i0 = 0; i0 < 7; i0++) {
    mnorm = 0.0;
    for (k = 0; k < 6; k++) {
      mnorm += K[i0 + 7 * k] * dv2[k];
    }

    x[i0] += mnorm;
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
  unnamed_idx_2 = x[1] * x[1];
  s = x[2] * x[2];
  a = x[3] * x[3];
  *roll_deg = atan2(2.0 * (x[0] * x[1] + x[2] * x[3]), ((mnorm + a) -
    unnamed_idx_2) - s) * 180.0 / 3.1415926535897931;
  *pitch_deg = asin(2.0 * (x[0] * x[2] - x[1] * x[3])) * 180.0 /
    3.1415926535897931;
  *yaw_deg = atan2(2.0 * (x[0] * x[3] + x[1] * x[2]), ((mnorm + unnamed_idx_2) -
    s) - a) * 180.0 / 3.1415926535897931;
}

/* End of code generation (updateEKFQuatAtt.c) */
