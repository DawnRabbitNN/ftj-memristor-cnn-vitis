# Reproducibility and Archive Status

## Current status

This repository preserves enough material to explain the system architecture and computation logic, but not enough to rebuild the complete experiment from a clean machine.

## Material currently available

- one curated C workflow derived from the surviving monolithic Vitis application;
- published system equations and architecture descriptions;
- publication and patent metadata;
- documented board type, ADC/DAC components, array size, and experiment dimensions;
- clean host-side refactoring of the mathematical mapping and ADC sign/averaging logic.

## Material not currently available

- complete Vitis workspace and BSP;
- Vivado block design, HDL sources, constraints, and generated hardware platform;
- custom `DacCtrl` and `AdcCtrl` implementation headers/sources;
- PCA9535 board-driver implementation;
- application headers such as `initialize.h`, `img_process.h`, `data.h`, `identifier.h`, and `w_b_.h`;
- complete trained model weights and biases;
- MNIST data converted into the embedded C representation;
- raw MAC, image-processing, and power-analysis data;
- definitive Vitis/Vivado tool version and original build instructions;
- board schematic and calibration records.

## What can be reproduced now

The host-side test under `tests/` verifies:

- 18-bit ADC sign extension;
- signal-window minus baseline-window averaging;
- affine weight-to-conductance conversion;
- recovery of a weighted sum from a conductance-domain dot product.

This test is a mathematical and data-processing check. It does not access the FPGA or FTJ hardware.

## Requirements for a future hardware build

A complete hardware reproduction would require:

1. the original or reconstructed Vivado design for the ZCU104;
2. AXI-connected GPIO, DAC, ADC, interrupt, and clock IP;
3. the custom board schematic and pin mapping;
4. device-safe read/write pulse limits and calibration data;
5. Vitis BSP and driver sources matching the hardware export;
6. a recovered or retrained model with export scripts;
7. documented steps for device programming, conductance verification, and data collection.

## Disclosure rule

If a missing component is reconstructed later, identify it explicitly as one of:

- `original recovered source`;
- `clean refactoring of recovered source`;
- `reimplementation from the publication/patent`;
- `new reference implementation`.

Do not backdate commits or present a later reconstruction as the exact original experimental code.

## Intellectual-property check

Before releasing additional source, weights, schematics, or raw data, confirm approval from the relevant collaborators and the University of Science and Technology of China. Preserve all third-party licence notices.

