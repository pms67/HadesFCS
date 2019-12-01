/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_updateEKF_api.h
 *
 * Code generation for function '_coder_updateEKF_api'
 *
 */

#ifndef _CODER_UPDATEEKF_API_H
#define _CODER_UPDATEEKF_API_H

/* Include files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_updateEKF_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void updateEKF(real_T gyr[3], real_T acc[3], real_T mag[3], real_T
                      airspeed, real_T dt, real_T *phi, real_T *theta, real_T
                      *psi);
extern void updateEKF_api(const mxArray * const prhs[5], int32_T nlhs, const
  mxArray *plhs[3]);
extern void updateEKF_atexit(void);
extern void updateEKF_initialize(void);
extern void updateEKF_terminate(void);
extern void updateEKF_xil_terminate(void);

#endif

/* End of code generation (_coder_updateEKF_api.h) */
