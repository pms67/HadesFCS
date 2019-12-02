/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * getKalmanGain.h
 *
 * Code generation for function 'getKalmanGain'
 *
 */

#ifndef GETKALMANGAIN_H
#define GETKALMANGAIN_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "getKalmanGain_types.h"

/* Function Declarations */
extern void getKalmanGain(const double P[16], const double C[12], double r,
  double K[12]);
extern void getKalmanGain_initialize(void);
extern void getKalmanGain_terminate(void);

#endif

/* End of code generation (getKalmanGain.h) */
