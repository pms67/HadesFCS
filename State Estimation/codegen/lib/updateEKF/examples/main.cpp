/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * main.cpp
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
#include "updateEKF.h"
#include "main.h"

/* Function Declarations */
static void argInit_3x1_real_T(double result[3]);
static double argInit_real_T();
static void main_updateEKF();

/* Function Definitions */
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

static double argInit_real_T()
{
  return 0.0;
}

static void main_updateEKF()
{
  double dv3[3];
  double dv4[3];
  double dv5[3];
  double phi;
  double theta;
  double psi;

  /* Initialize function 'updateEKF' input arguments. */
  /* Initialize function input argument 'gyr'. */
  /* Initialize function input argument 'acc'. */
  /* Initialize function input argument 'mag'. */
  /* Call the entry-point 'updateEKF'. */
  argInit_3x1_real_T(dv3);
  argInit_3x1_real_T(dv4);
  argInit_3x1_real_T(dv5);
  updateEKF(dv3, dv4, dv5, argInit_real_T(), argInit_real_T(), &phi, &theta,
            &psi);
}

int main(int, const char * const [])
{
  /* Initialize the application.
     You do not need to do this more than one time. */
  updateEKF_initialize();

  /* Invoke the entry-point functions.
     You can call entry-point functions multiple times. */
  main_updateEKF();

  /* Terminate the application.
     You do not need to do this more than one time. */
  updateEKF_terminate();
  return 0;
}

/* End of code generation (main.cpp) */
