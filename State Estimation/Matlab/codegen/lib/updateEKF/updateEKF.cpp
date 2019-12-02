/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKF.cpp
 *
 * Code generation for function 'updateEKF'
 *
 */

/* Include files */
#include <cmath>
#include <string.h>
#include <math.h>
#include "updateEKF.h"

/* Variable Definitions */
static double lpfGyrCoeff;
static double lpfAccCoeff;
static double lpfMagCoeff;
static double lpfVaCoeff;
static double P[100];
static double Q[100];
static double R[16];
static double xhat[10];
static double p;
static double q;
static double r;
static double ax;
static double ay;
static double az;
static double mx;
static double my;
static double mz;
static double V;
static double g;

/* Function Declarations */
static void mrdivide(double A[40], const double B[16]);
static void updateEKF_init();

/* Function Definitions */
static void mrdivide(double A[40], const double B[16])
{
  double b_A[16];
  int k;
  int j;
  signed char ipiv[4];
  int c;
  int jAcol;
  int jy;
  int ix;
  double smax;
  int iy;
  int i;
  double s;
  memcpy(&b_A[0], &B[0], sizeof(double) << 4);
  for (k = 0; k < 4; k++) {
    ipiv[k] = (signed char)(1 + k);
  }

  for (j = 0; j < 3; j++) {
    c = j * 5;
    jAcol = 0;
    ix = c;
    smax = std::abs(b_A[c]);
    for (k = 2; k <= 4 - j; k++) {
      ix++;
      s = std::abs(b_A[ix]);
      if (s > smax) {
        jAcol = k - 1;
        smax = s;
      }
    }

    if (b_A[c + jAcol] != 0.0) {
      if (jAcol != 0) {
        ipiv[j] = (signed char)((j + jAcol) + 1);
        ix = j;
        iy = j + jAcol;
        for (k = 0; k < 4; k++) {
          smax = b_A[ix];
          b_A[ix] = b_A[iy];
          b_A[iy] = smax;
          ix += 4;
          iy += 4;
        }
      }

      k = (c - j) + 4;
      for (i = c + 1; i < k; i++) {
        b_A[i] /= b_A[c];
      }
    }

    iy = c;
    jy = c + 4;
    for (jAcol = 1; jAcol <= 3 - j; jAcol++) {
      smax = b_A[jy];
      if (b_A[jy] != 0.0) {
        ix = c + 1;
        k = (iy - j) + 8;
        for (i = 5 + iy; i < k; i++) {
          b_A[i] += b_A[ix] * -smax;
          ix++;
        }
      }

      jy += 4;
      iy += 4;
    }
  }

  for (j = 0; j < 4; j++) {
    jy = 10 * j;
    jAcol = j << 2;
    for (k = 1; k <= j; k++) {
      iy = 10 * (k - 1);
      if (b_A[(k + jAcol) - 1] != 0.0) {
        for (i = 0; i < 10; i++) {
          A[i + jy] -= b_A[(k + jAcol) - 1] * A[i + iy];
        }
      }
    }

    smax = 1.0 / b_A[j + jAcol];
    for (i = 0; i < 10; i++) {
      A[i + jy] *= smax;
    }
  }

  for (j = 3; j >= 0; j--) {
    jy = 10 * j;
    jAcol = (j << 2) - 1;
    for (k = j + 2; k < 5; k++) {
      iy = 10 * (k - 1);
      if (b_A[k + jAcol] != 0.0) {
        for (i = 0; i < 10; i++) {
          A[i + jy] -= b_A[k + jAcol] * A[i + iy];
        }
      }
    }
  }

  for (jAcol = 2; jAcol >= 0; jAcol--) {
    if (ipiv[jAcol] != jAcol + 1) {
      iy = ipiv[jAcol] - 1;
      for (jy = 0; jy < 10; jy++) {
        smax = A[jy + 10 * jAcol];
        A[jy + 10 * jAcol] = A[jy + 10 * iy];
        A[jy + 10 * iy] = smax;
      }
    }
  }
}

