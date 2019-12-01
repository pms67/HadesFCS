/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * simpleEKFQuat.h
 *
 * Code generation for function 'simpleEKFQuat'
 *
 */

#ifndef SIMPLEEKFQUAT_H
#define SIMPLEEKFQUAT_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "simpleEKFQuat_types.h"

/* Function Declarations */
extern void simpleEKFQuat(const double gyr_rps[3], const double acc_mps2[3],
  double Va_mps, double T, double *rollDeg, double *pitchDeg);
extern void simpleEKFQuat_initialize(void);
extern void simpleEKFQuat_terminate(void);

#endif

/* End of code generation (simpleEKFQuat.h) */
