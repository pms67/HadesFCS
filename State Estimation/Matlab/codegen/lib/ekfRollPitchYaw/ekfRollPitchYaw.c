
#include "ekfRollPitchYaw.h"

static double g;
static double x[7];
static double P[49];
static double Q[49];
static double R[16];
static double noiseGyr;
static double noiseAcc;
static double noiseMag;
static double noiseBias;

static void ekfRollPitchYaw_init(void);
static void mrdivide(double A[28], const double B[16]);

static void ekfRollPitchYaw_init(void)
{
	g = 9.81;
	noiseGyr = 0.003455;
	noiseAcc = 0.015696;
	noiseMag = 0.004242;
	noiseBias = 0.0001;
  
	for (i = 0; i < 7; i++) {
	x[i] = 0.0;
	}

	x[0] = 1.0;
	v[0] = 3.0 * (noiseGyr * noiseGyr);
	v[1] = 3.0 * (noiseGyr * noiseGyr);
	v[2] = 3.0 * (noiseGyr * noiseGyr);
	v[3] = 3.0 * (noiseGyr * noiseGyr);
	v[4] = noiseBias * noiseBias;
	v[5] = noiseBias * noiseBias;
	v[6] = noiseBias * noiseBias;
	for (i = 0; i < 49; i++) {
	  P[i] = iv0[i];
	  Q[i] = 0.0;
	}

	for (i = 0; i < 7; i++) {
	  Q[i + 7 * i] = v[i];
	}

	b_v[0] = noiseAcc * noiseAcc;
	b_v[1] = noiseAcc * noiseAcc;
	b_v[2] = noiseAcc * noiseAcc;
	b_v[3] = noiseMag * noiseMag;
	memset(&R[0], 0, sizeof(double) << 4);
	for (i = 0; i < 4; i++) {
	  R[i + (i << 2)] = b_v[i];
	}
}

/*
 * Arguments    : double A[28]
 *                const double B[16]
 * Return Type  : void
 */
static void mrdivide(double A[28], const double B[16])
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
    jp = 7 * j;
    jAcol = j << 2;
    for (k = 1; k <= j; k++) {
      kBcol = 7 * (k - 1);
      if (b_A[(k + jAcol) - 1] != 0.0) {
        for (i = 0; i < 7; i++) {
          A[i + jp] -= b_A[(k + jAcol) - 1] * A[i + kBcol];
        }
      }
    }

    temp = 1.0 / b_A[j + jAcol];
    for (i = 0; i < 7; i++) {
      A[i + jp] *= temp;
    }
  }

  for (j = 3; j >= 0; j += -1) {
    jp = 7 * j;
    jAcol = (j << 2) - 1;
    for (k = j + 2; k < 5; k++) {
      kBcol = 7 * (k - 1);
      if (b_A[k + jAcol] != 0.0) {
        for (i = 0; i < 7; i++) {
          A[i + jp] -= b_A[k + jAcol] * A[i + kBcol];
        }
      }
    }
  }

  for (kBcol = 2; kBcol >= 0; kBcol += -1) {
    if (ipiv[kBcol] != kBcol + 1) {
      jp = ipiv[kBcol] - 1;
      for (jAcol = 0; jAcol < 7; jAcol++) {
        temp = A[jAcol + 7 * kBcol];
        A[jAcol + 7 * kBcol] = A[jAcol + 7 * jp];
        A[jAcol + 7 * jp] = temp;
      }
    }
  }
}

/*
 * Arguments    : const double gyr[3]
 *                const double acc[3]
 *                double psiMag
 *                double V
 *                double T
 *                double *roll
 *                double *pitch
 *                double *yaw
 * Return Type  : void
 */
