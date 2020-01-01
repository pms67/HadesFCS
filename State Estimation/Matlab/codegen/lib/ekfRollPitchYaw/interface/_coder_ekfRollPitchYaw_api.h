/*
 * File: _coder_ekfRollPitchYaw_api.h
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 27-Dec-2019 22:11:10
 */

#ifndef _CODER_EKFROLLPITCHYAW_API_H
#define _CODER_EKFROLLPITCHYAW_API_H

/* Include Files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_ekfRollPitchYaw_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void ekfRollPitchYaw(real_T gyr[3], real_T acc[3], real_T psiMag, real_T
  V, real_T T, real_T *roll, real_T *pitch, real_T *yaw);
extern void ekfRollPitchYaw_api(const mxArray *prhs[5], const mxArray *plhs[3]);
extern void ekfRollPitchYaw_atexit(void);
extern void ekfRollPitchYaw_initialize(void);
extern void ekfRollPitchYaw_terminate(void);
extern void ekfRollPitchYaw_xil_terminate(void);

#endif

/*
 * File trailer for _coder_ekfRollPitchYaw_api.h
 *
 * [EOF]
 */
