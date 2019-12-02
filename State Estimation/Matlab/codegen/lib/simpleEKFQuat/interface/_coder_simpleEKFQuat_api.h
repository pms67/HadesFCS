/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_simpleEKFQuat_api.h
 *
 * Code generation for function '_coder_simpleEKFQuat_api'
 *
 */

#ifndef _CODER_SIMPLEEKFQUAT_API_H
#define _CODER_SIMPLEEKFQUAT_API_H

/* Include files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_simpleEKFQuat_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void simpleEKFQuat(real_T gyr_rps[3], real_T acc_mps2[3], real_T Va_mps,
  real_T T, real_T *rollDeg, real_T *pitchDeg);
extern void simpleEKFQuat_api(const mxArray * const prhs[4], int32_T nlhs, const
  mxArray *plhs[2]);
extern void simpleEKFQuat_atexit(void);
extern void simpleEKFQuat_initialize(void);
extern void simpleEKFQuat_terminate(void);
extern void simpleEKFQuat_xil_shutdown(void);
extern void simpleEKFQuat_xil_terminate(void);

#endif

/* End of code generation (_coder_simpleEKFQuat_api.h) */