static void updateEKF_init()
{
  int i;
  double v[10];
  double b_v[4];
  lpfGyrCoeff = 0.8;
  lpfAccCoeff = 0.9;
  lpfMagCoeff = 0.3;
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

  for (i = 0; i < 3; i++) {
    b_v[i] = 0.01;
  }

  b_v[3] = 0.1;
  memset(&R[0], 0, sizeof(double) << 4);
  for (i = 0; i < 4; i++) {
    R[i + (i << 2)] = b_v[i];
  }

  memset(&xhat[0], 0, 10U * sizeof(double));
  xhat[0] = 1.0;
  g = 9.81;
  p = 0.0;
  q = 0.0;
  r = 0.0;
  ax = 0.0;
  ay = 0.0;
  az = 0.0;
  mx = 0.0;
  my = 0.0;
  mz = 0.0;
  V = 0.0;
}

void updateEKF(const double gyr[3], const double acc[3], const double mag[3],
               double airspeed, double dt, double *phi, double *theta, double
               *psi)
{
  double A[100];
  double b_dt[10];
  int k;
  int i0;
  double b_A[100];
  double c_A[100];
  int i1;
  double mphi;
  double mtheta;
  double fact;
  double a;
  double b_a;
  double C[40];
  double K[40];
  double b_C[16];
  double c_C[40];
  double dv0[4];
  double dv1[4];
  double dv2[4];

  /*  Sensor readings (+ low-pass filter) */
  p = lpfGyrCoeff * p + (1.0 - lpfGyrCoeff) * gyr[0];
  q = lpfGyrCoeff * q + (1.0 - lpfGyrCoeff) * gyr[1];
  r = lpfGyrCoeff * r + (1.0 - lpfGyrCoeff) * gyr[2];
  ax = lpfAccCoeff * ax + (1.0 - lpfAccCoeff) * acc[0];
  ay = lpfAccCoeff * ay + (1.0 - lpfAccCoeff) * acc[1];
  az = lpfAccCoeff * az + (1.0 - lpfAccCoeff) * acc[2];
  V = lpfVaCoeff * V + (1.0 - lpfVaCoeff) * airspeed;
  mx = lpfMagCoeff * mx + (1.0 - lpfMagCoeff) * mag[0];
  my = lpfMagCoeff * my + (1.0 - lpfMagCoeff) * mag[1];
  mz = lpfMagCoeff * mz + (1.0 - lpfMagCoeff) * mag[2];

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
  b_dt[0] = dt * ((xhat[1] * (xhat[4] / 2.0 - p / 2.0) + xhat[2] * (xhat[5] /
    2.0 - q / 2.0)) + xhat[3] * (xhat[6] / 2.0 - r / 2.0));
  b_dt[1] = dt * ((xhat[3] * (xhat[5] / 2.0 - q / 2.0) - xhat[0] * (xhat[4] /
    2.0 - p / 2.0)) - xhat[2] * (xhat[6] / 2.0 - r / 2.0));
  b_dt[2] = dt * ((xhat[1] * (xhat[6] / 2.0 - r / 2.0) - xhat[0] * (xhat[5] /
    2.0 - q / 2.0)) - xhat[3] * (xhat[4] / 2.0 - p / 2.0));
  b_dt[3] = dt * ((xhat[2] * (xhat[4] / 2.0 - p / 2.0) - xhat[1] * (xhat[5] /
    2.0 - q / 2.0)) - xhat[0] * (xhat[6] / 2.0 - r / 2.0));
  b_dt[4] = 0.0;
  b_dt[5] = 0.0;
  b_dt[6] = 0.0;
  b_dt[7] = 0.0;
  b_dt[8] = 0.0;
  b_dt[9] = 0.0;
  for (k = 0; k < 10; k++) {
    A[4 + 10 * k] = 0.0;
    A[5 + 10 * k] = 0.0;
    A[6 + 10 * k] = 0.0;
    A[7 + 10 * k] = 0.0;
    A[8 + 10 * k] = 0.0;
    A[9 + 10 * k] = 0.0;
    xhat[k] += b_dt[k];
  }

  for (k = 0; k < 10; k++) {
    for (i0 = 0; i0 < 10; i0++) {
      b_A[k + 10 * i0] = 0.0;
      c_A[k + 10 * i0] = 0.0;
      for (i1 = 0; i1 < 10; i1++) {
        b_A[k + 10 * i0] += A[k + 10 * i1] * P[i1 + 10 * i0];
        c_A[k + 10 * i0] += P[k + 10 * i1] * A[i0 + 10 * i1];
      }
    }
  }

  for (k = 0; k < 10; k++) {
    for (i0 = 0; i0 < 10; i0++) {
      P[i0 + 10 * k] += dt * ((b_A[i0 + 10 * k] + c_A[i0 + 10 * k]) + Q[i0 + 10 *
        k]);
    }
  }

  /*  Again, extract updated states from vector */
  /*  Output function */
  /*  Jacobian of h */
  mphi = (2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3])) - 1.0;
  mtheta = 2.0 * xhat[0] * xhat[3] + 2.0 * xhat[1] * xhat[2];
  fact = 1.0 / (mphi * mphi + mtheta * mtheta);
  mphi = (2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3])) - 1.0;
  mtheta = (2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3])) - 1.0;
  a = (2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3])) - 1.0;
  b_a = (2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3])) - 1.0;
  C[0] = 2.0 * xhat[2] * g;
  C[4] = -2.0 * xhat[3] * g;
  C[8] = 2.0 * xhat[0] * g;
  C[12] = -2.0 * xhat[1] * g;
  C[16] = 0.0;
  C[20] = 0.0;
  C[24] = 0.0;
  C[28] = -1.0;
  C[32] = 0.0;
  C[36] = 0.0;
  C[1] = -2.0 * xhat[1] * g;
  C[5] = -2.0 * xhat[0] * g;
  C[9] = -2.0 * xhat[3] * g;
  C[13] = -2.0 * xhat[2] * g;
  C[17] = 0.0;
  C[21] = 0.0;
  C[25] = -V;
  C[29] = 0.0;
  C[33] = -1.0;
  C[37] = 0.0;
  C[2] = -2.0 * xhat[0] * g;
  C[6] = 2.0 * xhat[1] * g;
  C[10] = 2.0 * xhat[2] * g;
  C[14] = -2.0 * xhat[3] * g;
  C[18] = 0.0;
  C[22] = V;
  C[26] = 0.0;
  C[30] = 0.0;
  C[34] = 0.0;
  C[38] = -1.0;
  C[3] = -(2.0 * xhat[3] * ((2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3]))
            - 1.0)) * fact;
  C[7] = -(2.0 * xhat[2] * ((2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] * xhat[3]))
            - 1.0)) * fact;
  C[11] = -((2.0 * xhat[1] / ((2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] *
    xhat[3])) - 1.0) - 4.0 * xhat[2] * (2.0 * xhat[0] * xhat[3] + 2.0 * xhat[1] *
              xhat[2]) / (mphi * mphi)) * (mtheta * mtheta)) * fact;
  C[15] = -((2.0 * xhat[0] / ((2.0 * (xhat[2] * xhat[2]) + 2.0 * (xhat[3] *
    xhat[3])) - 1.0) - 4.0 * xhat[3] * (2.0 * xhat[0] * xhat[3] + 2.0 * xhat[1] *
              xhat[2]) / (a * a)) * (b_a * b_a)) * fact;
  C[19] = 0.0;
  C[23] = 0.0;
  C[27] = 0.0;
  C[31] = 0.0;
  C[35] = 0.0;
  C[39] = 0.0;
  for (k = 0; k < 10; k++) {
    for (i0 = 0; i0 < 4; i0++) {
      K[k + 10 * i0] = 0.0;
      for (i1 = 0; i1 < 10; i1++) {
        K[k + 10 * i0] += P[k + 10 * i1] * C[i0 + (i1 << 2)];
      }
    }
  }

  for (k = 0; k < 4; k++) {
    for (i0 = 0; i0 < 10; i0++) {
      c_C[k + (i0 << 2)] = 0.0;
      for (i1 = 0; i1 < 10; i1++) {
        c_C[k + (i0 << 2)] += C[k + (i1 << 2)] * P[i1 + 10 * i0];
      }
    }

    for (i0 = 0; i0 < 4; i0++) {
      mphi = 0.0;
      for (i1 = 0; i1 < 10; i1++) {
        mphi += c_C[k + (i1 << 2)] * C[i0 + (i1 << 2)];
      }

      b_C[k + (i0 << 2)] = mphi + R[k + (i0 << 2)];
    }
  }

  mrdivide(K, b_C);
  memset(&A[0], 0, 100U * sizeof(double));
  for (k = 0; k < 10; k++) {
    A[k + 10 * k] = 1.0;
  }

  for (k = 0; k < 10; k++) {
    for (i0 = 0; i0 < 10; i0++) {
      mphi = 0.0;
      for (i1 = 0; i1 < 4; i1++) {
        mphi += K[k + 10 * i1] * C[i1 + (i0 << 2)];
      }

      b_A[k + 10 * i0] = A[k + 10 * i0] - mphi;
    }

    for (i0 = 0; i0 < 10; i0++) {
      c_A[k + 10 * i0] = 0.0;
      for (i1 = 0; i1 < 10; i1++) {
        c_A[k + 10 * i0] += b_A[k + 10 * i1] * P[i1 + 10 * i0];
      }
    }
  }

  for (k = 0; k < 10; k++) {
    memcpy(&P[k * 10], &c_A[k * 10], 10U * sizeof(double));
  }

  /*  Calculate yaw angle from magnetometer readings */
  mphi = atan2(2.0 * (xhat[0] * xhat[1] + xhat[2] * xhat[3]), ((xhat[0] * xhat[0]
    + xhat[3] * xhat[3]) - xhat[1] * xhat[1]) - xhat[2] * xhat[2]);
  mtheta = std::asin(2.0 * (xhat[0] * xhat[2] - xhat[1] * xhat[3]));
  dv0[0] = ax - xhat[7];
  dv0[1] = ay - xhat[8];
  dv0[2] = az - xhat[9];
  dv0[3] = atan2(-my * std::cos(mphi) + mz * std::sin(mphi), mx * std::cos
                 (mtheta) + (my * std::sin(mphi) + mz * std::cos(mphi)) * std::
                 sin(mtheta));
  dv1[0] = g * (2.0 * xhat[0] * xhat[2] - 2.0 * xhat[1] * xhat[3]) - xhat[7];
  dv1[1] = (-V * (xhat[6] - r) - g * (2.0 * xhat[0] * xhat[1] + 2.0 * xhat[2] *
             xhat[3])) - xhat[8];
  dv1[2] = (V * (xhat[5] - q) - g * (((xhat[0] * xhat[0] - xhat[1] * xhat[1]) -
              xhat[2] * xhat[2]) + xhat[3] * xhat[3])) - xhat[9];
  dv1[3] = atan2(2.0 * (xhat[0] * xhat[3] + xhat[1] * xhat[2]), 1.0 - 2.0 *
                 (xhat[2] * xhat[2] + xhat[3] * xhat[3]));
  for (k = 0; k < 4; k++) {
    dv2[k] = dv0[k] - dv1[k];
  }

  for (k = 0; k < 10; k++) {
    mphi = 0.0;
    for (i0 = 0; i0 < 4; i0++) {
      mphi += K[k + 10 * i0] * dv2[i0];
    }

    xhat[k] += mphi;
  }

  /*  Again, extract updated states from vector */
  *phi = atan2(2.0 * (xhat[0] * xhat[1] + xhat[2] * xhat[3]), ((xhat[0] * xhat[0]
    + xhat[3] * xhat[3]) - xhat[1] * xhat[1]) - xhat[2] * xhat[2]) * 180.0 /
    3.1415926535897931;
  *theta = std::asin(2.0 * (xhat[0] * xhat[2] - xhat[1] * xhat[3])) * 180.0 /
    3.1415926535897931;
  *psi = atan2(2.0 * (xhat[0] * xhat[3] + xhat[1] * xhat[2]), ((xhat[0] * xhat[0]
    + xhat[1] * xhat[1]) - xhat[2] * xhat[2]) - xhat[3] * xhat[3]) * 180.0 /
    3.1415926535897931;
  if (*psi < 0.0) {
    *psi += 360.0;
  }
}

void updateEKF_initialize()
{
  updateEKF_init();
}

void updateEKF_terminate()
{
  /* (no terminate code required) */
}

/* End of code generation (updateEKF.cpp) */
