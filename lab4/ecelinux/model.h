//===========================================================================
// model.h
//===========================================================================
// @brief: This header file include the parameters for BNN

#ifndef MODEL_CONV
#define MODEL_CONV

#include"layer.h"

const bit w_conv1[MAX_W_CONV] = {
    #include"data/weight_0b"
}; //binary weight

const bit w_conv2[MAX_W_CONV] = {
    #include"data/weight_5b"
}; //binary weight

const bit w_fc1[MAX_W_FC] = {
  #include"data/weight_10b"
};

const float b_fc1[FC2_UNITS] = {
  #include"data/weight_11p"
};

const bit w_fc2[FC2_UNITS*OUT] = {
  #include"data/weight_12b"
};

const float b_fc2[OUT] = {
  #include"data/weight_13p"
};

const bit8_t threshold1[MAX_FMAP] = {
  #include"data/batchnorm1"
};

const bit8_t threshold2[MAX_FMAP] = {
  #include"data/batchnorm2"
};

#endif
