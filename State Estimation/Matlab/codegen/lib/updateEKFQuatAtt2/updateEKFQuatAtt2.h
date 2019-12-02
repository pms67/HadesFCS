/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKFQuatAtt2.h
 *
 * Code generation for function 'updateEKFQuatAtt2'
 *
 */

#ifndef UPDATEEKFQUATATT2_H
#define UPDATEEKFQUATATT2_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "updateEKFQuatAtt2_types.h"

/* Function Declarations */
extern void updateEKFQuatAtt2(const double gyr_rps[3], const double acc_mps2[3],
  const double mag_unit[3], double Va_mps, double T, double *roll_deg, double
  *pitch_deg, double *yaw_deg, double *yaw_mag_deg);
extern void updateEKFQuatAtt2_initialize(void);
extern void updateEKFQuatAtt2_terminate(void);

#endif

/* End of code generation (updateEKFQuatAtt2.h) */
