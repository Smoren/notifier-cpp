# LOESS — Locally Weighted Regression in C++

A compact C++20 implementation of **LOESS** (LOcal regrESSion, also known as LOWESS) for
smoothing and predicting one-dimensional time series. The library is built on top of
[Eigen](https://eigen.tuxfamily.org/) and ships with a synthetic time-series generator,
a benchmark harness, and a [Catch2](https://github.com/catchorg/Catch2) test suite.

## Overview

LOESS fits a low-degree polynomial to a local neighbourhood of each target time using
weighted least squares, where the weights come from a tricube kernel. This implementation
supports:

- **Smoothing** — evaluate the regression at every sample of the input series.
- **Prediction** — evaluate the regression at arbitrary new time points (interpolation
  or extrapolation).
- Polynomial degrees **1 (local linear)** and **2 (local quadratic)**.
- Configurable **bandwidth** (fraction of the data used in each local window).
- Batch evaluation of all target points with reused, pre-allocated buffers.

## Features

- Header-only API surface (`src/notifier.h`) with a compiled implementation.
- Synthetic time-series generator with configurable sine + `asinh` trend, intercept,
  and Gaussian noise.
- Benchmark utilities that report mean and maximum execution time over N repetitions.
- Optimized hot path:
  - tricube weights computed in a single pass without temporaries;
  - normal equations assembled from **moments** instead of explicit `XᵀWX` products;
  - small determinants evaluated by cofactor expansion (no LU factorization);
  - all working buffers allocated once per call and reused across target points.

## Project Structure

```
.
├── CMakeLists.txt          # Build configuration (targets: notifier_cpp, notifier_tests)
├── Makefile                # Convenience wrapper around CMake
├── main.cpp                # Entry point (delegates to run_app)
├── src/
│   ├── notifier.h / notifier.cpp # LOESS implementation
│   ├── generator.h / .cpp  # Synthetic time-series generator
│   ├── app.h / app.cpp     # Demo application: generate → smooth → predict → benchmark
│   ├── tools.h / tools.cpp # Wall-clock timestamp helpers
│   └── printer.h           # Pretty-printing helpers for STL containers
├── tests/
│   └── sample_test.cpp     # Catch2 test suite
├── libs/
│   ├── eigen-3.4.0/        # Eigen (bundled)
│   ├── json-3.11.3/        # nlohmann/json (bundled)
│   └── catch2-3.11.0/      # Catch2 amalgamated (bundled)
└── scripts/                # Install / cross-compile / docs helpers
```

## Requirements

- A C++20-capable compiler (tested with GCC).
- CMake ≥ 3.22.
- Make (optional, for the convenience targets).
- Eigen 3.4.0 is bundled under `libs/` and is picked up automatically.

## Building

### With CMake

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target notifier_cpp   -j
cmake --build cmake-build-debug --target notifier_tests -j
```

### With the Makefile

```bash
make init            # configure both Debug and Release build trees
make build-debug     # build the demo executable
make build-release   # build the optimized executable
make build-tests     # build the test executable
```

Other useful targets:

| Target            | Description                                  |
| ----------------- | -------------------------------------------- |
| `make run-debug`  | Run the Debug demo executable                |
| `make run-release`| Run the Release demo executable              |
| `make test`       | Build and run the Catch2 test suite          |
| `make clean`      | Remove the CMake build directories           |

## Usage

```cpp
#include <Eigen/Dense>
#include "src/notifier.h"

// input_data: n x m matrix — column 0 is time, remaining columns are values
Eigen::MatrixXd series = /* ... */;

// bandwidth = 0.3, degree = 2
Loess notifier(0.3, 2);

// Smooth every sample of the series
Eigen::MatrixXd smoothed = notifier.smooth(series);

// Predict at new time points
Eigen::VectorXd new_times(3);
new_times << 0.5, 2.5, 7.5;
Eigen::MatrixXd predicted = notifier.predict(series, new_times);
```

Both methods return an `n x m` matrix whose first column holds the time stamps and whose
remaining columns hold the smoothed/predicted values.

### Generating a synthetic series

```cpp
#include "src/generator.h"

TimeSeriesParams params;
params.t_max = 10.0;
params.step = 0.1;
params.sine_amplitude = 0.3;
params.sine_frequency = 0.5;
params.asinh_amplitude = 0.5;
params.asinh_scale = 2.0;
params.intercept = 0.0;
params.noise_stddev = 0.3;

Eigen::MatrixXd series = generate_time_series(params);
// x(t) = intercept
//      + sine_amplitude  * sin(2*pi*sine_frequency*t + sine_phase)
//      + asinh_amplitude * asinh(asinh_scale * t)
//      + noise
```

The number of rows is `floor(t_max / step) + 1`, with `t = 0, step, 2*step, ...`.

### Running the demo

```bash
./cmake-build-debug/notifier_cpp
```

The demo generates a noisy sine + `asinh` series, prints it, smooths it, predicts at new
time points, and then benchmarks `smooth` and `predict` (100 repetitions each, reporting
mean and maximum time).

## API Reference

### `class Loess`

| Member | Description |
| ------ | ----------- |
| `Loess(double bandwidth = 0.3, int degree = 1)` | Constructs a model. `bandwidth` is clamped to `[0.05, 1.0]`, `degree` to `[1, 2]`. |
| `Eigen::MatrixXd smooth(const Eigen::MatrixXd &input_data) const` | Smooths the input series. |
| `Eigen::MatrixXd predict(const Eigen::MatrixXd &input_data, const Eigen::VectorXd &new_times) const` | Evaluates the model at the given time points. |

### `Eigen::MatrixXd generate_time_series(const TimeSeriesParams &params)`

Generates an `n x 2` matrix with columns `(t, x)` as described above.

### `src/tools.h`

| Function | Description |
| -------- | ----------- |
| `double get_current_timestamp()` | Seconds since the epoch as a `double`. |
| `long get_current_timestamp_micro()` | Microseconds since the epoch. |
| `long get_current_timestamp_milli()` | Milliseconds since the epoch. |

## Testing

```bash
make test
# or
cmake --build cmake-build-debug --target notifier_tests -j && ./cmake-build-debug/notifier_tests
```

## Performance

The hot path was profiled and optimized. On a 101-point series (`degree = 2`,
`bandwidth = 0.3`), measured with GCC `-O2` on the same machine:

| Operation | Baseline | Optimized | Speed-up |
| --------- | -------- | --------- | -------- |
| `smooth`  | ~700 µs  | ~90 µs    | ~7.8×    |
| `predict` | ~663 µs  | ~69 µs    | ~9.6×    |

Optimized results match the baseline to machine precision for interpolation and smoothing
(relative difference ~1e-14); differences only appear in the last few significant digits
during extrapolation (~1e-10 relative), caused by reordering of floating-point operations.

## Notes

- Input data is assumed to be a two-or-more-column matrix where **column 0 is time**.
- `smooth`/`predict` return an empty `0 x m` matrix when there are fewer rows than
  `degree + 1`.
- The bundled `libs/json-3.11.3` and `libs/catch2-3.11.0` are used for JSON handling and
  tests respectively. The `CMakeLists.txt` also declares (but does not currently link)
  optional curl/OpenCV paths used by the broader toolchain.

## License

Repository is licensed under the MIT License.
