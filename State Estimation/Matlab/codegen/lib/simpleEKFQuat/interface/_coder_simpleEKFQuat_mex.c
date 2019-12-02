/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_simpleEKFQuat_mex.c
 *
 * Code generation for function '_coder_simpleEKFQuat_mex'
 *
 */

/* Include files */
#include "_coder_simpleEKFQuat_api.h"
#include "_coder_simpleEKFQuat_mex.h"

/* Function Declarations */
static void simpleEKFQuat_mexFunction(int32_T nlhs, mxArray *plhs[2], int32_T
  nrhs, const mxArray *prhs[4]);

/* Function Definitions */
static void simpleEKFQuat_mexFunction(int32_T nlhs, mxArray *plhs[2], int32_T
  nrhs, const mxArray *prhs[4])
{
  const mxArray *outputs[2];
  int32_T b_nlhs;
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  st.tls = emlrtRootTLSGlobal;

  /* Check for proper number of arguments. */
  if (nrhs != 4) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 4, 4,
                        13, "simpleEKFQuat");
  }

  if (nlhs > 2) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 13,
                        "simpleEKFQuat");
  }

  /* Call the function. */
  simpleEKFQuat_api(prhs, nlhs, outputs);

  /* Copy over outputs to the caller. */
  if (nlhs < 1) {
    b_nlhs = 1;
  } else {
    b_nlhs = nlhs;
  }

  emlrtReturnArrays(b_nlhs, plhs, outputs);
}

void mexFunction(int32_T nlhs, mxArray *plhs[], int32_T nrhs, const mxArray
                 *prhs[])
{
  mexAtExit(simpleEKFQuat_atexit);

  /* Module initialization. */
  simpleEKFQuat_initialize();

  /* Dispatch the entry-point. */
  simpleEKFQuat_mexFunction(nlhs, plhs, nrhs, prhs);

  /* Module termination. */
  simpleEKFQuat_terminate();
}

emlrtCTX mexFunctionCreateRootTLS(void)
{
  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  return emlrtRootTLSGlobal;
}

/* End of code generation (_coder_simpleEKFQuat_mex.c) */