void ekfRollPitchYaw(const double gyr[3], const double acc[3], double psiMag,
                     double V, double T, double *roll, double *pitch, double
                     *yaw)
{
  double q0;
  int i;
  double q1;
  double q2;
  double v[7];
  static const signed char iv0[49] = { 10, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0,
    0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0,
    0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 10 };

  double b_v[4];
  double dv0[49];
  double A[49];
  double b_A[49];
  int i0;
  double h[4];
  int i1;
  double a;
  double b_a;
  double C[28];
  double b_C[16];
  double c_C[28];
  double K[28];
  double B[49];
  double b_acc[4];


  /*  Extract states */
  q0 = x[0];
  q1 = x[1];
  q2 = x[2];

  /*  State transition function f(x,u) */
  x[0] += 0.5 * T * ((x[1] * (x[4] - gyr[0]) + x[2] * (x[5] - gyr[1])) + x[3] *
                     (x[6] - gyr[2]));
  x[1] += 0.5 * T * ((x[3] * (x[5] - gyr[1]) - q0 * (x[4] - gyr[0])) - x[2] *
                     (x[6] - gyr[2]));
  x[2] += 0.5 * T * ((q1 * (x[6] - gyr[2]) - q0 * (x[5] - gyr[1])) - x[3] * (x[4]
    - gyr[0]));
  x[3] += 0.5 * T * ((q2 * (x[4] - gyr[0]) - q1 * (x[5] - gyr[1])) - q0 * (x[6]
    - gyr[2]));

  /*  Normalise quaternion and re-extract states */
  q0 = 1.0 / sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] *= q0;
  x[1] *= q0;
  x[2] *= q0;
  x[3] *= q0;

  /*  Jacobian of f(x,u) */
  dv0[0] = 0.0;
  dv0[7] = x[4] - gyr[0];
  dv0[14] = x[5] - gyr[1];
  dv0[21] = x[6] - gyr[2];
  dv0[28] = x[1];
  dv0[35] = x[2];
  dv0[42] = x[3];
  dv0[1] = gyr[0] - x[4];
  dv0[8] = 0.0;
  dv0[15] = gyr[2] - x[6];
  dv0[22] = x[5] - gyr[1];
  dv0[29] = -x[0];
  dv0[36] = x[3];
  dv0[43] = -x[2];
  dv0[2] = gyr[1] - x[5];
  dv0[9] = x[6] - gyr[2];
  dv0[16] = 0.0;
  dv0[23] = gyr[0] - x[4];
  dv0[30] = -x[3];
  dv0[37] = -x[0];
  dv0[44] = x[1];
  dv0[3] = gyr[2] - x[6];
  dv0[10] = gyr[1] - x[5];
  dv0[17] = x[4] - gyr[0];
  dv0[24] = 0.0;
  dv0[31] = x[2];
  dv0[38] = -x[1];
  dv0[45] = -x[0];
  for (i = 0; i < 7; i++) {
    dv0[4 + 7 * i] = 0.0;
    dv0[5 + 7 * i] = 0.0;
    dv0[6 + 7 * i] = 0.0;
    for (i0 = 0; i0 < 7; i0++) {
      A[i0 + 7 * i] = 0.5 * dv0[i0 + 7 * i];
    }
  }

  /*  Update error covariance matrix */
  for (i = 0; i < 7; i++) {
    for (i0 = 0; i0 < 7; i0++) {
      b_A[i + 7 * i0] = 0.0;
      dv0[i + 7 * i0] = 0.0;
      for (i1 = 0; i1 < 7; i1++) {
        b_A[i + 7 * i0] += A[i + 7 * i1] * P[i1 + 7 * i0];
        dv0[i + 7 * i0] += P[i + 7 * i1] * A[i0 + 7 * i1];
      }
    }
  }

  for (i = 0; i < 7; i++) {
    for (i0 = 0; i0 < 7; i0++) {
      P[i0 + 7 * i] += T * ((b_A[i0 + 7 * i] + dv0[i0 + 7 * i]) + Q[i0 + 7 * i]);
    }
  }

  /*  Output function h(x,u) */
  h[0] = g * (2.0 * x[0] * x[2] - 2.0 * x[1] * x[3]);
  h[1] = -V * (x[6] - gyr[2]) - g * (2.0 * x[0] * x[1] + 2.0 * x[2] * x[3]);
  h[2] = V * (x[5] - gyr[1]) + g * ((2.0 * x[1] * x[1] + 2.0 * x[2] * x[2]) -
    1.0);
  h[3] = atan2(2.0 * x[0] * x[3] + 2.0 * x[1] * x[2], (-2.0 * x[2] * x[2] - 2.0 *
    x[3] * x[3]) + 1.0);

  /*  Jacobian of h(x,u) */
  q0 = (2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0;
  q1 = 2.0 * x[0] * x[3] + 2.0 * x[1] * x[2];
  q2 = 1.0 / (q0 * q0 + q1 * q1);
  q0 = (2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0;
  q1 = (2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0;
  a = (2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0;
  b_a = (2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0;
  C[0] = 2.0 * g * x[2];
  C[4] = -2.0 * g * x[3];
  C[8] = 2.0 * g * x[0];
  C[12] = -2.0 * g * x[1];
  C[16] = 0.0;
  C[20] = 0.0;
  C[24] = 0.0;
  C[1] = -2.0 * g * x[1];
  C[5] = -2.0 * g * x[0];
  C[9] = -2.0 * g * x[3];
  C[13] = -2.0 * g * x[2];
  C[17] = 0.0;
  C[21] = 0.0;
  C[25] = -V;
  C[2] = 0.0;
  C[6] = 4.0 * g * x[1];
  C[10] = 4.0 * g * x[2];
  C[14] = 0.0;
  C[18] = 0.0;
  C[22] = V;
  C[26] = 0.0;
  C[3] = -(2.0 * x[3] * ((2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0)) *
    q2;
  C[7] = -(2.0 * x[2] * ((2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0)) *
    q2;
  C[11] = -((2.0 * x[1] / ((2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0) -
             4.0 * x[2] * (2.0 * x[0] * x[3] + 2.0 * x[1] * x[2]) / (q0 * q0)) *
            (q1 * q1)) * q2;
  C[15] = -((2.0 * x[0] / ((2.0 * (x[2] * x[2]) + 2.0 * (x[3] * x[3])) - 1.0) -
             4.0 * x[3] * (2.0 * x[0] * x[3] + 2.0 * x[1] * x[2]) / (a * a)) *
            (b_a * b_a)) * q2;
  C[19] = 0.0;
  C[23] = 0.0;
  C[27] = 0.0;

  /*  Kalman gain */
  for (i = 0; i < 7; i++) {
    for (i0 = 0; i0 < 4; i0++) {
      K[i + 7 * i0] = 0.0;
      for (i1 = 0; i1 < 7; i1++) {
        K[i + 7 * i0] += P[i + 7 * i1] * C[i0 + (i1 << 2)];
      }
    }
  }

  for (i = 0; i < 4; i++) {
    for (i0 = 0; i0 < 7; i0++) {
      c_C[i + (i0 << 2)] = 0.0;
      for (i1 = 0; i1 < 7; i1++) {
        c_C[i + (i0 << 2)] += C[i + (i1 << 2)] * P[i1 + 7 * i0];
      }
    }

    for (i0 = 0; i0 < 4; i0++) {
      q0 = 0.0;
      for (i1 = 0; i1 < 7; i1++) {
        q0 += c_C[i + (i1 << 2)] * C[i0 + (i1 << 2)];
      }

      b_C[i + (i0 << 2)] = q0 + R[i + (i0 << 2)];
    }
  }

  mrdivide(K, b_C);

  /*  Update error covariance matrix */
  memset(&A[0], 0, 49U * sizeof(double));
  for (i = 0; i < 7; i++) {
    A[i + 7 * i] = 1.0;
  }

  memcpy(&B[0], &P[0], 49U * sizeof(double));
  for (i = 0; i < 7; i++) {
    for (i0 = 0; i0 < 7; i0++) {
      q0 = 0.0;
      for (i1 = 0; i1 < 4; i1++) {
        q0 += K[i + 7 * i1] * C[i1 + (i0 << 2)];
      }

      b_A[i + 7 * i0] = A[i + 7 * i0] - q0;
    }

    for (i0 = 0; i0 < 7; i0++) {
      P[i + 7 * i0] = 0.0;
      for (i1 = 0; i1 < 7; i1++) {
        P[i + 7 * i0] += b_A[i + 7 * i1] * B[i1 + 7 * i0];
      }
    }
  }

  /*  Update state estimate */
  b_acc[0] = acc[0];
  b_acc[1] = acc[1];
  b_acc[2] = acc[2];
  b_acc[3] = psiMag;
  for (i = 0; i < 4; i++) {
    b_v[i] = b_acc[i] - h[i];
  }

  for (i = 0; i < 7; i++) {
    q0 = 0.0;
    for (i0 = 0; i0 < 4; i0++) {
      q0 += K[i + 7 * i0] * b_v[i0];
    }

    x[i] += q0;
  }

  /*  Normalise quaternion and re-extract states */
  q0 = 1.0 / sqrt(((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]) + x[3] * x[3]);
  x[0] *= q0;
  x[1] *= q0;
  x[2] *= q0;
  x[3] *= q0;

  /*  Convert to Euler angles */
  *roll = atan2(2.0 * (x[0] * x[1] + x[2] * x[3]), 1.0 - 2.0 * (x[1] * x[1] + x
    [2] * x[2]));
  *pitch = asin(2.0 * (x[0] * x[2] - x[3] * x[1]));
  *yaw = atan2(2.0 * (x[0] * x[3] + x[1] * x[2]), 1.0 - 2.0 * (x[2] * x[2] + x[3]
    * x[3]));
}