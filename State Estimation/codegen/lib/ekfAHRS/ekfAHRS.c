/*
 * File: ekfAHRS.c
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 15-Apr-2019 03:04:04
 */

/* Include Files */
#include "ekfAHRS.h"

/* Variable Definitions */
static boolean_T flag_not_empty;
static double P[81];
static double Q[81];
static double R[16];
static double xhat[9];
static double g;
static double lpfGyrCoeff;
static double lpfAccCoeff;
static double lpfVaCoeff;
static double p;
static double q;
static double r;
static double ax;
static double ay;
static double az;
static double V;

/* Function Declarations */
static void ekfAHRS_init(void);
static void mrdivide(double A[36], const double B[16]);

/* Function Definitions */

/*
 * Arguments    : void
 * Return Type  : void
 */
static void ekfAHRS_init(void)
{
  g = 9.80665;
  memset(&xhat[0], 0, 9U * sizeof(double));
  lpfGyrCoeff = 0.8;
  lpfAccCoeff = 0.6;
  lpfVaCoeff = 0.75;
  p = 0.0;
  q = 0.0;
  r = 0.0;
  ax = 0.0;
  ay = 0.0;
  az = 0.0;
  V = 0.0;
}

/*
 * Arguments    : double A[36]
 *                const double B[16]
 * Return Type  : void
 */
static void mrdivide(double A[36], const double B[16])
{
  double b_A[16];
  signed char ipiv[4];
  int k;
  int j;
  int c;
  int kBcol;
  int jp;
  int ix;
  int jAcol;
  double temp;
  int i;
  double s;
  memcpy(&b_A[0], &B[0], sizeof(double) << 4);
  for (k = 0; k < 4; k++) {
    ipiv[k] = (signed char)(1 + k);
  }

  for (j = 0; j < 3; j++) {
    c = j * 5;
    kBcol = 0;
    ix = c;
    temp = fabs(b_A[c]);
    for (k = 2; k <= 4 - j; k++) {
      ix++;
      s = fabs(b_A[ix]);
      if (s > temp) {
        kBcol = k - 1;
        temp = s;
      }
    }

    if (b_A[c + kBcol] != 0.0) {
      if (kBcol != 0) {
        ipiv[j] = (signed char)((j + kBcol) + 1);
        ix = j;
        kBcol += j;
        for (k = 0; k < 4; k++) {
          temp = b_A[ix];
          b_A[ix] = b_A[kBcol];
          b_A[kBcol] = temp;
          ix += 4;
          kBcol += 4;
        }
      }

      k = (c - j) + 4;
      for (i = c + 1; i + 1 <= k; i++) {
        b_A[i] /= b_A[c];
      }
    }

    jp = c;
    jAcol = c + 4;
    for (kBcol = 1; kBcol <= 3 - j; kBcol++) {
      temp = b_A[jAcol];
      if (b_A[jAcol] != 0.0) {
        ix = c + 1;
        k = (jp - j) + 8;
        for (i = 5 + jp; i + 1 <= k; i++) {
          b_A[i] += b_A[ix] * -temp;
          ix++;
        }
      }

      jAcol += 4;
      jp += 4;
    }
  }

  for (j = 0; j < 4; j++) {
    jp = 9 * j;
    jAcol = j << 2;
    for (k = 1; k <= j; k++) {
      kBcol = 9 * (k - 1);
      if (b_A[(k + jAcol) - 1] != 0.0) {
        for (i = 0; i < 9; i++) {
          A[i + jp] -= b_A[(k + jAcol) - 1] * A[i + kBcol];
        }
      }
    }

    temp = 1.0 / b_A[j + jAcol];
    for (i = 0; i < 9; i++) {
      A[i + jp] *= temp;
    }
  }

  for (j = 3; j >= 0; j += -1) {
    jp = 9 * j;
    jAcol = (j << 2) - 1;
    for (k = j + 2; k < 5; k++) {
      kBcol = 9 * (k - 1);
      if (b_A[k + jAcol] != 0.0) {
        for (i = 0; i < 9; i++) {
          A[i + jp] -= b_A[k + jAcol] * A[i + kBcol];
        }
      }
    }
  }

  for (kBcol = 2; kBcol >= 0; kBcol += -1) {
    if (ipiv[kBcol] != kBcol + 1) {
      jp = ipiv[kBcol] - 1;
      for (jAcol = 0; jAcol < 9; jAcol++) {
        temp = A[jAcol + 9 * kBcol];
        A[jAcol + 9 * kBcol] = A[jAcol + 9 * jp];
        A[jAcol + 9 * jp] = temp;
      }
    }
  }
}

