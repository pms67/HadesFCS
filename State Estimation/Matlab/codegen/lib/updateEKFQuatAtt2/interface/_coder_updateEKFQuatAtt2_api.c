/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * _coder_updateEKFQuatAtt2_api.c
 *
 * Code generation for function '_coder_updateEKFQuatAtt2_api'
 *
 */

/* Include files */
#include "tmwtypes.h"
#include "_coder_updateEKFQuatAtt2_api.h"
#include "_coder_updateEKFQuatAtt2_mex.h"

/* Variable Definitions */
emlrtCTX emlrtRootTLSGlobal = NULL;
emlrtContext emlrtContextGlobal = { true,/* bFirstTime */
  false,                               /* bInitialized */
  131482U,                             /* fVersionInfo */
  NULL,                                /* fErrorFunction */
  "updateEKFQuatAtt2",                 /* fFunctionName */
  NULL,                                /* fRTCallStack */
  false,                               /* bDebugMode */
  { 2045744189U, 2170104910U, 2743257031U, 4284093946U },/* fSigWrd */
  NULL                                 /* fSigMem */
};

/* Function Declarations */
static real_T (*b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId))[3];
static real_T c_emlrt_marshallIn(const emlrtStack *sp, const mxArray *Va_mps,
  const char_T *identifier);
static real_T d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId);
static real_T (*e_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
  const emlrtMsgIdentifier *msgId))[3];
static real_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *gyr_rps,
  const char_T *identifier))[3];
static const mxArray *emlrt_marshallOut(const real_T u);
static real_T f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId);

/* Function Definitions */
static real_T (*b_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId))[3]
{
  real_T (*y)[3];
  y = e_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}
  static real_T c_emlrt_marshallIn(const emlrtStack *sp, const mxArray *Va_mps,
  const char_T *identifier)
{
  real_T y;
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = (const char *)identifier;
  thisId.fParent = NULL;
  thisId.bParentIsCell = false;
  y = d_emlrt_marshallIn(sp, emlrtAlias(Va_mps), &thisId);
  emlrtDestroyArray(&Va_mps);
  return y;
}

static real_T d_emlrt_marshallIn(const emlrtStack *sp, const mxArray *u, const
  emlrtMsgIdentifier *parentId)
{
  real_T y;
  y = f_emlrt_marshallIn(sp, emlrtAlias(u), parentId);
  emlrtDestroyArray(&u);
  return y;
}

static real_T (*e_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src,
  const emlrtMsgIdentifier *msgId))[3]
{
  real_T (*ret)[3];
  static const int32_T dims[1] = { 3 };

  emlrtCheckBuiltInR2012b(sp, msgId, src, "double", false, 1U, dims);
  ret = (real_T (*)[3])emlrtMxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}
  static real_T (*emlrt_marshallIn(const emlrtStack *sp, const mxArray *gyr_rps,
  const char_T *identifier))[3]
{
  real_T (*y)[3];
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = (const char *)identifier;
  thisId.fParent = NULL;
  thisId.bParentIsCell = false;
  y = b_emlrt_marshallIn(sp, emlrtAlias(gyr_rps), &thisId);
  emlrtDestroyArray(&gyr_rps);
  return y;
}

static const mxArray *emlrt_marshallOut(const real_T u)
{
  const mxArray *y;
  const mxArray *m0;
  y = NULL;
  m0 = emlrtCreateDoubleScalar(u);
  emlrtAssign(&y, m0);
  return y;
}

static real_T f_emlrt_marshallIn(const emlrtStack *sp, const mxArray *src, const
  emlrtMsgIdentifier *msgId)
{
  real_T ret;
  static const int32_T dims = 0;
  emlrtCheckBuiltInR2012b(sp, msgId, src, "double", false, 0U, &dims);
  ret = *(real_T *)emlrtMxGetData(src);
  emlrtDestroyArray(&src);
  return ret;
}

void updateEKFQuatAtt2_api(const mxArray * const prhs[5], int32_T nlhs, const
  mxArray *plhs[4])
{
  real_T (*gyr_rps)[3];
  real_T (*acc_mps2)[3];
  real_T (*mag_unit)[3];
  real_T Va_mps;
  real_T T;
  real_T roll_deg;
  real_T pitch_deg;
  real_T yaw_deg;
  real_T yaw_mag_deg;
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  st.tls = emlrtRootTLSGlobal;

  /* Marshall function inputs */
  gyr_rps = emlrt_marshallIn(&st, emlrtAlias(prhs[0]), "gyr_rps");
  acc_mps2 = emlrt_marshallIn(&st, emlrtAlias(prhs[1]), "acc_mps2");
  mag_unit = emlrt_marshallIn(&st, emlrtAlias(prhs[2]), "mag_unit");
  Va_mps = c_emlrt_marshallIn(&st, emlrtAliasP(prhs[3]), "Va_mps");
  T = c_emlrt_marshallIn(&st, emlrtAliasP(prhs[4]), "T");

  /* Invoke the target function */
  updateEKFQuatAtt2(*gyr_rps, *acc_mps2, *mag_unit, Va_mps, T, &roll_deg,
                    &pitch_deg, &yaw_deg, &yaw_mag_deg);

  /* Marshall function outputs */
  plhs[0] = emlrt_marshallOut(roll_deg);
  if (nlhs > 1) {
    plhs[1] = emlrt_marshallOut(pitch_deg);
  }

  if (nlhs > 2) {
    plhs[2] = emlrt_marshallOut(yaw_deg);
  }

  if (nlhs > 3) {
    plhs[3] = emlrt_marshallOut(yaw_mag_deg);
  }
}

void updateEKFQuatAtt2_atexit(void)
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtEnterRtStackR2012b(&st);
  emlrtLeaveRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
  updateEKFQuatAtt2_xil_terminate();
  updateEKFQuatAtt2_xil_shutdown();
  emlrtExitTimeCleanup(&emlrtContextGlobal);
}

void updateEKFQuatAtt2_initialize(void)
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtClearAllocCountR2012b(&st, false, 0U, 0);
  emlrtEnterRtStackR2012b(&st);
  emlrtFirstTimeR2012b(emlrtRootTLSGlobal);
}

void updateEKFQuatAtt2_terminate(void)
{
  emlrtStack st = { NULL,              /* site */
    NULL,                              /* tls */
    NULL                               /* prev */
  };

  st.tls = emlrtRootTLSGlobal;
  emlrtLeaveRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

/* End of code generation (_coder_updateEKFQuatAtt2_api.c) */
