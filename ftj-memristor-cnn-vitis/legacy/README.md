# Curated Vitis Application Workflow

`main.c` is a readability-focused refactoring of the surviving experimental
application. It is intentionally kept separate from the platform-independent
mathematical helpers in `src/core/`.

## Important limitations

The original application was not a standalone build. Its unavailable
project-specific or platform-generated dependencies included:

- `initialize.h`
- `img_process.h`
- `data.h`
- `identifier.h`
- `w_b_.h`
- `platform.h`
- `dac_ctrl.h`
- `pca9535config.h`
- `adc_ctrl.h`

Some of these files contained model weights, image data, board-control
definitions, or functions for the remaining CNN layers. The curated file does
not invent replacements: `ftj_runtime` exposes the required operations as
callbacks for a future recovered or reconstructed platform adapter.

## Code map

| Function | Purpose |
|---|---|
| `run_mac_validation` | Read nine conductances, apply voltage vectors, and report mean/standard-deviation error |
| `run_image_convolution` | Map the signed sharpening kernel to conductance and scan image patches |
| `build_pooled_padded_image` | Preserve the documented 28 x 28 to padded 15 x 15 preprocessing flow |
| `measure_first_feature_map` | Execute the 3 x 3, stride-2 FTJ first convolution and add its bias |
| `run_digit_recognition` | Run the remaining model through an adapter and report running/final accuracy |
| `ftj_run_showcase` | Execute the three research demonstrations in order |

Compile-time syntax checking is available with `make check-main`. The default
`main` reports that the platform adapter is absent. A real Vitis build must bind
the callbacks and define `FTJ_PLATFORM_BINDING_AVAILABLE`.

## Licence notice

The file retains the original Xilinx copyright and permission notice that
limits use to applications running on or interacting with Xilinx devices.
Preserve that header in all copies or derived distributions where required.

The curated workflow is not represented as the exact source revision that
generated every published figure.

