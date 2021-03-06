#define GET_FLOAT_WORD(a,b) ((a)=*(int32_t*)&(b))
#define SET_FLOAT_WORD(a,b) ((*(int32_t*)&(a)) = b)

#include <stdint.h>

#define USE_ARMS 0

#ifdef TEST_PROGRAM
#include <stdio.h>
#include <math.h>
#endif

/* ----------------------------------------------------------------------
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.
*
* $Date:        21. September 2015
* $Revision:    V.1.4.5 a
*
* Project:      CMSIS DSP Library
* Title:        arm_sin_f32.c
*
* Description:  Fast sine calculation for floating-point values.
*
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

#ifdef TEST_PROGRAM
#define _sin test_sin
#endif

#define FAST_MATH_TABLE_SIZE  512 /* MUST BE POWER OF TWO TO ALLOW MASKING OPERATIONS */
#define FAST_MATH_TABLE_MASK  (FAST_MATH_TABLE_SIZE - 1)

typedef float float32_t;

// grantham - Has an extra element so interpolation can always get lower and upper table elements
const float32_t sinTable_f32[FAST_MATH_TABLE_SIZE + 1] = {
   0.00000000f, 0.01227154f, 0.02454123f, 0.03680722f, 0.04906767f, 0.06132074f,
   0.07356456f, 0.08579731f, 0.09801714f, 0.11022221f, 0.12241068f, 0.13458071f,
   0.14673047f, 0.15885814f, 0.17096189f, 0.18303989f, 0.19509032f, 0.20711138f,
   0.21910124f, 0.23105811f, 0.24298018f, 0.25486566f, 0.26671276f, 0.27851969f,
   0.29028468f, 0.30200595f, 0.31368174f, 0.32531029f, 0.33688985f, 0.34841868f,
   0.35989504f, 0.37131719f, 0.38268343f, 0.39399204f, 0.40524131f, 0.41642956f,
   0.42755509f, 0.43861624f, 0.44961133f, 0.46053871f, 0.47139674f, 0.48218377f,
   0.49289819f, 0.50353838f, 0.51410274f, 0.52458968f, 0.53499762f, 0.54532499f,
   0.55557023f, 0.56573181f, 0.57580819f, 0.58579786f, 0.59569930f, 0.60551104f,
   0.61523159f, 0.62485949f, 0.63439328f, 0.64383154f, 0.65317284f, 0.66241578f,
   0.67155895f, 0.68060100f, 0.68954054f, 0.69837625f, 0.70710678f, 0.71573083f,
   0.72424708f, 0.73265427f, 0.74095113f, 0.74913639f, 0.75720885f, 0.76516727f,
   0.77301045f, 0.78073723f, 0.78834643f, 0.79583690f, 0.80320753f, 0.81045720f,
   0.81758481f, 0.82458930f, 0.83146961f, 0.83822471f, 0.84485357f, 0.85135519f,
   0.85772861f, 0.86397286f, 0.87008699f, 0.87607009f, 0.88192126f, 0.88763962f,
   0.89322430f, 0.89867447f, 0.90398929f, 0.90916798f, 0.91420976f, 0.91911385f,
   0.92387953f, 0.92850608f, 0.93299280f, 0.93733901f, 0.94154407f, 0.94560733f,
   0.94952818f, 0.95330604f, 0.95694034f, 0.96043052f, 0.96377607f, 0.96697647f,
   0.97003125f, 0.97293995f, 0.97570213f, 0.97831737f, 0.98078528f, 0.98310549f,
   0.98527764f, 0.98730142f, 0.98917651f, 0.99090264f, 0.99247953f, 0.99390697f,
   0.99518473f, 0.99631261f, 0.99729046f, 0.99811811f, 0.99879546f, 0.99932238f,
   0.99969882f, 0.99992470f, 1.00000000f, 0.99992470f, 0.99969882f, 0.99932238f,
   0.99879546f, 0.99811811f, 0.99729046f, 0.99631261f, 0.99518473f, 0.99390697f,
   0.99247953f, 0.99090264f, 0.98917651f, 0.98730142f, 0.98527764f, 0.98310549f,
   0.98078528f, 0.97831737f, 0.97570213f, 0.97293995f, 0.97003125f, 0.96697647f,
   0.96377607f, 0.96043052f, 0.95694034f, 0.95330604f, 0.94952818f, 0.94560733f,
   0.94154407f, 0.93733901f, 0.93299280f, 0.92850608f, 0.92387953f, 0.91911385f,
   0.91420976f, 0.90916798f, 0.90398929f, 0.89867447f, 0.89322430f, 0.88763962f,
   0.88192126f, 0.87607009f, 0.87008699f, 0.86397286f, 0.85772861f, 0.85135519f,
   0.84485357f, 0.83822471f, 0.83146961f, 0.82458930f, 0.81758481f, 0.81045720f,
   0.80320753f, 0.79583690f, 0.78834643f, 0.78073723f, 0.77301045f, 0.76516727f,
   0.75720885f, 0.74913639f, 0.74095113f, 0.73265427f, 0.72424708f, 0.71573083f,
   0.70710678f, 0.69837625f, 0.68954054f, 0.68060100f, 0.67155895f, 0.66241578f,
   0.65317284f, 0.64383154f, 0.63439328f, 0.62485949f, 0.61523159f, 0.60551104f,
   0.59569930f, 0.58579786f, 0.57580819f, 0.56573181f, 0.55557023f, 0.54532499f,
   0.53499762f, 0.52458968f, 0.51410274f, 0.50353838f, 0.49289819f, 0.48218377f,
   0.47139674f, 0.46053871f, 0.44961133f, 0.43861624f, 0.42755509f, 0.41642956f,
   0.40524131f, 0.39399204f, 0.38268343f, 0.37131719f, 0.35989504f, 0.34841868f,
   0.33688985f, 0.32531029f, 0.31368174f, 0.30200595f, 0.29028468f, 0.27851969f,
   0.26671276f, 0.25486566f, 0.24298018f, 0.23105811f, 0.21910124f, 0.20711138f,
   0.19509032f, 0.18303989f, 0.17096189f, 0.15885814f, 0.14673047f, 0.13458071f,
   0.12241068f, 0.11022221f, 0.09801714f, 0.08579731f, 0.07356456f, 0.06132074f,
   0.04906767f, 0.03680722f, 0.02454123f, 0.01227154f, 0.00000000f, -0.01227154f,
   -0.02454123f, -0.03680722f, -0.04906767f, -0.06132074f, -0.07356456f,
   -0.08579731f, -0.09801714f, -0.11022221f, -0.12241068f, -0.13458071f,
   -0.14673047f, -0.15885814f, -0.17096189f, -0.18303989f, -0.19509032f, 
   -0.20711138f, -0.21910124f, -0.23105811f, -0.24298018f, -0.25486566f, 
   -0.26671276f, -0.27851969f, -0.29028468f, -0.30200595f, -0.31368174f, 
   -0.32531029f, -0.33688985f, -0.34841868f, -0.35989504f, -0.37131719f, 
   -0.38268343f, -0.39399204f, -0.40524131f, -0.41642956f, -0.42755509f, 
   -0.43861624f, -0.44961133f, -0.46053871f, -0.47139674f, -0.48218377f, 
   -0.49289819f, -0.50353838f, -0.51410274f, -0.52458968f, -0.53499762f, 
   -0.54532499f, -0.55557023f, -0.56573181f, -0.57580819f, -0.58579786f, 
   -0.59569930f, -0.60551104f, -0.61523159f, -0.62485949f, -0.63439328f, 
   -0.64383154f, -0.65317284f, -0.66241578f, -0.67155895f, -0.68060100f, 
   -0.68954054f, -0.69837625f, -0.70710678f, -0.71573083f, -0.72424708f, 
   -0.73265427f, -0.74095113f, -0.74913639f, -0.75720885f, -0.76516727f, 
   -0.77301045f, -0.78073723f, -0.78834643f, -0.79583690f, -0.80320753f, 
   -0.81045720f, -0.81758481f, -0.82458930f, -0.83146961f, -0.83822471f, 
   -0.84485357f, -0.85135519f, -0.85772861f, -0.86397286f, -0.87008699f, 
   -0.87607009f, -0.88192126f, -0.88763962f, -0.89322430f, -0.89867447f, 
   -0.90398929f, -0.90916798f, -0.91420976f, -0.91911385f, -0.92387953f, 
   -0.92850608f, -0.93299280f, -0.93733901f, -0.94154407f, -0.94560733f, 
   -0.94952818f, -0.95330604f, -0.95694034f, -0.96043052f, -0.96377607f, 
   -0.96697647f, -0.97003125f, -0.97293995f, -0.97570213f, -0.97831737f, 
   -0.98078528f, -0.98310549f, -0.98527764f, -0.98730142f, -0.98917651f, 
   -0.99090264f, -0.99247953f, -0.99390697f, -0.99518473f, -0.99631261f, 
   -0.99729046f, -0.99811811f, -0.99879546f, -0.99932238f, -0.99969882f, 
   -0.99992470f, -1.00000000f, -0.99992470f, -0.99969882f, -0.99932238f, 
   -0.99879546f, -0.99811811f, -0.99729046f, -0.99631261f, -0.99518473f, 
   -0.99390697f, -0.99247953f, -0.99090264f, -0.98917651f, -0.98730142f, 
   -0.98527764f, -0.98310549f, -0.98078528f, -0.97831737f, -0.97570213f, 
   -0.97293995f, -0.97003125f, -0.96697647f, -0.96377607f, -0.96043052f, 
   -0.95694034f, -0.95330604f, -0.94952818f, -0.94560733f, -0.94154407f, 
   -0.93733901f, -0.93299280f, -0.92850608f, -0.92387953f, -0.91911385f, 
   -0.91420976f, -0.90916798f, -0.90398929f, -0.89867447f, -0.89322430f, 
   -0.88763962f, -0.88192126f, -0.87607009f, -0.87008699f, -0.86397286f, 
   -0.85772861f, -0.85135519f, -0.84485357f, -0.83822471f, -0.83146961f, 
   -0.82458930f, -0.81758481f, -0.81045720f, -0.80320753f, -0.79583690f, 
   -0.78834643f, -0.78073723f, -0.77301045f, -0.76516727f, -0.75720885f, 
   -0.74913639f, -0.74095113f, -0.73265427f, -0.72424708f, -0.71573083f, 
   -0.70710678f, -0.69837625f, -0.68954054f, -0.68060100f, -0.67155895f, 
   -0.66241578f, -0.65317284f, -0.64383154f, -0.63439328f, -0.62485949f, 
   -0.61523159f, -0.60551104f, -0.59569930f, -0.58579786f, -0.57580819f, 
   -0.56573181f, -0.55557023f, -0.54532499f, -0.53499762f, -0.52458968f, 
   -0.51410274f, -0.50353838f, -0.49289819f, -0.48218377f, -0.47139674f, 
   -0.46053871f, -0.44961133f, -0.43861624f, -0.42755509f, -0.41642956f, 
   -0.40524131f, -0.39399204f, -0.38268343f, -0.37131719f, -0.35989504f, 
   -0.34841868f, -0.33688985f, -0.32531029f, -0.31368174f, -0.30200595f, 
   -0.29028468f, -0.27851969f, -0.26671276f, -0.25486566f, -0.24298018f, 
   -0.23105811f, -0.21910124f, -0.20711138f, -0.19509032f, -0.18303989f, 
   -0.17096189f, -0.15885814f, -0.14673047f, -0.13458071f, -0.12241068f, 
   -0.11022221f, -0.09801714f, -0.08579731f, -0.07356456f, -0.06132074f, 
   -0.04906767f, -0.03680722f, -0.02454123f, -0.01227154f, -0.00000000f
};

