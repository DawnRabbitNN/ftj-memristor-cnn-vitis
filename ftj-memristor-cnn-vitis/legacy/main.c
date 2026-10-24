/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Xilinx shall not be used in
 * advertising or otherwise to promote the sale, use or other dealings in the
 * Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * Curated FTJ-array workflow for portfolio and research documentation.
 *
 * This file condenses the surviving experimental Vitis application into the
 * four operations used by the paper and patent:
 *
 *   1. conductance-voltage multiply-accumulate (MAC) validation;
 *   2. hardware-assisted image convolution;
 *   3. FTJ-assisted first-layer MNIST inference;
 *   4. classification-accuracy reporting.
 *
 * Board drivers, trained parameters and embedded MNIST data were not recovered.
 * They are represented by the ftj_runtime callbacks below rather than replaced
 * with guessed implementations. A future Vitis platform adapter can bind the
 * real DAC, ADC, GPIO, model and data functions to this interface.
 */

#include "board_config.example.h"
#include "ftj_math.h"
#include "model_config.example.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FTJ_KERNEL_ELEMENTS 9U
#define FTJ_EPSILON 1.0e-12

typedef struct {
    size_t count;
    double mean;
    double squared_difference_sum;
} ftj_running_statistics;

/*
 * Integration boundary for the unavailable board, data and model files.
 *
 * measure_conductance_dot_product() must return sum(G[i] * x[i]) after the
 * platform-specific DAC conversion, ADC baseline subtraction and calibration.
 * The core code then applies the affine correction needed for signed weights.
 */
typedef struct {
    void *context;

    size_t mac_trial_count;
    size_t mnist_test_count;

    const double *demo_image;
    double *demo_image_output;
    size_t demo_image_height;
    size_t demo_image_width;

    ftj_affine_mapping image_kernel_mapping;
    ftj_affine_mapping first_conv_mapping;
    double first_conv_bias;

    int (*read_active_conductances)(
        void *context,
        double *conductances,
        size_t count
    );
    int (*load_mac_input)(
        void *context,
        size_t trial_index,
        double *input_voltages,
        size_t count
    );
    int (*program_conductances)(
        void *context,
        const double *conductances,
        size_t count
    );
    int (*measure_conductance_dot_product)(
        void *context,
        const double *inputs,
        size_t count,
        double *result
    );
    int (*select_first_conv_kernel)(void *context);
    int (*load_mnist_sample)(
        void *context,
        size_t sample_index,
        double image[FTJ_MNIST_INPUT_HEIGHT * FTJ_MNIST_INPUT_WIDTH],
        uint8_t *label
    );
    double (*pool_and_normalize_2x2)(
        void *context,
        double top_left,
        double top_right,
        double bottom_left,
        double bottom_right
    );
    int (*classify_remaining_layers)(
        void *context,
        const double first_feature_map[
            FTJ_FIRST_FEATURE_HEIGHT * FTJ_FIRST_FEATURE_WIDTH
        ],
        uint8_t *predicted_label
    );
} ftj_runtime;

static void statistics_add(ftj_running_statistics *statistics, double value)
{
    ++statistics->count;

    const double delta = value - statistics->mean;
    statistics->mean += delta / (double)statistics->count;
    statistics->squared_difference_sum +=
        delta * (value - statistics->mean);
}

static double statistics_standard_deviation(
    const ftj_running_statistics *statistics
)
{
    if (statistics->count < 2U) {
        return 0.0;
    }

    return sqrt(
        statistics->squared_difference_sum /
        (double)(statistics->count - 1U)
    );
}

static double dot_product(
    const double *left,
    const double *right,
    size_t count
)
{
    double result = 0.0;

    for (size_t index = 0U; index < count; ++index) {
        result += left[index] * right[index];
    }

    return result;
}

static double sum_elements(const double *values, size_t count)
{
    double sum = 0.0;

    for (size_t index = 0U; index < count; ++index) {
        sum += values[index];
    }

    return sum;
}

