//=========================================================================
// cordic.cpp
//=========================================================================
// @brief: A CORDIC implementation of sine and cosine functions.
/* @desc:
 * 1. The code uses radians when measuring angles. 
 * 2. Each angle in iteration is stored in a predefined look-up table
 * 3. The fixed point version of CORDIC does not need DSP48 since all 
 *    MUL/DIV have been eliminated with the use of shift and look-up
 *    tables. 
 * 4. The floating point version of CORDIC was adopted from the demo
 *    program of Parallel Programming of FPGA, Chapter 3
*/

#include "cordic.h"
#include <math.h>

#include <iostream>

//-----------------------------------
// cordic function
//-----------------------------------
// @param[in]  : theta - input angle
// @param[out] : s - sine output
// @param[out] : c - cosine output
void cordic(theta_type theta, cos_sin_type &s, cos_sin_type &c)
{
    index_type step;

#ifdef FIXED_TYPE // fixed-point design

    // Starting coordinates are stored in a look-up table to eliminate
    // the MUL/DIV operations at the end of the CORDIC computation
    cos_sin_type curr_cos = cordic_reciprocal_cuml_scaling[20];
    cos_sin_type curr_sin = 0.0;

FIXED_STEP_LOOP:
    for ( step = 0; step < 20; step++ ) 
    {
      // Determine whether we need a positive or negative angle 
      sigma_type sigma = (theta < 0) ? -1 : 1;

      // Store the original x-coordinate
      cos_sin_type tmp_cos = curr_cos;

      // Finish rotation with only addition, subtraction, and shifting.
      // 2**(-i) is implemented with division and explicit shifting, 
      // which ensures Vivado HLS synthesizes this behavior with shift
      // registers rather than DSP48.
      curr_cos = curr_cos - (curr_sin * sigma) / (1ULL << step);
      curr_sin = (tmp_cos * sigma) / (1ULL << step) + curr_sin;

      // Compute the difference between target and current angle 
      theta = theta - sigma * cordic_ctab[step];
    }

    // Setup results
    s = curr_sin; c = curr_cos;

#else // floating point design

    // Starting coordinates are stored in a look-up table to eliminate
    // the MUL/DIV operations at the end of the CORDIC computation
    cos_sin_type curr_cos = cordic_reciprocal_cuml_scaling[NUM_ITER];
    cos_sin_type curr_sin = 0.0;
    // The iterative shift facotr, beginning at 1.0
    cos_sin_type factor = 1.0;

FLOAT_STEP_LOOP:
    for ( step = 0; step < NUM_ITER; step++ ) 
    {
      // Determine whether we need a positive or negative angle 
      sigma_type sigma = (theta < 0) ? -1 : 1;

      // Store the original x-coordinate
      cos_sin_type tmp_cos = curr_cos;

      // Finish rotation with only addition, subtraction, and shifting
      curr_cos = curr_cos - curr_sin * sigma * factor;
      curr_sin = tmp_cos * sigma * factor + curr_sin;

      // Compute the difference between target and current angle 
      theta = theta - sigma * cordic_ctab[step];

      // Computing the Tan of the next angle with the shift operation
      // This implementation follows the hint from the lab guide
      factor = factor / (double)(1ULL << 1);
    }

    // Setup results
    s = curr_sin; c = curr_cos;

#endif

}