#if USE_ARMS

float32_t _sin(
  float32_t x)
{
  float32_t sinVal, fract, u;                           /* Temporary variables for input, output */
  uint32_t index;                                        /* Index variable */
  float32_t lower, upper;                                        /* Two nearest output values */
  int32_t whole;
  float32_t findex;
  float32_t alpha, beta;

  /* input x is in radians */
  /* Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi */
  u = x * 0.159154943092f;

  /* whole is u truncated (round-towards-zero) */
  whole = (int32_t) u;

  /* Make "whole" be floor of u unless u was a negative whole number in which case "whole" = u - 1 */
  /* really should use floorf here */
  if(x < 0.0f)
  {
    whole--;
  }

  /* Get fractional part of "in" */
  /* result can be 1.0 if "in" is a whole negative number */
  fract = u - (float32_t) whole;

  if (fract >= 1.0f) {
    fract -= 1.0f;
  }

  /* Calculation of index of the table */
  findex = (float32_t) FAST_MATH_TABLE_SIZE * u;

  /* fractional value calculation */
  fract = findex - (float32_t) index;

  // index = ((uint16_t)findex) & FAST_MATH_TABLE_MASK;
  index = ((uint32_t)findex) & FAST_MATH_TABLE_MASK;

  /* Read two nearest values of input value from the sin table */
  lower = sinTable_f32[index];
  upper = sinTable_f32[index+1];

  alpha = 1.0f - fract;
  beta = fract;

  /* Linear interpolation */
  sinVal = alpha * lower + beta * upper;

  /* Return the output value */
  return sinVal;
}

