/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * main.c
 *
 * Code generation for function 'main'
 *
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
/* Include files */
#include "updateEKFQuatAtt.h"
#include "main.h"

/* Function Declarations */
static void argInit_3x1_real_T(double result[3]);
static double argInit_real_T(void);
static void main_updateEKFQuatAtt(void);

/* Function Definitions */
static void argInit_3x1_real_T(double result[3])
{
  double result_tmp;

  /* Loop over the array to initialize each element. */
  /* Set the value of the array element.
     Change this value to the value that the application requires. */
  result_tmp = argInit_real_T();
  result[0] = result_tmp;

  /* Set the value of the array element.
     Change this value to the value that the application requires. */
  result[1] = result_tmp;

  /* Set the value of the array element.
     Change this value to the value that the application requires. */
  result[2] = argInit_real_T();
}

static double argInit_real_T(void)
{
  return 0.0;
}

static void main_updateEKFQuatAtt(void)
{
  double gyr_rps_tmp_tmp[3];
  double roll_deg;
  double pitch_deg;
  double yaw_deg;

  /* Initialize function 'updateEKFQuatAtt' input arguments. */
  /* Initialize function input argument 'gyr_rps'. */
  argInit_3x1_real_T(gyr_rps_tmp_tmp);

  /* Initialize function input argument 'acc_mps2'. */
  /* Initialize function input argument 'mag_unit'. */
  /* Call the entry-point 'updateEKFQuatAtt'. */
  updateEKFQuatAtt(gyr_rps_tmp_tmp, gyr_rps_tmp_tmp, gyr_rps_tmp_tmp,
                   argInit_real_T(), argInit_real_T(), argInit_real_T(),
                   argInit_real_T(), &roll_deg, &pitch_deg, &yaw_deg);
}

int main(int argc, const char * const argv[])
{
  (void)argc;
  (void)argv;

  /* Initialize the application.
     You do not need to do this more than one time. */
  updateEKFQuatAtt_initialize();

  /* Invoke the entry-point functions.
     You can call entry-point functions multiple times. */
  main_updateEKFQuatAtt();

  /* Terminate the application.
     You do not need to do this more than one time. */
  updateEKFQuatAtt_terminate();
  return 0;
}

/* End of code generation (main.c) */
