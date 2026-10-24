# Experiments and Reported Results

## Evidence hierarchy

Results in this repository are labelled using the following categories:

- **Measured:** obtained from physical FTJ devices or the custom board.
- **Hardware-assisted:** includes a physical FTJ computation followed by embedded/software processing.
- **Software reference:** conventional floating-point or conductance-state simulation.
- **Estimated:** calculated from component/circuit parameters rather than measured wall power.
- **Projected:** extrapolated to a larger array configuration.

## 1. Device and array characteristics

The publication reports Pt/SiO2/HZO/TiN FTJs with:

- 32 conductance states (5-bit representation);
- cycle-to-cycle variation of 1.6%;
- LTP nonlinearity of 0.80;
- LTD nonlinearity of -0.52;
- approximately linear low-voltage I-V behaviour suitable for MAC reads.

These device results establish the conductance range used by the computing demonstrations. Device fabrication and microscopy are outside the scope of the surviving Vitis source.

## 2. Vector dot products

The article reports the following relative-error standard deviations:

| Vector length | Error standard deviation |
|---:|---:|
| 3 | 1.22% |
| 6 | 1.82% |
| 9 | 2.01% |

The patent presents a separate 9-element, 84-input-vector validation with an error standard deviation of 2.24%. This repository does not merge the patent value with the article data because they may correspond to different calibration or experiment versions.

## 3. Image processing

The published image-processing demonstrations include:

| Operator | Array usage | Purpose |
|---|---|---|
| Roberts (+) | 4 x 1 FTJ array | One diagonal edge direction |
| Roberts (-) | 4 x 1 FTJ array | Orthogonal diagonal edge direction |
| Sharpen | 9 x 1 FTJ array | Image sharpening |

Weights from -5 to +5 were mapped to conductance values from approximately 200 to 460 microSiemens. Hardware output was compared visually with software convolution.

The historical source snapshot contained related Sobel and sharpening code. The curated `main.c` keeps only the sharpening path; it should be described as representative code unless the original Roberts-specific source is recovered.

## 4. MNIST inference

The network structure reported in the article is:

| Stage | Shape |
|---|---|
| Input | 28 x 28 |
| 2 x 2 pooling / padded map | 15 x 15 |
| First 3 x 3 convolution | 1 feature map, 7 x 7 |
| Second convolution | 10 feature maps, 3 x 3 |
| Flatten | 90 values |
| First fully connected layer | 90 x 10 |
| Second fully connected layer | 10 x 10 |
| Output | 10 classes |

The first convolution was executed with a physical 9 x 1 FTJ array. The remaining layers used the embedded computation path and device-informed parameters.

| Method | Reported test accuracy |
|---|---:|
| Floating-point reference | 93.87% |
| 5-bit conductance simulation | 93.35% |
| FTJ first layer + remaining simulated/embedded layers | 92.53% |

The reported confusion matrix uses all 10,000 MNIST test images. The current repository does not include the original model-parameter header or the complete MNIST data header.

## 5. Energy efficiency

For a 9 x 10 matrix-vector multiplication associated with the second convolutional layer, the article estimates:

| Platform | Reported or estimated efficiency |
|---|---:|
| CPU comparison | 0.021 TOPS/W |
| GPU comparison | 0.184 TOPS/W |
| FTJ array configuration | 0.692 TOPS/W |

The article projects 6.21 TOPS/W for a scaled 90 x 10 configuration where one ADC serves more input lines. This value must be labelled **projected**, not measured.

## Figure policy for this archive

The publisher-formatted article PDF is not copied into this repository. Preferred future figure additions are:

1. newly redrawn system and data-flow diagrams;
2. plots regenerated from raw data, if the data is recovered and cleared for release;
3. plots recreated from published numerical values with an explicit citation;
4. original laboratory photographs only after confirming permission to publish them.

