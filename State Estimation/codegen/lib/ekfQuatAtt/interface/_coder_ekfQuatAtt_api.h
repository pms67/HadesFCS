/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_ekfQuatAtt_api.h
 *
 * Code generation for function '_coder_ekfQuatAtt_api'
 *
 */

#ifndef _CODER_EKFQUATATT_API_H
#define _CODER_EKFQUATATT_API_H

/* Include files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_ekfQuatAtt_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void ekfQuatAtt(real_T gyr[3], real_T acc[3], real_T mag[3], real_T
  airspeed, real_T dt, real_T *roll, real_T *pitch, real_T *yaw);
extern void ekfQuatAtt_api(const mxArray * const prhs[5], int32_T nlhs, const
  mxArray *plhs[3]);
extern void ekfQuatAtt_atexit(void);
extern void ekfQuatAtt_initialize(void);
extern void ekfQuatAtt_terminate(void);
extern void ekfQuatAtt_xil_terminate(void);

#endif

/* End of code generation (_coder_ekfQuatAtt_api.h) */
