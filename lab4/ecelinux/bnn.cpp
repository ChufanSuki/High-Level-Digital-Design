//==========================================================================
// bnn.cpp
//==========================================================================
// @brief: A convolution kernel for CNN on digit recognition

#include "layer.h"
#include "model.h"
#include "bnn.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

//----------------------------------------------------------
// Top function
//----------------------------------------------------------

void dut(
    hls::stream<bit32_t> &strm_in,
    hls::stream<bit32_t> &strm_out
)
{
  bit input[MAX_FMAP];
  bit32_t input_l;
  bit32_t output;

  //read one test image into digit
  for (int i = 0; i < I_WIDTH1 * I_WIDTH1 / BUS_WIDTH; i++) {
     input_l = strm_in.read();
     for (int j = 0; j < BUS_WIDTH; j++) {
       input[i*BUS_WIDTH+j] = input_l(j,j);
     }
  }
  //call bnn
  output = bnn_xcel(input);
 
  // write out the result
  strm_out.write(output);
}

//----------------------------------------------------------
// BNN Accelerator
//----------------------------------------------------------
// @param[in] : input - the testing instance
// @return : the predicted digit

bit32_t bnn_xcel(bit input[MAX_FMAP]){
  #pragma HLS DATAFLOW
  bit mem_pad1[MAX_FMAP];
  bit mem_conv1[MAX_FMAP];
  bit mem_pool1[MAX_FMAP];
  bit mem_pad2[MAX_FMAP];
  bit mem_conv2[MAX_FMAP];
  bit mem_pool2[MAX_FMAP];
  bit mem_reshape[MAX_FMAP];
  bit mem_dense1[MAX_FMAP];
  bit mem_dense2[MAX_FMAP];
  /*
  #pragma HLS array_partition variable=mem_pad1 -block factor=18
  #pragma HLS array_partition variable=mem_conv1 -block factor=3
  #pragma HLS array_partition variable=mem_pad2 -block factor=10
  #pragma HLS array_partition variable=mem_conv2 -block factor=3
  */

  /* First Conv Layer */
  /*
  pad(input, mem_conv1_pad, 1, I_WIDTH1);
  // conv(mem_conv1, mem_conv2, threshold1, 1, N_CHANNEL1, I_WIDTH1+PADDING, 0);
  conv_first(mem_conv1_pad, mem_conv2, threshold1, N_CHANNEL1, I_WIDTH1+PADDING);
  max_pool(mem_conv2, mem_conv1, N_CHANNEL1, I_WIDTH1);
  */
  pad(input, mem_pad1, 1, I_WIDTH1);
  conv_first(mem_pad1, mem_conv1, threshold1, N_CHANNEL1, I_WIDTH1+PADDING);
  max_pool(mem_conv1, mem_pool1, N_CHANNEL1, I_WIDTH1);

  /* Second Conv Layer */
  /*
  pad(mem_conv1, mem_conv2, N_CHANNEL1, I_WIDTH2);
  // conv(mem_conv2, mem_conv1, threshold2, N_CHANNEL1, N_CHANNEL2, I_WIDTH2+PADDING, 1);
  conv_second(mem_conv2, mem_conv1, threshold2, N_CHANNEL2, I_WIDTH2+PADDING);
  max_pool(mem_conv1, mem_conv2, N_CHANNEL2, I_WIDTH2);
  reshape(mem_conv2, mem_conv1);
  */
  pad(mem_pool1, mem_pad2, N_CHANNEL1, I_WIDTH2);
  conv_second(mem_pad2, mem_conv2, threshold2, N_CHANNEL2, I_WIDTH2+PADDING);
  max_pool(mem_conv2, mem_pool2, N_CHANNEL2, I_WIDTH2);
  reshape(mem_pool2, mem_reshape);

  /* Dense Layers */
  /*
  dense(mem_conv1, mem_conv2, w_fc1, b_fc1, FC1_UNITS, FC2_UNITS, true);
  dense(mem_conv2, mem_conv1, w_fc2, b_fc2, FC2_UNITS, 10, false);
  */
  dense(mem_reshape, mem_dense1, w_fc1, b_fc1, FC1_UNITS, FC2_UNITS, true);
  dense(mem_dense1, mem_dense2, w_fc2, b_fc2, FC2_UNITS, 10, false);
  
  // find predicted digit 
  bit32_t max_id = 0;
  for(int i = 1; i < 10; i++)
    if(mem_dense2[i])
      max_id = i;
  return max_id;
}
