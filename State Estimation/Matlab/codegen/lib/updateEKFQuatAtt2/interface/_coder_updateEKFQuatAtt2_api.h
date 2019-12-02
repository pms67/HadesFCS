/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_updateEKFQuatAtt2_api.h
 *
 * Code generation for function '_coder_updateEKFQuatAtt2_api'
 *
 */

#ifndef _CODER_UPDATEEKFQUATATT2_API_H
#define _CODER_UPDATEEKFQUATATT2_API_H

/* Include files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_updateEKFQuatAtt2_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void updateEKFQuatAtt2(real_T gyr_rps[3], real_T acc_mps2[3], real_T
  mag_unit[3], real_T Va_mps, real_T T, real_T *roll_deg, real_T *pitch_deg,
  real_T *yaw_deg, real_T *yaw_mag_deg);
extern void updateEKFQuatAtt2_api(const mxArray * const prhs[5], int32_T nlhs,
  const mxArray *plhs[4]);
extern void updateEKFQuatAtt2_atexit(void);
extern void updateEKFQuatAtt2_initialize(void);
extern void updateEKFQuatAtt2_terminate(void);
extern void updateEKFQuatAtt2_xil_shutdown(void);
extern void updateEKFQuatAtt2_xil_terminate(void);

#endif

/* End of code generation (_coder_updateEKFQuatAtt2_api.h) */
