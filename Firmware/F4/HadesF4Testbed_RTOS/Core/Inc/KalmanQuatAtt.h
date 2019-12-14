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

/* Function Declarations */
extern void updateEKFQuatAtt(const float gyr_rps[3], const float acc_mps2[3],
  const float mag_unit[3], float Va_mps, float magDecRad, float T, float
  NdivT, float *roll_deg, float *pitch_deg, float *yaw_deg);
extern void updateEKFQuatAtt_initialize(void);
extern void updateEKFQuatAtt_terminate(void);

#endif

/* End of code generation (updateEKFQuatAtt.h) */
