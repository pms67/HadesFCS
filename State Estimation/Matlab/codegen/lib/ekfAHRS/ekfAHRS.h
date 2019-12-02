/*
 * File: ekfAHRS.h
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 15-Apr-2019 03:04:04
 */

#ifndef EKFAHRS_H
#define EKFAHRS_H

/* Include Files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rtwtypes.h"
#include "ekfAHRS_types.h"

/* Function Declarations */
extern void ekfAHRS(const double acc[3], const double gyr[3], const double mag[3],
                    double airspeed, double pressure, double dt, double *roll,
                    double *pitch, double *yaw, double *altitude);
extern void ekfAHRS_initialize(void);
extern void ekfAHRS_terminate(void);

#endif

/*
 * File trailer for ekfAHRS.h
 *
 * [EOF]
 */
