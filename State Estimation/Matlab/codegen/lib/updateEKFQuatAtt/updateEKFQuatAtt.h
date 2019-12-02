/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * updateEKFQuatAtt.h
 *
 * Code generation for function 'updateEKFQuatAtt'
 *
 */

#ifndef UPDATEEKFQUATATT_H
#define UPDATEEKFQUATATT_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "updateEKFQuatAtt_types.h"

/* Function Declarations */
extern void updateEKFQuatAtt(const double gyr_rps[3], const double acc_mps2[3],
  const double mag_unit[3], double Va_mps, double magDecRad, double T, double
  NdivT, double *roll_deg, double *pitch_deg, double *yaw_deg);
extern void updateEKFQuatAtt_initialize(void);
extern void updateEKFQuatAtt_terminate(void);

#endif

/* End of code generation (updateEKFQuatAtt.h) */