static int validate_runtime(const ftj_runtime *runtime)
{
    return runtime != NULL &&
           runtime->read_active_conductances != NULL &&
           runtime->load_mac_input != NULL &&
           runtime->program_conductances != NULL &&
           runtime->measure_conductance_dot_product != NULL &&
           runtime->select_first_conv_kernel != NULL &&
           runtime->load_mnist_sample != NULL &&
           runtime->pool_and_normalize_2x2 != NULL &&
           runtime->classify_remaining_layers != NULL &&
           runtime->demo_image != NULL &&
           runtime->demo_image_output != NULL &&
           runtime->demo_image_height >= FTJ_FIRST_KERNEL_SIZE &&
           runtime->demo_image_width >= FTJ_FIRST_KERNEL_SIZE &&
           ftj_mapping_is_valid(&runtime->image_kernel_mapping) &&
           ftj_mapping_is_valid(&runtime->first_conv_mapping);
}

static int run_mac_validation(ftj_runtime *runtime)
{
    double conductances[FTJ_KERNEL_ELEMENTS] = {0.0};
    ftj_running_statistics relative_errors = {0U, 0.0, 0.0};

    if (runtime->mac_trial_count == 0U ||
        runtime->read_active_conductances(
            runtime->context,
            conductances,
            FTJ_KERNEL_ELEMENTS
        ) != 0) {
        return -1;
    }

    puts("MAC validation: measured versus expected sum(G[i] * V[i])");

    for (size_t trial = 0U; trial < runtime->mac_trial_count; ++trial) {
        double voltages[FTJ_KERNEL_ELEMENTS] = {0.0};
        double measured = 0.0;

        if (runtime->load_mac_input(
                runtime->context,
                trial,
                voltages,
                FTJ_KERNEL_ELEMENTS
            ) != 0 ||
            runtime->measure_conductance_dot_product(
                runtime->context,
                voltages,
                FTJ_KERNEL_ELEMENTS,
                &measured
            ) != 0) {
            return -1;
        }

        const double expected = dot_product(
            conductances,
            voltages,
            FTJ_KERNEL_ELEMENTS
        );
        if (fabs(expected) > FTJ_EPSILON) {
            const double relative_error = (measured - expected) / expected;

            statistics_add(&relative_errors, relative_error);
            printf(
                "  trial %zu: measured=%g, expected=%g, "
                "relative error=%+.3f%%\n",
                trial + 1U,
                measured,
                expected,
                100.0 * relative_error
            );
        } else {
            printf(
                "  trial %zu: measured=%g, expected=%g, absolute error=%g\n",
                trial + 1U,
                measured,
                expected,
                fabs(measured - expected)
            );
        }
    }

    printf(
        "MAC summary: mean signed error=%+.3f%%, "
        "sample standard deviation=%.3f%%\n\n",
        100.0 * relative_errors.mean,
        100.0 * statistics_standard_deviation(&relative_errors)
    );
    return 0;
}