#endif /* USE_ARMS */


/* This ends the ARM copyrighted portion of this file --------------------- */
/* ------------------------------------------------------------------------ */

#if !USE_ARMS

#define floorf our_floorf

inline float our_floorf(float x)
{
    int32_t truncated = (int32_t)x;

    int32_t floored;
    if(x < 0) {
        floored = truncated - 1;
    } else {
        floored = truncated;
    }

    float fraction = x - floored;

    /* special case where x is a whole negative number */
    if(fraction >= 1.0) {
        floored += 1;
    }

    return floored;
}

float _sin(float x)
{
    float u = x * 0.159154943092f;

    float indexf = u * FAST_MATH_TABLE_SIZE;
    int32_t index = (int32_t)floorf(indexf);

    float beta = indexf - index;
    float alpha = 1.0f - beta;

    uint32_t lower = index & FAST_MATH_TABLE_MASK;
    uint32_t upper = lower + 1;

    return sinTable_f32[lower] * alpha + sinTable_f32[upper] * beta;
}

#endif /* !USE_ARMS */


#ifdef TEST_PROGRAM
int main(int argc, char **argv)
{
    float a;

    for(a = -100.0; a < 100.0; a+= .01) {
        float real = sin(a);
        float ours = test_sin(a);
        float error = fabs(real - ours);
        if(error > .0001) {
            printf("%10f: %10f.  %10f <=> %10f\n", a, error, sin(a), test_sin(a));
        }
    }
}
#endif
