/*
 * File: main.c
 *
 * MATLAB Coder version            : 3.2
 * C/C++ source code generated on  : 27-Dec-2019 22:11:10
 */

/*************************************************************************/
/* This automatically generated example C main file shows how to call    */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/
/* Include Files */
#include "ekfRollPitchYaw.h"
#include "main.h"

/* Function Declarations */
static void argInit_3x1_real_T(double result[3]);
static double argInit_real_T(void);
static void main_ekfRollPitchYaw(void);

/* Function Definitions */

/*
 * Arguments    : double result[3]
 * Return Type  : void
 */
static void argInit_3x1_real_T(double result[3])
{
  int idx0;

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < 3; idx0++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0] = argInit_real_T();
  }
}

/*
 * Arguments    : void
 * Return Type  : double
 */
static double argInit_real_T(void)
{
  return 0.0;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void main_ekfRollPitchYaw(void)
{
  double dv1[3];
  double dv2[3];
  double roll;
  double pitch;
  double yaw;

  /* Initialize function 'ekfRollPitchYaw' input arguments. */
  /* Initialize function input argument 'gyr'. */
  /* Initialize function input argument 'acc'. */
  /* Call the entry-point 'ekfRollPitchYaw'. */
  argInit_3x1_real_T(dv1);
  argInit_3x1_real_T(dv2);
  ekfRollPitchYaw(dv1, dv2, argInit_real_T(), argInit_real_T(), argInit_real_T(),
                  &roll, &pitch, &yaw);
}

/*
 * Arguments    : int argc
 *                const char * const argv[]
 * Return Type  : int
 */
int main(int argc, const char * const argv[])
{
  (void)argc;
  (void)argv;

  /* Initialize the application.
     You do not need to do this more than one time. */
  ekfRollPitchYaw_initialize();

  /* Invoke the entry-point functions.
     You can call entry-point functions multiple times. */
  main_ekfRollPitchYaw();

  /* Terminate the application.
     You do not need to do this more than one time. */
  ekfRollPitchYaw_terminate();
  return 0;
}

/*
 * File trailer for main.c
 *
 * [EOF]
 */
