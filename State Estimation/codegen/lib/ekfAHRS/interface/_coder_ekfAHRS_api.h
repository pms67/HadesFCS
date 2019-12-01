/*
 * File: _coder_ekfAHRS_api.h
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 15-Apr-2019 03:04:04
 */

#ifndef _CODER_EKFAHRS_API_H
#define _CODER_EKFAHRS_API_H

/* Include Files */
#include "tmwtypes.h"
#include "mex.h"
#include "emlrt.h"
#include <stddef.h>
#include <stdlib.h>
#include "_coder_ekfAHRS_api.h"

/* Variable Declarations */
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

/* Function Declarations */
extern void ekfAHRS(real_T acc[3], real_T gyr[3], real_T mag[3], real_T airspeed,
                    real_T pressure, real_T dt, real_T *roll, real_T *pitch,
                    real_T *yaw, real_T *altitude);
extern void ekfAHRS_api(const mxArray *prhs[6], const mxArray *plhs[4]);
extern void ekfAHRS_atexit(void);
extern void ekfAHRS_initialize(void);
extern void ekfAHRS_terminate(void);
extern void ekfAHRS_xil_terminate(void);

#endif

/*
 * File trailer for _coder_ekfAHRS_api.h
 *
 * [EOF]
 */
