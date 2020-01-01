/*
 * File: ekfRollPitchYaw.h
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 27-Dec-2019 22:11:10
 */

#ifndef EKFROLLPITCHYAW_H
#define EKFROLLPITCHYAW_H

/* Include Files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rtwtypes.h"
#include "ekfRollPitchYaw_types.h"

/* Function Declarations */
extern void ekfRollPitchYaw(const double gyr[3], const double acc[3], double
  psiMag, double V, double T, double *roll, double *pitch, double *yaw);
extern void ekfRollPitchYaw_initialize(void);
extern void ekfRollPitchYaw_terminate(void);

#endif

/*
 * File trailer for ekfRollPitchYaw.h
 *
 * [EOF]
 */
