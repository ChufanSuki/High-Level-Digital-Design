//===========================================================================
// bnn.h
//===========================================================================
// @brief: This header file defines the interface for the core functions.

#ifndef BNN
#define BPNN
#include "layer.h"
#include "typedefs.h"
#include <hls_stream.h>

// Top function for synthesis
void dut (
  hls::stream<bit32_t> &strm_in,
  hls::stream<bit32_t> &strm_out
);

// Top function for bnn accelerator
bit32_t bnn_xcel(bit input[MAX_FMAP]);

#endif
