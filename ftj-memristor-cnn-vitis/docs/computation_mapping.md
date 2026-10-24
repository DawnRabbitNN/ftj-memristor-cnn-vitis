# Computation Mapping

## 1. From weights to conductance

An FTJ stores a positive conductance, while neural-network weights or image kernels may be negative. A general affine mapping between a weight range $[W_{min}, W_{max}]$ and conductance range $[G_{min}, G_{max}]$ is:

$$
W = aG + b
$$

where

$$
a = \frac{W_{max}-W_{min}}{G_{max}-G_{min}}
$$

and

$$
b = W_{min}-aG_{min}
$$

For an input vector $x_i$, the desired weighted sum is therefore:

$$
\sum_i W_i x_i
=a\sum_iG_i x_i+b\sum_i x_i
$$

The first term comes from the measured array current. The second term is an offset correction that can be implemented digitally or with an additional bias column.

For a symmetric weight interval $[-W_{max},W_{max}]$, the patent describes a bias-column formulation. A bias column with conductance $G_{min}+G_{max}$ permits the corrected ADC output to be formed as:

$$
V'_{ADC}=2V_{ADC}-V_{ADC,bias}
$$

followed by scaling with $W_{max}/(G_{max}-G_{min})$.

## 2. From inputs to DAC voltages

The surviving program uses a midpoint DAC code of 8192 and a legacy conversion factor of approximately 819.2 codes per volt:

$$
DAC(x)=8192+819.2V
$$

For MAC validation, the publication reports word-line input voltages in the range -0.1 V to +0.1 V. The code should treat the conversion factor as a board calibration constant rather than a universal device property.

## 3. Parallel MAC operation

For one bit line:

```text
conductance vector G[0:m]
input voltage vector V[0:m]
            |
            v
physical current sum I = sum(G[i] * V[i])
            |
            v
TIA and low-pass filter
            |
            v
ADC samples
            |
            v
baseline subtraction + averaging + scale correction
```

The original `start1msVMM` command performs three useful operations:

1. reads individual device conductances;
2. applies sets of random voltage vectors;
3. prints measured and expected dot products for comparison.

The raw program repeats the same register and acquisition sequence for each channel. A maintainable implementation should expose channel-indexed helpers instead.

## 4. ADC conversion and baseline correction

The experimental program repeatedly takes an acquisition window, converts raw 18-bit ADC codes to signed values, subtracts a later baseline window, and averages the difference.

A safe software implementation should:

1. mask the raw code to 18 bits;
2. explicitly sign-extend bit 17;
3. average the configured signal window;
4. average or pair it with a baseline window;
5. subtract the baseline before converting to current.

The self-contained implementation in `src/core/ftj_math.c` demonstrates this logic without depending on unavailable Xilinx drivers.

## 5. Image convolution

For each image window:

1. flatten the pixel patch into a voltage vector;
2. program or select devices whose conductances represent the kernel;
3. apply the voltages in parallel;
4. read the accumulated current;
5. recover the signed convolution value using the affine or bias correction;
6. move the window and repeat.

The publication demonstrates two 2 x 2 Roberts operators and one 3 x 3 sharpening kernel. The surviving source additionally contains a Sobel-right experiment and a sharpening routine named `Laplacian`/`arrayLaplacian`.

The latter name is historically convenient but mathematically imprecise: the kernel

```text
 0 -1  0
-1  5 -1
 0 -1  0
```

is a sharpening kernel obtained by combining an identity term with a Laplacian term.

## 6. Hardware-assisted CNN

The demonstrated inference path is:

1. load a 28 x 28 MNIST sample;
2. apply 2 x 2 pooling;
3. store the result in a padded 15 x 15 feature map;
4. slide a 3 x 3 window with stride 2;
5. encode the nine pixels as nine DAC voltages;
6. use a 9 x 1 FTJ array for the first convolution;
7. scale the ADC result and add the learned bias;
8. execute the remaining convolution and fully connected layers on the embedded software path;
9. choose the largest of ten output values as the predicted digit.

Because the complete weight header is unavailable, this repository documents the dimensions and interface but does not provide substitute weights or claim end-to-end reproducibility.

