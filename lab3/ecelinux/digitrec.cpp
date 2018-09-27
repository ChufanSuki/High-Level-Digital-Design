//==========================================================================
//digitrec.cpp
//==========================================================================
// @brief: A k-nearest-neighbor implementation for digit recognition (k=1)

#include "digitrec.h"

//----------------------------------------------------------
// Top function
//----------------------------------------------------------

void dut(
    hls::stream<bit32_t> &strm_in,
    hls::stream<bit32_t> &strm_out
)
{
  digit input_digit;
  bit32_t input_lo = strm_in.read();
  bit32_t input_hi = strm_in.read();

  input_digit(31, 0)            = input_lo;
  input_digit(DIGIT_SIZE-1, 32) = input_hi;

  bit4_t output = digitrec(input_digit);
  bit32_t out;
  out(output.length()-1, 0) = output(output.length()-1, 0);

  strm_out.write(out);
}

//----------------------------------------------------------
// Digitrec
//----------------------------------------------------------
// @param[in] : input - the testing instance
// @return : the recognized digit (0~9)

bit4_t digitrec( digit input ) 
{
  #include "training_data.h"

  // This array stores K minimum distances per training set
  bit6_t knn_set[10][K_CONST];

  // Initialize the knn set
DIGITREC_INIT_LOOP_OUTER:
  for ( int i = 0; i < 10; ++i )
DIGITREC_INIT_LOOP_INNER:
    for ( int k = 0; k < K_CONST; ++k )
      // Note that the max distance is 49
      knn_set[i][k] = DIGIT_SIZE+1; 

  L1800: for ( int i = 0; i < TRAINING_SIZE; ++i ) {
    L10: for ( int j = 0; j < 10; j++ ) {
      // Read a new instance from the training set
      digit training_instance = training_data[j][i];
      // Update the KNN set
      update_knn( input, training_instance, knn_set[j] );
    }
  } 

  // Compute the final output
  return knn_vote( knn_set ); 
}


//-----------------------------------------------------------------------
// update_knn function
//-----------------------------------------------------------------------
// Given the test instance and a (new) training instance, this
// function maintains/updates an array of K minimum
// distances per training set.

// @param[in] : test_inst - the testing instance
// @param[in] : train_inst - the training instance
// @param[in/out] : min_distances[K_CONST] - the array that stores the current
//                  K_CONST minimum distance values per training set

void update_knn( digit test_inst, digit train_inst, bit6_t min_distances[K_CONST] )
{
  bit6_t dist = 0;
  // Compute the difference using XOR
  digit difference = test_inst ^ train_inst;

  // Count the number of set bits
UPDATE_DIFF_LOOP:
  for ( int i = 0; i < DIGIT_SIZE; ++i ) { 
    dist += difference[i];
  }

UPDATE_DIST_LOOP:
  for(int i = 0; i < K_CONST; ++i)
    if(min_distances[i] > dist) {
      min_distances[i] = dist;
      break;
    }
}

//-----------------------------------------------------------------------
// knn_vote function
//-----------------------------------------------------------------------
// Given 10xK minimum distance values, this function 
// finds the actual K nearest neighbors and determines the
// final output based on the most common digit represented by 
// these nearest neighbors (i.e., a vote among KNNs). 
//
// @param[in] : knn_set - 10xK_CONST min distance values
// @return : the recognized digit
// 

bit4_t knn_vote( bit6_t knn_set[10][K_CONST] )
{
  bit6_t cur_dist[K_CONST];
  bit4_t cur_digit[K_CONST];

  bit4_t vote[10];
  bit4_t max_voted_digit = -1, max_vote = 0;

VOTE_INIT_DIST_LOOP:
  for(int i = 0;i < K_CONST; ++i) {
    cur_dist[i] = DIGIT_SIZE + 1;
    cur_digit[i] = -1;
  }
 
VOTE_INIT_VOTE_LOOP:
  for(int i = 0; i < 10; ++i)
    vote[i] = 0;

VOTE_MIN_DIST_DIGIT_LOOP:
  for(int d = 0; d < 10; ++d)
VOTE_MIN_DIST_CONST_LOOP:
    for(int k = 0; k < K_CONST; ++k) {
VOTE_MIN_DIST_CUR_CONST_LOOP:
      for(int cur_k = 0; cur_k < K_CONST; ++cur_k)
        if(cur_dist[cur_k] > knn_set[d][k]) {
          cur_dist[cur_k] = knn_set[d][k];
          cur_digit[cur_k] = d;
          break;
        }
    }

VOTE_CALC_VOTE_LOOP:
  for(int i = 0; i < K_CONST; ++i)
    vote[cur_digit[i]]++;

VOTE_FIND_MAX_VOTE_LOOP:
  for(int i = 0; i < 10; ++i)
    if(vote[i] > max_vote) {
      max_vote = vote[i];
      max_voted_digit = i;
    }

  return max_voted_digit;
}

