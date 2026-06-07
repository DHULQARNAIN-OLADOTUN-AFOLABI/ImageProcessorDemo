# Parallel Image Processor
### CSC 426 — Parallel Programming | Group 1 | University of Ilorin

A high-performance image processing desktop application built in C++ using
OpenMP for parallel processing. The application demonstrates the performance
benefits of parallel computing by applying 15 different image filters to any
image, measuring and comparing serial vs parallel execution times, speedup,
and efficiency in real time.

---

## Features

- 15 image filters with real-time performance comparison
- Serial vs parallel execution timing for every filter
- Adjustable thread count (1–64) to demonstrate Amdahl's Law and diminishing returns
- Intensity controls for blur, sharpen, emboss and pixelate filters
- Total summary showing overall speedup and efficiency across all selected filters
- Native Windows GUI — no browser, no Python, no external runtime needed
- Supports any JPG, PNG, or BMP image
- Output images saved automatically to an `output` folder next to your image

---

## Requirements

| Tool | Purpose |
|---|---|
| MSYS2 + MinGW-w64 GCC | C++ compiler with OpenMP support |
| Windows 10 or 11 | Target platform |
| stb_image.h | Image loading (included in repo) |
| stb_image_write.h | Image saving (included in repo) |

---

## Installation & Setup

### 1. Install MSYS2

Download and install MSYS2 from https://www.msys2.org

### 2. Install GCC with OpenMP

Open **MSYS2 MINGW64** from the Start menu and run:

```bash
pacman -Syu
pacman -Su
pacman -S mingw-w64-x86_64-gcc
```

### 3. Add to PATH

Add the following to your Windows system PATH environment variable:
`C:\msys64\mingw64\bin`

### 4. Clone the repository

```bash
git clone https://github.com/yourusername/parallel-image-processor.git
cd parallel-image-processor
```

### 5. Compile

Open Command Prompt in the project folder and run:

```bash
g++ -O2 -fopenmp -mwindows -o ImageProcessing.exe ImageProcessing.cpp -lcomdlg32 -lshell32
```

### 6. Run

Double-click `ImageProcessing.exe` — no installation required.

---

## How to Use

1. Launch `ImageProcessing.exe`
2. Click **Browse** and select any JPG, PNG, or BMP image from your computer
3. Enter the number of threads to use (try 1, 2, 4, 8, 16 to see diminishing returns)
4. Select the filters you want to apply, or leave **ALL FILTERS** checked
5. For filters with intensity selectors, choose your preferred strength from the dropdown
6. Click **▶ Run Filters**
7. Watch the results panel — each filter shows serial time, parallel time, speedup and efficiency
8. A total summary appears at the end showing overall performance across all filters
9. Click **Open Output Folder** to view the processed images

---

## Filters

### Pixel-wise Filters
These filters operate on each pixel independently with no knowledge of neighbouring pixels.

| Filter | Description |
|---|---|
| **Grayscale** | Converts the image to black and white using the luminance formula |
| **Brightness** | Increases the intensity of every pixel by a fixed amount |
| **Sepia** | Applies a warm brownish tone mimicking old photographs |
| **Negative** | Inverts every colour channel — R becomes 255-R, G becomes 255-G, B becomes 255-B |
| **Threshold** | Converts the image to pure black and white based on a brightness cutoff |
| **Channel Swap** | Rotates the RGB channels — Red becomes Green, Green becomes Blue, Blue becomes Red |
| **Warm Tone** | Boosts reds, slightly boosts greens, reduces blues for a warm sunset feel |
| **Cool Tone** | Reduces reds, slightly boosts greens, boosts blues for a cold cinematic feel |

### Kernel-based Filters
These filters use a 3×3 matrix (kernel) that slides over every pixel, computing a weighted sum of the pixel and its 8 neighbours.

| Filter | Intensity | Description |
|---|---|---|
| **Sharpen** | 1x – 5x | Amplifies contrast between a pixel and its neighbours, making edges crisper |
| **Box Blur** | 1x – 5x | Averages each pixel with its 8 neighbours equally, producing a simple blur |
| **Gaussian Blur** | 1x – 5x | Weighted average where closer neighbours matter more, producing a smooth natural blur |
| **Edge Detection** | — | Uses Sobel operators to detect boundaries between light and dark regions |
| **Emboss** | 1x – 5x | Asymmetric kernel that creates a 3D raised effect with light and shadow |

### Spatial Filters
These filters operate on regions of pixels rather than individual pixels or fixed kernels.

| Filter | Intensity | Description |
|---|---|---|
| **Pixelate** | 5px – 30px | Divides the image into blocks and fills each block with its average colour |
| **Vignette** | Light – Extreme | Darkens the edges of the image using a radial distance formula |

---

## OpenMP Parallelism

This project uses four OpenMP features:

**1. `#include <omp.h>`**
Includes the OpenMP library, giving access to all parallel directives and timing functions.

**2. `omp_set_num_threads(n)`**
Sets the number of threads before each run. Called twice per filter — once with 1 thread for the serial baseline and once with the user's chosen thread count for the parallel run.

**3. `omp_get_wtime()`**
Returns high-precision wall-clock time in seconds. Used to measure elapsed time before and after each filter run, giving accurate serial and parallel timings.

**4. `#pragma omp parallel for schedule(static) if(parallel)`**
The core parallelism directive. Splits the outer row loop of each filter across multiple threads so each thread processes an independent chunk of image rows simultaneously.

```cpp
#pragma omp parallel for schedule(static) if(parallel)
for (int i = 0; i < src.height; i++) {
    for (int j = 0; j < src.width; j++) {
        // process pixel (i, j)
    }
}
```

With 4 threads on a 1000-row image:
- Thread 0 processes rows 0–249
- Thread 1 processes rows 250–499
- Thread 2 processes rows 500–749
- Thread 3 processes rows 750–999

All four run simultaneously on separate CPU cores. This is safe because each pixel's output depends only on the original input image — no two threads ever write to the same memory location, meaning there are no race conditions.

`schedule(static)` divides rows evenly upfront, which is ideal for image processing since every pixel takes roughly the same amount of work.

The `if(parallel)` clause allows the same filter function to handle both serial and parallel execution — when `parallel=false` the pragma is deactivated and the loop runs on a single thread.

---
---

## Performance Notes

- Results vary depending on image size, CPU core count, and system load
- Larger images produce more dramatic speedup differences
- For best results use images of at least 1920×1080 resolution
- The speedup on simple filters like grayscale may be modest on small images
  due to thread creation overhead outweighing the computation — this is
  itself a demonstration of Amdahl's Law in action

---

## Built With

- **C++17**
- **OpenMP** — parallel processing
- **Win32 API** — native Windows GUI
- **stb_image / stb_image_write** — image I/O (https://github.com/nothings/stb)

---

## License

This project was developed for academic purposes as part of CSC 426 —
Parallel Programming at the University of Ilorin.

The stb_image and stb_image_write libraries are public domain software
by Sean Barrett (https://github.com/nothings/stb).
