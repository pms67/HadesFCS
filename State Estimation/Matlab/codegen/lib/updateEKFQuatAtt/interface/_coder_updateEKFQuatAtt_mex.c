/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_updateEKFQuatAtt_mex.c
 *
 * Code generation for function '_coder_updateEKFQuatAtt_mex'
 *
 */

/* Include files */
#include "_coder_updateEKFQuatAtt_api.h"
#include "_coder_updateEKFQuatAtt_mex.h"

/* Function Declarations */
static void updateEKFQuatAtt_mexFunction(int32_T nlhs, mxArray *plhs[3], int32_T
  nrhs, const mxArray *prhs[7]);

/* Function Definitions */
static void updateEKFQuatAtt_mexFunction(int32_T nlhs, mxArray *plhs[3], int32_T
  nrhs, const mxArray *prhs[7])
{
  const mxArray *outputs[3];
  int32_T b_nlhs;
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  st.tls = emlrtRootTLSGlobal;

  /* Check for proper number of arguments. */
  if (nrhs != 7) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 7, 4,
                        16, "updateEKFQuatAtt");
  }

  if (nlhs > 3) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 16,
                        "updateEKFQuatAtt");
  }

  /* Call the function. */
  updateEKFQuatAtt_api(prhs, nlhs, outputs);

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
  mexAtExit(updateEKFQuatAtt_atexit);

  /* Module initialization. */
  updateEKFQuatAtt_initialize();

  /* Dispatch the entry-point. */
  updateEKFQuatAtt_mexFunction(nlhs, plhs, nrhs, prhs);

  /* Module termination. */
  updateEKFQuatAtt_terminate();
}

emlrtCTX mexFunctionCreateRootTLS(void)
{
  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  return emlrtRootTLSGlobal;
}

/* End of code generation (_coder_updateEKFQuatAtt_mex.c) */
