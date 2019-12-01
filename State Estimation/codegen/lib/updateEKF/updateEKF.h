/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKF.h
 *
 * Code generation for function 'updateEKF'
 *
 */

#ifndef UPDATEEKF_H
#define UPDATEEKF_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "updateEKF_types.h"

/* Function Declarations */
extern void updateEKF(const double gyr[3], const double acc[3], const double
                      mag[3], double airspeed, double dt, double *phi, double
                      *theta, double *psi);
extern void updateEKF_initialize();
extern void updateEKF_terminate();

#endif

/* End of code generation (updateEKF.h) */
