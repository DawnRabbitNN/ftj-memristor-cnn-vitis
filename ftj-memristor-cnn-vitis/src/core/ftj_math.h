#ifndef FTJ_MATH_H
#define FTJ_MATH_H

#include <stddef.h>
#include <stdint.h>

/*
 * Clean explanatory refactoring of equations and ADC-processing patterns
 * documented by the publication, patent, and surviving Vitis source.
 * This is not represented as the original experimental implementation.
 */

typedef struct {
    double weight_min;
    double weight_max;
    double conductance_min;
    double conductance_max;
} ftj_affine_mapping;

int ftj_mapping_is_valid(const ftj_affine_mapping *mapping);

double ftj_weight_to_conductance(
    double weight,
    const ftj_affine_mapping *mapping
);

double ftj_conductance_to_weight(
    double conductance,
    const ftj_affine_mapping *mapping
);

double ftj_recover_weighted_sum(
    double conductance_domain_sum,
    double input_sum,
    const ftj_affine_mapping *mapping
);

int32_t ftj_sign_extend_adc18(uint32_t raw_code);

double ftj_average_baseline_corrected_adc18(
    const uint32_t *samples,
    size_t sample_count,
    size_t signal_offset,
    size_t baseline_offset,
    size_t window_size
);

uint16_t ftj_voltage_to_dac_code(
    double voltage,
    uint16_t zero_code,
    double codes_per_volt,
    uint16_t maximum_code
);

double ftj_dac_code_to_voltage(
    uint16_t code,
    uint16_t zero_code,
    double codes_per_volt
);

#endif

