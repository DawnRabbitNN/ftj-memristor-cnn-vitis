#include "ftj_math.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static int nearly_equal(double left, double right, double tolerance)
{
    return fabs(left - right) <= tolerance;
}

int main(void)
{
    const ftj_affine_mapping mapping = {
        .weight_min = -5.0,
        .weight_max = 5.0,
        .conductance_min = 200.0,
        .conductance_max = 460.0,
    };

    assert(ftj_mapping_is_valid(&mapping));
    assert(nearly_equal(ftj_weight_to_conductance(-5.0, &mapping), 200.0, 1e-12));
    assert(nearly_equal(ftj_weight_to_conductance(0.0, &mapping), 330.0, 1e-12));
    assert(nearly_equal(ftj_weight_to_conductance(5.0, &mapping), 460.0, 1e-12));

    const double weights[] = {-5.0, 0.0, 5.0};
    const double inputs[] = {0.25, -0.5, 1.0};
    double conductance_sum = 0.0;
    double input_sum = 0.0;
    double expected_weighted_sum = 0.0;

    for (size_t index = 0; index < 3U; ++index) {
        const double conductance =
            ftj_weight_to_conductance(weights[index], &mapping);
        conductance_sum += conductance * inputs[index];
        input_sum += inputs[index];
        expected_weighted_sum += weights[index] * inputs[index];
    }

    const double recovered =
        ftj_recover_weighted_sum(conductance_sum, input_sum, &mapping);
    assert(nearly_equal(recovered, expected_weighted_sum, 1e-12));

    assert(ftj_sign_extend_adc18(0x00000U) == 0);
    assert(ftj_sign_extend_adc18(0x1FFFFU) == 131071);
    assert(ftj_sign_extend_adc18(0x20000U) == -131072);
    assert(ftj_sign_extend_adc18(0x3FFFFU) == -1);

    const uint32_t samples[] = {10U, 20U, 30U, 1U, 2U, 3U};
    const double corrected = ftj_average_baseline_corrected_adc18(
        samples,
        6U,
        0U,
        3U,
        3U
    );
    assert(nearly_equal(corrected, 18.0, 1e-12));

    assert(ftj_voltage_to_dac_code(0.0, 8192U, 819.2, 16383U) == 8192U);
    assert(ftj_voltage_to_dac_code(0.1, 8192U, 819.2, 16383U) == 8274U);
    assert(nearly_equal(
        ftj_dac_code_to_voltage(8274U, 8192U, 819.2),
        82.0 / 819.2,
        1e-12
    ));

    puts("All ftj_math tests passed.");
    return 0;
}

