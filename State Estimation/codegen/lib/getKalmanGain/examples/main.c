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
#include "getKalmanGain.h"
#include "main.h"

/* Function Declarations */
static void argInit_3x4_real_T(double result[12]);
static void argInit_4x4_real_T(double result[16]);
static double argInit_real_T(void);
static void main_getKalmanGain(void);

/* Function Definitions */
static void argInit_3x4_real_T(double result[12])
{
  int idx0;
  double result_tmp;

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < 3; idx0++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result_tmp = argInit_real_T();
    result[idx0] = result_tmp;

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 3] = result_tmp;

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 6] = argInit_real_T();

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 9] = argInit_real_T();
  }
}

static void argInit_4x4_real_T(double result[16])
{
  int idx0;
  double result_tmp;

  /* Loop over the array to initialize each element. */
  for (idx0 = 0; idx0 < 4; idx0++) {
    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result_tmp = argInit_real_T();
    result[idx0] = result_tmp;

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 4] = result_tmp;

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 8] = argInit_real_T();

    /* Set the value of the array element.
       Change this value to the value that the application requires. */
    result[idx0 + 12] = argInit_real_T();
  }
}

static double argInit_real_T(void)
{
  return 0.0;
}

static void main_getKalmanGain(void)
{
  double dv0[16];
  double dv1[12];
  double K[12];

  /* Initialize function 'getKalmanGain' input arguments. */
  /* Initialize function input argument 'P'. */
  /* Initialize function input argument 'C'. */
  /* Call the entry-point 'getKalmanGain'. */
  argInit_4x4_real_T(dv0);
  argInit_3x4_real_T(dv1);
  getKalmanGain(dv0, dv1, argInit_real_T(), K);
}

int main(int argc, const char * const argv[])
{
  (void)argc;
  (void)argv;

  /* Initialize the application.
     You do not need to do this more than one time. */
  getKalmanGain_initialize();

  /* Invoke the entry-point functions.
     You can call entry-point functions multiple times. */
  main_getKalmanGain();

  /* Terminate the application.
     You do not need to do this more than one time. */
  getKalmanGain_terminate();
  return 0;
}

/* End of code generation (main.c) */
