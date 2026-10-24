#ifndef FTJ_BOARD_CONFIG_EXAMPLE_H
#define FTJ_BOARD_CONFIG_EXAMPLE_H

/*
 * Documented or inferred from the surviving source snapshot.
 * Verify every value against the actual board calibration before hardware use.
 */

#define FTJ_MAX_WORD_LINES              10U
#define FTJ_MAX_BIT_LINES               10U

/* Legacy DAC conversion used by the Vitis program. */
#define FTJ_DAC_ZERO_CODE               8192U
#define FTJ_DAC_MAX_CODE                16383U
#define FTJ_DAC_CODES_PER_VOLT          819.2

/* MAC validation used an approximately +/-0.1 V read/input range. */
#define FTJ_DEFAULT_READ_VOLTAGE_V      0.1

/* Repeated acquisition windows in the legacy program. */
#define FTJ_ADC_SIGNAL_OFFSET           (27U * 16U)
#define FTJ_ADC_BASELINE_OFFSET         (57U * 16U)
#define FTJ_ADC_AVERAGE_COUNT           (5U * 16U)

/* Example pulse-register values from the legacy source. */
#define FTJ_PULSE_WIDTH_1US_REGISTER    26U
#define FTJ_PULSE_WIDTH_1MS_REGISTER    47597U

#endif

