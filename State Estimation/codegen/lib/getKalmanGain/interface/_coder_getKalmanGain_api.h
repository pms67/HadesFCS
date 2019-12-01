/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_getKalmanGain_api.h
 *
 * Code generation for function '_coder_getKalmanGain_api'
 *
 */

#ifndef _CODER_GETKALMANGAIN_API_H
#define _CODER_GETKALMANGAIN_API_H

/* Include files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_getKalmanGain_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void getKalmanGain(real_T P[16], real_T C[12], real_T r, real_T K[12]);
extern void getKalmanGain_api(const mxArray * const prhs[3], int32_T nlhs, const
  mxArray *plhs[1]);
extern void getKalmanGain_atexit(void);
extern void getKalmanGain_initialize(void);
extern void getKalmanGain_terminate(void);
extern void getKalmanGain_xil_shutdown(void);
extern void getKalmanGain_xil_terminate(void);

#endif

/* End of code generation (_coder_getKalmanGain_api.h) */