static int run_image_convolution(ftj_runtime *runtime)
{
    /* Sharpening kernel reported in the publication and surviving source. */
    static const double sharpen_kernel[FTJ_KERNEL_ELEMENTS] = {
         0.0, -1.0,  0.0,
        -1.0,  5.0, -1.0,
         0.0, -1.0,  0.0
    };

    double conductances[FTJ_KERNEL_ELEMENTS] = {0.0};

    for (size_t index = 0U; index < FTJ_KERNEL_ELEMENTS; ++index) {
        conductances[index] = ftj_weight_to_conductance(
            sharpen_kernel[index],
            &runtime->image_kernel_mapping
        );
    }

    if (runtime->program_conductances(
            runtime->context,
            conductances,
            FTJ_KERNEL_ELEMENTS
        ) != 0) {
        return -1;
    }

    const size_t output_height =
        runtime->demo_image_height - FTJ_FIRST_KERNEL_SIZE + 1U;
    const size_t output_width =
        runtime->demo_image_width - FTJ_FIRST_KERNEL_SIZE + 1U;

    for (size_t row = 0U; row < output_height; ++row) {
        for (size_t column = 0U; column < output_width; ++column) {
            double patch[FTJ_KERNEL_ELEMENTS] = {0.0};
            size_t patch_index = 0U;

            for (size_t kernel_row = 0U;
                 kernel_row < FTJ_FIRST_KERNEL_SIZE;
                 ++kernel_row) {
                for (size_t kernel_column = 0U;
                     kernel_column < FTJ_FIRST_KERNEL_SIZE;
                     ++kernel_column) {
                    const size_t image_index =
                        (row + kernel_row) * runtime->demo_image_width +
                        column + kernel_column;
                    patch[patch_index++] = runtime->demo_image[image_index];
                }
            }

            double conductance_domain_sum = 0.0;
            if (runtime->measure_conductance_dot_product(
                    runtime->context,
                    patch,
                    FTJ_KERNEL_ELEMENTS,
                    &conductance_domain_sum
                ) != 0) {
                return -1;
            }

            runtime->demo_image_output[row * output_width + column] =
                ftj_recover_weighted_sum(
                    conductance_domain_sum,
                    sum_elements(patch, FTJ_KERNEL_ELEMENTS),
                    &runtime->image_kernel_mapping
                );
        }
    }

    printf(
        "Image processing: generated a %zux%zu valid-convolution output "
        "with the 3x3 sharpening kernel.\n\n",
        output_height,
        output_width
    );
    return 0;
}

static void build_pooled_padded_image(
    ftj_runtime *runtime,
    const double input[FTJ_MNIST_INPUT_HEIGHT * FTJ_MNIST_INPUT_WIDTH],
    double output[
        FTJ_POOLED_PADDED_HEIGHT * FTJ_POOLED_PADDED_WIDTH
    ]
)
{
    for (size_t index = 0U;
         index < FTJ_POOLED_PADDED_HEIGHT * FTJ_POOLED_PADDED_WIDTH;
         ++index) {
        output[index] = -1.0;
    }

    for (size_t row = 0U; row < FTJ_MNIST_INPUT_HEIGHT / 2U; ++row) {
        for (size_t column = 0U;
             column < FTJ_MNIST_INPUT_WIDTH / 2U;
             ++column) {
            const size_t top_left =
                (2U * row) * FTJ_MNIST_INPUT_WIDTH + 2U * column;
            const size_t output_index =
                (row + 1U) * FTJ_POOLED_PADDED_WIDTH + column + 1U;

            output[output_index] = runtime->pool_and_normalize_2x2(
                runtime->context,
                input[top_left],
                input[top_left + 1U],
                input[top_left + FTJ_MNIST_INPUT_WIDTH],
                input[top_left + FTJ_MNIST_INPUT_WIDTH + 1U]
            );
        }
    }
}

static int measure_first_feature_map(
    ftj_runtime *runtime,
    const double input[
        FTJ_POOLED_PADDED_HEIGHT * FTJ_POOLED_PADDED_WIDTH
    ],
    double output[FTJ_FIRST_FEATURE_HEIGHT * FTJ_FIRST_FEATURE_WIDTH]
)
{
    size_t output_index = 0U;

    for (size_t row = 0U;
         row + FTJ_FIRST_KERNEL_SIZE <= FTJ_POOLED_PADDED_HEIGHT;
         row += FTJ_FIRST_CONV_STRIDE) {
        for (size_t column = 0U;
             column + FTJ_FIRST_KERNEL_SIZE <= FTJ_POOLED_PADDED_WIDTH;
             column += FTJ_FIRST_CONV_STRIDE) {
            double patch[FTJ_KERNEL_ELEMENTS] = {0.0};
            size_t patch_index = 0U;

            for (size_t kernel_row = 0U;
                 kernel_row < FTJ_FIRST_KERNEL_SIZE;
                 ++kernel_row) {
                for (size_t kernel_column = 0U;
                     kernel_column < FTJ_FIRST_KERNEL_SIZE;
                     ++kernel_column) {
                    patch[patch_index++] =
                        input[
                            (row + kernel_row) * FTJ_POOLED_PADDED_WIDTH +
                            column + kernel_column
                        ];
                }
            }

            double conductance_domain_sum = 0.0;
            if (runtime->measure_conductance_dot_product(
                    runtime->context,
                    patch,
                    FTJ_KERNEL_ELEMENTS,
                    &conductance_domain_sum
                ) != 0) {
                return -1;
            }

            output[output_index++] =
                ftj_recover_weighted_sum(
                    conductance_domain_sum,
                    sum_elements(patch, FTJ_KERNEL_ELEMENTS),
                    &runtime->first_conv_mapping
                ) +
                runtime->first_conv_bias;
        }
    }

    return output_index ==
                   FTJ_FIRST_FEATURE_HEIGHT * FTJ_FIRST_FEATURE_WIDTH
               ? 0
               : -1;
}

