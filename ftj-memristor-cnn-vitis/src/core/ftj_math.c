#include "ftj_math.h"

#include <math.h>

int ftj_mapping_is_valid(const ftj_affine_mapping *mapping)
{
    return mapping != NULL &&
           mapping->weight_max > mapping->weight_min &&
           mapping->conductance_max > mapping->conductance_min;
}

double ftj_weight_to_conductance(
    double weight,
    const ftj_affine_mapping *mapping
)
{
    if (!ftj_mapping_is_valid(mapping)) {
        return NAN;
    }

    const double normalized =
        (weight - mapping->weight_min) /
        (mapping->weight_max - mapping->weight_min);

    return mapping->conductance_min +
           normalized *
               (mapping->conductance_max - mapping->conductance_min);
}

double ftj_conductance_to_weight(
    double conductance,
    const ftj_affine_mapping *mapping
)
{
    if (!ftj_mapping_is_valid(mapping)) {
        return NAN;
    }

    const double normalized =
        (conductance - mapping->conductance_min) /
        (mapping->conductance_max - mapping->conductance_min);

    return mapping->weight_min +
           normalized * (mapping->weight_max - mapping->weight_min);
}

double ftj_recover_weighted_sum(
    double conductance_domain_sum,
    double input_sum,
    const ftj_affine_mapping *mapping
)
{
    if (!ftj_mapping_is_valid(mapping)) {
        return NAN;
    }

    /* W = aG + b, so sum(W_i x_i) = a sum(G_i x_i) + b sum(x_i). */
    const double a =
        (mapping->weight_max - mapping->weight_min) /
        (mapping->conductance_max - mapping->conductance_min);
    const double b = mapping->weight_min - a * mapping->conductance_min;

    return a * conductance_domain_sum + b * input_sum;
}

int32_t ftj_sign_extend_adc18(uint32_t raw_code)
{
    const uint32_t mask = (1U << 18U) - 1U;
    const uint32_t sign_bit = 1U << 17U;
    raw_code &= mask;

    if ((raw_code & sign_bit) != 0U) {
        return (int32_t)(raw_code | ~mask);
    }

    return (int32_t)raw_code;
}

double ftj_average_baseline_corrected_adc18(
    const uint32_t *samples,
    size_t sample_count,
    size_t signal_offset,
    size_t baseline_offset,
    size_t window_size
)
{
    if (samples == NULL || window_size == 0U ||
        signal_offset + window_size > sample_count ||
        baseline_offset + window_size > sample_count) {
        return NAN;
    }

    double difference_sum = 0.0;

    for (size_t index = 0; index < window_size; ++index) {
        const int32_t signal =
            ftj_sign_extend_adc18(samples[signal_offset + index]);
        const int32_t baseline =
            ftj_sign_extend_adc18(samples[baseline_offset + index]);
        difference_sum += (double)signal - (double)baseline;
    }

    return difference_sum / (double)window_size;
}

uint16_t ftj_voltage_to_dac_code(
    double voltage,
    uint16_t zero_code,
    double codes_per_volt,
    uint16_t maximum_code
)
{
    if (!(codes_per_volt > 0.0)) {
        return zero_code;
    }

    long code = lround((double)zero_code + voltage * codes_per_volt);

    if (code < 0L) {
        code = 0L;
    }
    if (code > (long)maximum_code) {
        code = (long)maximum_code;
    }

    return (uint16_t)code;
}

double ftj_dac_code_to_voltage(
    uint16_t code,
    uint16_t zero_code,
    double codes_per_volt
)
{
    if (!(codes_per_volt > 0.0)) {
        return NAN;
    }

    return ((double)code - (double)zero_code) / codes_per_volt;
}

