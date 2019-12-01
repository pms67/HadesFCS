/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_updateEKFQuatAtt2_mex.c
 *
 * Code generation for function '_coder_updateEKFQuatAtt2_mex'
 *
 */

/* Include files */
#include "_coder_updateEKFQuatAtt2_api.h"
#include "_coder_updateEKFQuatAtt2_mex.h"

/* Function Declarations */
static void updateEKFQuatAtt2_mexFunction(int32_T nlhs, mxArray *plhs[4],
  int32_T nrhs, const mxArray *prhs[5]);

/* Function Definitions */
static void updateEKFQuatAtt2_mexFunction(int32_T nlhs, mxArray *plhs[4],
  int32_T nrhs, const mxArray *prhs[5])
{
  const mxArray *outputs[4];
  int32_T b_nlhs;
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  st.tls = emlrtRootTLSGlobal;

  /* Check for proper number of arguments. */
  if (nrhs != 5) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 5, 4,
                        17, "updateEKFQuatAtt2");
  }

  if (nlhs > 4) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 17,
                        "updateEKFQuatAtt2");
  }

  /* Call the function. */
  updateEKFQuatAtt2_api(prhs, nlhs, outputs);

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
  mexAtExit(updateEKFQuatAtt2_atexit);

  /* Module initialization. */
  updateEKFQuatAtt2_initialize();

  /* Dispatch the entry-point. */
  updateEKFQuatAtt2_mexFunction(nlhs, plhs, nrhs, prhs);

  /* Module termination. */
  updateEKFQuatAtt2_terminate();
}

emlrtCTX mexFunctionCreateRootTLS(void)
{
  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  return emlrtRootTLSGlobal;
}

/* End of code generation (_coder_updateEKFQuatAtt2_mex.c) */