static int run_digit_recognition(ftj_runtime *runtime)
{
    if (runtime->mnist_test_count == 0U ||
        runtime->select_first_conv_kernel(runtime->context) != 0) {
        return -1;
    }

    size_t correct = 0U;

    for (size_t sample = 0U; sample < runtime->mnist_test_count; ++sample) {
        double image[FTJ_MNIST_INPUT_HEIGHT * FTJ_MNIST_INPUT_WIDTH] = {0.0};
        double pooled[
            FTJ_POOLED_PADDED_HEIGHT * FTJ_POOLED_PADDED_WIDTH
        ] = {0.0};
        double first_feature_map[
            FTJ_FIRST_FEATURE_HEIGHT * FTJ_FIRST_FEATURE_WIDTH
        ] = {0.0};
        uint8_t label = 0U;
        uint8_t prediction = 0U;

        if (runtime->load_mnist_sample(
                runtime->context,
                sample,
                image,
                &label
            ) != 0) {
            return -1;
        }

        build_pooled_padded_image(runtime, image, pooled);

        if (measure_first_feature_map(
                runtime,
                pooled,
                first_feature_map
            ) != 0 ||
            runtime->classify_remaining_layers(
                runtime->context,
                first_feature_map,
                &prediction
            ) != 0) {
            return -1;
        }

        if (prediction == label) {
            ++correct;
        }

        printf(
            "MNIST %zu/%zu: predicted=%u, label=%u, running accuracy=%.2f%%\n",
            sample + 1U,
            runtime->mnist_test_count,
            (unsigned int)prediction,
            (unsigned int)label,
            100.0 * (double)correct / (double)(sample + 1U)
        );
    }

    printf(
        "Recognition summary: %zu/%zu correct (%.2f%%).\n",
        correct,
        runtime->mnist_test_count,
        100.0 * (double)correct / (double)runtime->mnist_test_count
    );
    return 0;
}

int ftj_run_showcase(ftj_runtime *runtime)
{
    if (!validate_runtime(runtime)) {
        fputs("Invalid or incomplete FTJ runtime configuration.\n", stderr);
        return EXIT_FAILURE;
    }

    if (run_mac_validation(runtime) != 0 ||
        run_image_convolution(runtime) != 0 ||
        run_digit_recognition(runtime) != 0) {
        fputs("FTJ workflow stopped after a platform operation failed.\n", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#ifdef FTJ_PLATFORM_BINDING_AVAILABLE
/* Implement this function in the recovered/reconstructed Vitis adapter. */
extern int ftj_platform_bind(ftj_runtime *runtime);

int main(void)
{
    ftj_runtime runtime = {0};

    if (ftj_platform_bind(&runtime) != 0) {
        fputs("Unable to bind the FTJ platform adapter.\n", stderr);
        return EXIT_FAILURE;
    }

    return ftj_run_showcase(&runtime);
}
#else
int main(void)
{
    puts(
        "FTJ workflow source is available, but the original board/model "
        "adapter is not included."
    );
    return EXIT_SUCCESS;
}
#endif