/*
 * Arguments    : const double acc[3]
 *                const double gyr[3]
 *                const double mag[3]
 *                double airspeed
 *                double pressure
 *                double dt
 *                double *roll
 *                double *pitch
 *                double *yaw
 *                double *altitude
 * Return Type  : void
 */
void ekfAHRS(const double acc[3], const double gyr[3], const double mag[3],
             double airspeed, double pressure, double dt, double *roll, double
             *pitch, double *yaw, double *altitude)
{
  int j;
  static const double dv0[81] = { 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0001, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0001, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0001, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0001 };

  static const double dv1[9] = { 0.1, 0.1, 0.1, 1.0E-7, 1.0E-7, 1.0E-7, 1.0E-6,
    1.0E-6, 1.0E-6 };

  static const signed char iv0[16] = { 10, 0, 0, 0, 0, 10, 0, 0, 0, 0, 10, 0, 0,
    0, 0, 10 };

  int IT;
  double phi;
  double theta;
  double psi;
  double bp;
  double bq;
  double br;
  double bx;
  double A[81];
  double by;
  double bz;
  double C[36];
  double dv2[9];
  static const signed char iv1[9] = { 0, 0, 1, 0, 0, 0, 0, 0, 0 };

  double b_C[16];
  int i0;
  double c_C[36];
  double K[36];
  int i1;
  double b_A[81];
  double dv3[81];
  double d0;
  double B[81];
  double dv4[4];
  double dv5[4];
  double dv6[4];
  if (!flag_not_empty) {
    flag_not_empty = true;
    for (j = 0; j < 81; j++) {
      P[j] = dv0[j];
      Q[j] = 0.0;
    }

    for (j = 0; j < 9; j++) {
      Q[j + 9 * j] = dv1[j];
    }

    for (j = 0; j < 16; j++) {
      R[j] = iv0[j];
    }
  }

  p = lpfGyrCoeff * p + (1.0 - lpfGyrCoeff) * gyr[0];
  q = lpfGyrCoeff * q + (1.0 - lpfGyrCoeff) * gyr[1];
  r = lpfGyrCoeff * r + (1.0 - lpfGyrCoeff) * gyr[2];
  ax = lpfAccCoeff * ax + (1.0 - lpfAccCoeff) * acc[0];
  ay = lpfAccCoeff * ay + (1.0 - lpfAccCoeff) * acc[1];
  az = lpfAccCoeff * az + (1.0 - lpfAccCoeff) * acc[2];
  V = lpfVaCoeff * V + (1.0 - lpfVaCoeff) * airspeed;
  *altitude = -8576.6 * log(pressure / 101325.0);
  for (IT = 0; IT < 10; IT++) {
    /*  State transition function */
    /*  Jacobian of f     */
    phi = tan(xhat[1]);
    theta = tan(xhat[1]);
    psi = cos(xhat[1]);
    bp = cos(xhat[1]);
    A[0] = sin(xhat[0]) * tan(xhat[1]) * (xhat[5] - r) - cos(xhat[0]) * tan
      (xhat[1]) * (xhat[4] - q);
    A[9] = -cos(xhat[0]) * (xhat[5] - r) * (phi * phi + 1.0) - sin(xhat[0]) *
      (xhat[4] - q) * (theta * theta + 1.0);
    A[18] = 0.0;
    A[27] = -1.0;
    A[36] = -sin(xhat[0]) * tan(xhat[1]);
    A[45] = -cos(xhat[0]) * tan(xhat[1]);
    A[54] = 0.0;
    A[63] = 0.0;
    A[72] = 0.0;
    A[1] = cos(xhat[0]) * (xhat[5] - r) + sin(xhat[0]) * (xhat[4] - q);
    A[10] = 0.0;
    A[19] = 0.0;
    A[28] = 0.0;
    A[37] = -cos(xhat[0]);
    A[46] = sin(xhat[0]);
    A[55] = 0.0;
    A[64] = 0.0;
    A[73] = 0.0;
    A[2] = sin(xhat[0]) * (xhat[5] - r) / cos(xhat[1]) - cos(xhat[0]) * (xhat[4]
      - q) / cos(xhat[1]);
    A[11] = -(cos(xhat[0]) * sin(xhat[1]) * (xhat[5] - r)) / (psi * psi) - sin
      (xhat[0]) * sin(xhat[1]) * (xhat[4] - q) / (bp * bp);
    A[20] = 0.0;
    A[29] = 0.0;
    A[38] = -sin(xhat[0]) / cos(xhat[1]);
    A[47] = -cos(xhat[0]) / cos(xhat[1]);
    A[56] = 0.0;
    A[65] = 0.0;
    A[74] = 0.0;
    phi = dt / 10.0;
    dv2[0] = ((p - xhat[3]) - cos(xhat[0]) * tan(xhat[1]) * (xhat[5] - r)) - sin
      (xhat[0]) * tan(xhat[1]) * (xhat[4] - q);
    dv2[1] = sin(xhat[0]) * (xhat[5] - r) - cos(xhat[0]) * (xhat[4] - q);
    dv2[2] = -(sin(xhat[0]) * (xhat[4] - q)) / cos(xhat[1]) - cos(xhat[0]) *
      (xhat[5] - r) / cos(xhat[1]);
    dv2[3] = 0.0;
    dv2[4] = 0.0;
    dv2[5] = 0.0;
    dv2[6] = 0.0;
    dv2[7] = 0.0;
    dv2[8] = 0.0;
    for (j = 0; j < 9; j++) {
      A[3 + 9 * j] = 0.0;
      A[4 + 9 * j] = 0.0;
      A[5 + 9 * j] = 0.0;
      A[6 + 9 * j] = 0.0;
      A[7 + 9 * j] = 0.0;
      A[8 + 9 * j] = 0.0;
      xhat[j] += phi * dv2[j];
    }

    phi = dt / 10.0;
    for (j = 0; j < 9; j++) {
      for (i0 = 0; i0 < 9; i0++) {
        b_A[j + 9 * i0] = 0.0;
        dv3[j + 9 * i0] = 0.0;
        for (i1 = 0; i1 < 9; i1++) {
          b_A[j + 9 * i0] += A[j + 9 * i1] * P[i1 + 9 * i0];
          dv3[j + 9 * i0] += P[j + 9 * i1] * A[i0 + 9 * i1];
        }
      }
    }

    for (j = 0; j < 9; j++) {
      for (i0 = 0; i0 < 9; i0++) {
        P[i0 + 9 * j] += phi * ((b_A[i0 + 9 * j] + dv3[i0 + 9 * j]) + Q[i0 + 9 *
          j]);
      }
    }
  }

  phi = xhat[0];
  theta = xhat[1];
  psi = xhat[2];
  bp = xhat[3];
  bq = xhat[4];
  br = xhat[5];
  bx = xhat[6];
  by = xhat[7];
  bz = xhat[8];

  /*  Jacobian of h    */
  C[0] = 0.0;
  C[4] = g * cos(xhat[1]) - V * cos(xhat[1]) * (xhat[4] - q);
  C[8] = 0.0;
  C[12] = 0.0;
  C[16] = -V * sin(xhat[1]);
  C[20] = 0.0;
  C[24] = -1.0;
  C[28] = 0.0;
  C[32] = 0.0;
  C[1] = -g * cos(xhat[0]) * cos(xhat[1]);
  C[5] = (V * cos(xhat[1]) * (xhat[3] - p) + V * sin(xhat[1]) * (xhat[5] - r)) +
    g * sin(xhat[0]) * sin(xhat[1]);
  C[9] = 0.0;
  C[13] = V * sin(xhat[1]);
  C[17] = 0.0;
  C[21] = -V * cos(xhat[1]);
  C[25] = 0.0;
  C[29] = -1.0;
  C[33] = 0.0;
  C[2] = g * cos(xhat[1]) * sin(xhat[0]);
  C[6] = g * cos(xhat[0]) * sin(xhat[1]) - V * sin(xhat[1]) * (xhat[4] - q);
  C[10] = 0.0;
  C[14] = 0.0;
  C[18] = V * cos(xhat[1]);
  C[22] = 0.0;
  C[26] = 0.0;
  C[30] = 0.0;
  C[34] = -1.0;
  for (j = 0; j < 9; j++) {
    C[3 + (j << 2)] = iv1[j];
  }

  for (j = 0; j < 9; j++) {
    for (i0 = 0; i0 < 4; i0++) {
      K[j + 9 * i0] = 0.0;
      for (i1 = 0; i1 < 9; i1++) {
        K[j + 9 * i0] += P[j + 9 * i1] * C[i0 + (i1 << 2)];
      }
    }
  }

  for (j = 0; j < 4; j++) {
    for (i0 = 0; i0 < 9; i0++) {
      c_C[j + (i0 << 2)] = 0.0;
      for (i1 = 0; i1 < 9; i1++) {
        c_C[j + (i0 << 2)] += C[j + (i1 << 2)] * P[i1 + 9 * i0];
      }
    }

    for (i0 = 0; i0 < 4; i0++) {
      d0 = 0.0;
      for (i1 = 0; i1 < 9; i1++) {
        d0 += c_C[j + (i1 << 2)] * C[i0 + (i1 << 2)];
      }

      b_C[j + (i0 << 2)] = d0 + R[j + (i0 << 2)];
    }
  }

  mrdivide(K, b_C);
  memset(&A[0], 0, 81U * sizeof(double));
  for (j = 0; j < 9; j++) {
    A[j + 9 * j] = 1.0;
  }

  memcpy(&B[0], &P[0], 81U * sizeof(double));
  for (j = 0; j < 9; j++) {
    for (i0 = 0; i0 < 9; i0++) {
      d0 = 0.0;
      for (i1 = 0; i1 < 4; i1++) {
        d0 += K[j + 9 * i1] * C[i1 + (i0 << 2)];
      }

      b_A[j + 9 * i0] = A[j + 9 * i0] - d0;
    }

    for (i0 = 0; i0 < 9; i0++) {
      P[j + 9 * i0] = 0.0;
      for (i1 = 0; i1 < 9; i1++) {
        P[j + 9 * i0] += b_A[j + 9 * i1] * B[i1 + 9 * i0];
      }
    }
  }

  /*  Output function h     */
  dv4[0] = ax;
  dv4[1] = ay;
  dv4[2] = az;
  dv4[3] = atan2(-mag[1] * cos(phi) + mag[2] * sin(phi), mag[0] * cos(theta) +
                 (mag[1] * sin(phi) + mag[2] * cos(phi)) * sin(theta)) +
    0.87266462599716477;
  dv5[0] = (g * sin(theta) - V * sin(theta) * (bq - q)) - bx;
  dv5[1] = ((V * sin(theta) * (bp - p) - V * cos(theta) * (br - r)) - g * cos
            (theta) * sin(phi)) - by;
  dv5[2] = (V * cos(theta) * (bq - q) - g * cos(phi) * cos(theta)) - bz;
  dv5[3] = psi;
  for (j = 0; j < 4; j++) {
    dv6[j] = dv4[j] - dv5[j];
  }

  for (j = 0; j < 9; j++) {
    d0 = 0.0;
    for (i0 = 0; i0 < 4; i0++) {
      d0 += K[j + 9 * i0] * dv6[i0];
    }

    xhat[j] += d0;
  }

  *roll = xhat[0] * 180.0 / 3.1415926535897931;
  *pitch = xhat[1] * 180.0 / 3.1415926535897931;
  *yaw = xhat[2] * 180.0 / 3.1415926535897931;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void ekfAHRS_initialize(void)
{
  flag_not_empty = false;
  ekfAHRS_init();
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void ekfAHRS_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for ekfAHRS.c
 *
 * [EOF]
 */
