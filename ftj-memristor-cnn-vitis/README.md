# FTJ Memristor CNN on Zynq/Vitis

Technical documentation and surviving source materials for a hardware-in-the-loop in-memory-computing system built from ferroelectric tunnel junction (FTJ) devices, a custom array-testing board, and a Xilinx Zynq UltraScale+ ZCU104 development board.

> [!IMPORTANT]
> This is a documentation-first research archive, not a turnkey Vitis project. Some original headers, drivers, model parameters, raw data, and generated platform files are no longer available. The repository distinguishes surviving experimental source from explanatory refactoring and reported results.

## What the system demonstrates

The system maps computation onto an FTJ array as follows:

\[
\text{weight } W_{ij} \rightarrow \text{conductance } G_{ij},\qquad
\text{input } x_i \rightarrow \text{word-line voltage } V_i
\]

The accumulated bit-line current performs an analog multiply-accumulate operation:

\[
I_j = \sum_i G_{ij}V_i
\]

The current is converted by a transimpedance amplifier, filtered, digitized by an ADC, and corrected in embedded software. The same data path supports vector dot products, image convolution, and a hardware-assisted convolutional neural network.

```mermaid
flowchart LR
    PC[PC / Vitis] -->|UART| PS[ARM Processing System]
    PS -->|AXI| PL[FPGA Programmable Logic]
    PL --> CTRL[GPIO, DAC and ADC controllers]
    CTRL -->|FMC / board interface| DAC[DAC and WL MUX]
    DAC --> FTJ[FTJ array]
    FTJ --> READ[BL MUX, TIA and LPF]
    READ --> ADC[ADC]
    ADC --> CTRL
```

## Reported results

The values below are reported in the associated publication. Energy-efficiency values are estimates based on the circuit configuration, not direct end-to-end wall-power measurements.

| Demonstration | Reported result |
|---|---:|
| FTJ conductance levels | 32 states (5 bit) |
| Cycle-to-cycle variation | 1.6% |
| 3-element MAC error, standard deviation | 1.22% |
| 6-element MAC error, standard deviation | 1.82% |
| 9-element MAC error, standard deviation | 2.01% |
| Hardware-assisted MNIST accuracy | 92.53% |
| Floating-point reference accuracy | 93.87% |
| Estimated FTJ-array efficiency | 0.692 TOPS/W |

The patent describes a separate 9-element validation data set with an error standard deviation of 2.24%. The two values are kept separate rather than treated as the same experiment.

## Computation demonstrations

### 1. Vector dot product

The conductance of each FTJ represents one vector element. Independent DAC channels apply the second vector as voltages. Devices sharing a bit line sum their currents physically, producing the dot product in one read operation.

### 2. Image convolution

Convolution kernels are mapped to positive conductance values using an affine transformation. Negative kernel weights are recovered through an offset or bias correction after ADC acquisition. The publication demonstrates Roberts edge operators and a sharpening operator. The historical source snapshot also contained representative Sobel and sharpening routines based on the same mapping principle; the curated workflow keeps only the sharpening path.

### 3. Hardware-in-the-loop CNN inference

The demonstrated network follows this path:

```mermaid
flowchart LR
    A[MNIST 28 x 28] --> B[2 x 2 pooling]
    B --> C[15 x 15 feature map]
    C --> D[FTJ 3 x 3 first convolution]
    D --> E[7 x 7 feature map]
    E --> F[Remaining convolution and FC layers on the SoC software path]
    F --> G[10-class output]
```

The FTJ array executes the first convolutional layer. The remaining network operations use the embedded software path and stored model parameters. The original complete parameter files are not included in this archive.

## Repository guide

| Path | Purpose |
|---|---|
| `docs/architecture.md` | Hardware and control-system architecture |
| `docs/computation_mapping.md` | Weight, voltage, current, ADC, and convolution mapping |
| `docs/experiments.md` | Experimental hierarchy and reported results |
| `docs/reproducibility.md` | Available and missing material, and reproduction limits |
| `src/core/` | Self-contained explanatory refactoring of mathematical and ADC-processing logic |
| `config/` | Known board constants and model dimensions, without missing weights |
| `legacy/` | Curated Vitis application workflow and integration notes |
| `references/` | Publication and patent links |
| `tests/` | Host-side checks for the self-contained core logic |

## Source-code status

`legacy/main.c` is a curated refactoring of the surviving monolithic Vitis
application. It retains the research logic relevant to the publication and
patent while removing unrelated manual commands, repeated register sequences,
disabled experiments, and embedded test vectors. The file now presents four
clear stages:

- 9-element conductance-voltage MAC validation and error statistics;
- signed 3 x 3 sharpening through conductance mapping and digital correction;
- FTJ-assisted first-layer MNIST convolution;
- end-to-end prediction and accuracy reporting.

Unavailable board drivers, data loaders, trained weights, and remaining CNN
layers are represented by the documented `ftj_runtime` interface. They are not
silently replaced with fabricated implementations. The default host build only
checks this workflow for valid C syntax; a hardware build requires a separate
Vitis adapter defining `FTJ_PLATFORM_BINDING_AVAILABLE`.

The files in `src/core/` contain self-contained mathematical and ADC-processing
logic used by the curated workflow. Neither file is claimed to reproduce every
line of the historical experiment revision.

## Publication and patent

- Ziming Cheng, He Wang, Zeyu Guan, Zhengxu Zhu, Shengchun Shen, Yuewei Yin, and Xiaoguang Li, **Implementation of Multiply Accumulate Operation and Convolutional Neural Network Based on Ferroelectric Tunnel Junction Memristors**, *ACS Applied Materials & Interfaces* (2025). [DOI: 10.1021/acsami.5c00740](https://doi.org/10.1021/acsami.5c00740)
- **CN119512040A - A neural-network testing system based on an electronic synapse array**, assignee: University of Science and Technology of China. [Google Patents](https://patents.google.com/patent/CN119512040A/en)

## Use and ownership

No repository-wide open-source licence is granted at this stage. The legacy source contains a Xilinx licence notice, and the associated patent is assigned to the University of Science and Technology of China. See [`NOTICE.md`](NOTICE.md) before reusing any material.

