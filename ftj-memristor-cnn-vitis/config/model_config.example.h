#ifndef FTJ_MODEL_CONFIG_EXAMPLE_H
#define FTJ_MODEL_CONFIG_EXAMPLE_H

/* Network dimensions documented in the publication and surviving source. */

#define FTJ_MNIST_INPUT_HEIGHT          28U
#define FTJ_MNIST_INPUT_WIDTH           28U

#define FTJ_POOLED_PADDED_HEIGHT        15U
#define FTJ_POOLED_PADDED_WIDTH         15U

#define FTJ_FIRST_KERNEL_SIZE           3U
#define FTJ_FIRST_CONV_STRIDE           2U
#define FTJ_FIRST_FEATURE_HEIGHT        7U
#define FTJ_FIRST_FEATURE_WIDTH         7U

#define FTJ_SECOND_FEATURE_MAPS         10U
#define FTJ_SECOND_FEATURE_HEIGHT       3U
#define FTJ_SECOND_FEATURE_WIDTH        3U

#define FTJ_FLATTENED_FEATURES          90U
#define FTJ_FC1_OUTPUTS                 10U
#define FTJ_CLASS_COUNT                 10U

/*
 * Trained weights and biases are deliberately not declared here because the
 * complete original parameter file is not available. Do not insert guessed
 * values. Add a separately documented recovered or retrained model later.
 */

#endif

