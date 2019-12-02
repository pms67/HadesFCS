/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * ekfQuatAtt.h
 *
 * Code generation for function 'ekfQuatAtt'
 *
 */

#ifndef EKFQUATATT_H
#define EKFQUATATT_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "ekfQuatAtt_types.h"

/* Function Declarations */
extern void ekfQuatAtt(const double gyr[3], const double acc[3], const double
  mag[3], double airspeed, double dt, double *roll, double *pitch, double *yaw);
extern void ekfQuatAtt_initialize();
extern void ekfQuatAtt_terminate();

#endif

/* End of code generation (ekfQuatAtt.h) */
