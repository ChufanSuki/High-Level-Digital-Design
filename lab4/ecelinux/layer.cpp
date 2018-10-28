//==========================================================================
// layer.cpp
//==========================================================================
// @brief: this file contains all layers

#include "layer.h"
#include "model.h"
#include <cmath>
#include <iostream>

using namespace std;

// helper function to neglect padding pixels
inline bool if_mac(int x, int y, int I)
{
        if (x < PADDING / 2 || x >= (I - PADDING / 2) || y < PADDING / 2 || y >= (I - PADDING / 2))
                return false;
        return true;
}

//----------------------------------------------------------
// Padding
//----------------------------------------------------------
// @param[in] : input - input fmaps
//              M - number of input fmaps
//              I - width of input fmaps
// @param[out] : output - output fmaps

void pad(bit input[MAX_FMAP], bit output[MAX_FMAP], int M, int I) {
  int ifmap_size = I * I;
  int ofmap_size = (I+PADDING) * (I+PADDING);

  for (int i = 0; i < MAX_FMAP; i++) output[i] = 0;

  for (int m = 0; m < M; m++) {
    for (int x = 0; x < I; x++) {
      for (int y = 0; y < I; y++) {
        int i_index = x + y*I + m*ifmap_size;
        int o_index = (x + PADDING/2) + (y + PADDING/2)*(I + PADDING) + m*ofmap_size;
        output[o_index] = input[i_index];
      }
    }
  }
}

//----------------------------------------------------------
// Perform Convolution Layer
//----------------------------------------------------------
// @param[in] : input - input fmaps
//              threshold - threshold for batchnorm operation
//              M - number of input fmaps
//              N - number of output fmaps
//              I - width of input fmaps
//              L - id for conv layers: 0 means conv1, 1 means conv2
// @param[out] : output - output fmaps

void conv(bit input[MAX_FMAP], bit output[MAX_FMAP], const bit8_t threshold[MAX_FMAP], int M, int N, int I, int L)
{
  int O = I - F + 1;
  int ifmap_size = I * I;
  int ofmap_size = O * O;
  
  // MAC and batchnorm
  LOOP_N: for (int n = 0; n < N; n++){
    LOOP_X: for (int x = 0; x < O; x++){
      LOOP_Y: for (int y = 0; y < O; y++){
        int sum = 0;
        int o_index = x + y * O + n * ofmap_size;
        LOOP_M: for (int m = 0; m < M; m++){
          int one_out = 0;
          int mac_num = 0;
          LOOP_C: for (int c = 0; c < F; c++){
            LOOP_R: for (int r = 0; r < F; r++){
              if (if_mac(x + c, y + r, I)) { //neglect padding pixels in mac
                int i_index = x + c + (y + r) * I + m * ifmap_size;
                int w_index = c + r * F + (n + m * N) * FILTER_SIZE;
                if (L == 0) one_out += input[i_index] == w_conv1[w_index]; //XNOR
                else        one_out += input[i_index] == w_conv2[w_index];
                mac_num++;
              }
            }
          }
          sum += (one_out << 1) - mac_num;
        }
        output[o_index] = sum > threshold[o_index] ? 1 : 0;
      }
    }
  }
}

//----------------------------------------------------------
// Max pooling
//----------------------------------------------------------
// @param[in] : input - input fmaps
//              M - number of input fmaps
//              I - width of input fmaps
// @param[out] : output - output fmaps

void max_pool(bit input[MAX_FMAP], bit output[MAX_FMAP], int M, int I){
  int O = I / 2;
  int ifmap_size = I * I;
  int ofmap_size = O * O;

  for (int i = 0; i < MAX_FMAP; i++) output[i] = 0;

  for (int m = 0; m < M; m++){
    for (int x = 0; x < O; x++){
      for (int y = 0; y < O; y++){
        int o_index = x + y * O + m * ofmap_size;
        bit max = 0;
        for (int c = 0; c < 2; c++){
          for (int r = 0; r < 2; r++){
            int i_index = 2 * x + c + (2 * y + r) * I + m * ifmap_size;
            if (input[i_index]) max = 1; //this is because bit 1 is represented as 0xff memory
          }
        }
        output[o_index] = max;
      }
    }
  }
}

//----------------------------------------------------------
// Reshpae the Output from Conv Layer
//----------------------------------------------------------
// @param[in] : input - output fmaps from the last conv layer
// @param[out] : output - input famps of the first dense layer

void reshape(bit* input, bit* output) {
  for (int c = 0; c < N_CHANNEL2; c++) {
    for (int y = 0; y < O_WIDTH; y++) {
      for (int x = 0; x < O_WIDTH; x++) {
        int o_index = c + (x + y * O_WIDTH ) * N_CHANNEL2;
        int i_index = x + y * O_WIDTH + c * O_WIDTH*O_WIDTH;
        output[o_index] = input[i_index];
      }
    }
  }
}

//----------------------------------------------------------
// Perform Dense Layer
//----------------------------------------------------------
// @param[in] : input - input fmaps
//              weight - weights
//              bias - biases
//              M - number of input fmaps
//              N - number of output fmaps
//              use_relu - enable relu or not
// @param[out] : output - output fmaps

void dense(bit input[MAX_FMAP], bit output[MAX_FMAP], const bit* weight, const float* bias, int M, int N, bool use_relu){
  float var_w = 2. / M;
  float c = sqrt(var_w);
  float max = -100;

  for (int n = 0; n < N; n++){
    float one_out = 0;
    for (int m = 0; m < M; m++) {
      int w_index = m * N + n;
      one_out += input[m] == weight[w_index]; //XNOR
    }
    one_out = (2 * one_out - M)*c;
    float biased = one_out + bias[n];
    if (use_relu) output[n] = (biased > 0) ? 1 : 0;
    else { // last layer
      if (biased > max) {
        max = biased;
        output[n] = 1;
      } else {
        output[n] = 0;
      }
    }
  }
}
