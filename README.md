# C++ Image Edge and Feature Detector

A C++11-based computer vision project that implements **Sobel edge detection from scratch** and provides a simple Windows GUI for visualizing the original image, detected edges, image statistics, processing time, and the detected edge bounding box.

The project is implemented without OpenCV or other external image-processing libraries.

---

## 📸 Project Demo

### Edge Detection GUI

<img width="1920" height="1080" alt="Edge Detection GUI" src="https://github.com/user-attachments/assets/238d5a21-6f3f-4c0a-8a8f-a322e39c936e" />

### Original Image

<img width="1920" height="1080" alt="Original Image" src="https://github.com/user-attachments/assets/2c4e0708-4050-4736-8399-f995c933edaa" />

### Detected Edges

<img width="1920" height="1080" alt="Detected Edges" src="https://github.com/user-attachments/assets/4308a097-6a39-4421-bcf5-46aec4a7e9d0" />

---

## 🚀 Project Overview

Edge detection is a fundamental operation in computer vision and image processing.

Edges generally occur where there is a significant change in image intensity. They can represent:

* Object boundaries
* Shapes
* Lines
* Corners
* Texture changes

This project implements the **Sobel edge detection algorithm manually in C++**.

The processing pipeline is:

```text
Input PPM Image
       |
       v
RGB Image
       |
       v
Grayscale Conversion
       |
       v
Sobel Gx + Gy
       |
       v
Edge Magnitude
       |
       v
Thresholding
       |
       v
Binary Edge Image
       |
       v
Edge Statistics
       |
       v
Bounding Box
```

---

## ✨ Features

* P3 ASCII PPM image loading
* RGB to grayscale conversion
* Manual Sobel edge detection
* Horizontal gradient calculation (`Gx`)
* Vertical gradient calculation (`Gy`)
* Edge magnitude calculation
* Configurable edge threshold
* Binary edge image generation
* Edge pixel counting
* Edge percentage calculation
* Average edge strength calculation
* Edge bounding-box detection
* Processing-time measurement
* Windows GUI
* Windows file selection dialog
* Save processed edge image
* Built-in self-test mode

---

## 🛠️ Technologies Used

* **C++11**
* **MinGW GCC 6.3.0**
* **Windows 11**
* **Visual Studio Code**
* **Windows Win32 API**
* **GDI**
* **P3 ASCII PPM**

No OpenCV, CMake, Qt, CUDA, or other external image-processing libraries are required.

---

## 📁 Project Structure

```text
image-edge-detector/
|
├── include/
│   ├── image.h
│   ├── edge_detector.h
│   └── gui.h
│
├── src/
│   ├── image.cpp
│   └── edge_detector.cpp
│
├── gui/
│   └── gui.cpp
│
├── data/
│   └── output/
│       ├── edges.ppm
│       └── edges2.ppm
│
├── main.cpp
├── self_test.cpp
├── self_test.h
└── README.md
```

---

# 🔍 Image Processing Pipeline

## 1. RGB Image

The input image contains three color channels:

```text
R = Red
G = Green
B = Blue
```

The program loads the image as RGB pixel data.

---

## 2. RGB to Grayscale

Before applying the Sobel operator, the RGB image is converted into a grayscale image.

The grayscale value is calculated using:

```text
Gray = 0.299R + 0.587G + 0.114B
```

This produces a single intensity value for each pixel.

Using grayscale simplifies the edge-detection process because the Sobel operator operates on image intensity.

---

## 3. Sobel Operator

The Sobel operator uses two 3 × 3 convolution kernels.

### Horizontal Gradient — Gx

```text
-1   0   1
-2   0   2
-1   0   1
```

### Vertical Gradient — Gy

```text
-1  -2  -1
 0   0   0
 1   2   1
```

For every pixel, the surrounding 3 × 3 neighborhood is multiplied by the corresponding kernel values.

The results are summed to calculate:

```text
Gx
Gy
```

`Gx` represents the horizontal intensity gradient, while `Gy` represents the vertical intensity gradient.

---

## 4. Edge Magnitude

After calculating `Gx` and `Gy`, the program calculates the edge magnitude:

```text
Magnitude = sqrt(Gx² + Gy²)
```

A larger magnitude represents a stronger change in image intensity.

```text
Small magnitude  -> weak edge
Large magnitude  -> strong edge
```

---

## 5. Thresholding

The edge magnitude is compared against a configurable threshold.

Conceptually:

```text
if magnitude >= threshold
        |
        v
      EDGE
else
        |
        v
    NOT AN EDGE
```

This converts the edge-strength image into a binary edge image.

The threshold can be changed from the GUI.

---

# 📊 Image Statistics

The application calculates several statistics after edge detection.

### Resolution

The width and height of the input image.

Example:

```text
1920 x 1080
```

### Total Pixels

Calculated as:

```text
Total Pixels = Width × Height
```

### Edge Pixels

The number of pixels whose calculated edge magnitude is greater than or equal to the selected threshold.

### Edge Percentage

Calculated as:

```text
Edge Percentage =
(Edge Pixels / Total Pixels) × 100
```

### Average Edge Strength

The average edge magnitude of the detected edge pixels.

### Processing Time

The program measures the time required for the Sobel processing and thresholding stage.

---

# 📦 Edge Bounding Box

The program calculates a bounding box around the detected edge pixels.

It tracks:

```text
Minimum X
Maximum X
Minimum Y
Maximum Y
```

For example:

```text
Bounding Box:
X = 20 .. 800
Y = 30 .. 600
```

This provides a simple representation of the region containing the detected edges.

---

# 🖥️ GUI

The graphical interface is implemented using:

* Windows Win32 API
* GDI

The GUI provides:

* Load Image
* Detect Edges
* Save Result
* Threshold input
* Original image display
* Edge image display
* Image statistics
* Processing time
* Bounding-box information

The interface is intentionally simple so that the image-processing algorithm remains the main focus.

---

# 🧪 Self-Test

The project includes a simple self-test system without GoogleTest.

The self-tests are implemented in:

```text
self_test.cpp
self_test.h
```

Run the tests using:

```cmd
image-edge-detector.exe --test
```

The tests cover important parts of the image-processing pipeline, including:

* Image creation
* Image loading
* Grayscale conversion
* Sobel processing
* Thresholding
* Statistics
* Bounding-box calculation

---

# ⏱️ Performance Measurement

The project uses C++ timing facilities to measure processing time.

The main measured processing consists of:

```text
Sobel Processing
       +
Thresholding
```

The measured processing time is displayed in the GUI.

Actual processing time depends on:

* Image resolution
* CPU
* Compiler
* Operating system
* Current system load

No fixed performance numbers are hard-coded into the project.

---

# 🖼️ Input Format

The current implementation supports:

```text
P3 ASCII PPM
```

Example:

```text
P3
8 8
255
255 0 0
0 255 0
0 0 255
...
```

PPM was selected so that the image-loading implementation could remain simple and dependency-free.

Currently, JPEG and PNG images are not supported.

---

# 💾 Output

Processed edge images are saved as PPM files.

Example output files:

```text
data/output/edges.ppm
data/output/edges2.ppm
```

---

# ▶️ Build Instructions

## Requirements

* Windows 11
* MinGW GCC 6.3.0
* C++11
* Visual Studio Code

Check the compiler:

```cmd
g++ --version
```

Expected compiler version:

```text
g++ (MinGW.org GCC-6.3.0-1) 6.3.0
```

---

## Clone the Repository

```cmd
git clone https://github.com/Nageshkumar01/image-edge-detector.git
```

Enter the project directory:

```cmd
cd image-edge-detector
```

---

## Compile

Run:

```cmd
g++ -std=c++11 -Wall -Wextra -Iinclude main.cpp self_test.cpp src\image.cpp src\edge_detector.cpp gui\gui.cpp -o image-edge-detector.exe -lgdi32 -luser32 -lcomdlg32
```

The required Windows libraries are:

```text
-lgdi32
-luser32
-lcomdlg32
```

After successful compilation:

```text
image-edge-detector.exe
```

will be created.

---

# ▶️ Run the GUI

Run:

```cmd
image-edge-detector.exe
```

The Windows GUI should open.

Typical workflow:

```text
Load Image
     |
     v
Select PPM Image
     |
     v
Set Threshold
     |
     v
Detect Edges
     |
     v
View Results
     |
     v
Save Result
```

---

# 🧪 Run Self-Tests

Run:

```cmd
image-edge-detector.exe --test
```

This executes the built-in tests without requiring the GUI.

---

# 🎯 Why I Built This

I built this project to understand fundamental computer vision and image-processing concepts using C++.

Instead of relying on a high-level computer vision library, the main processing operations are implemented manually.

The project helped me understand:

* Image representation
* RGB to grayscale conversion
* Convolution
* Sobel filtering
* Gradient calculation
* Edge magnitude
* Thresholding
* Edge statistics
* Bounding-box calculation
* Performance measurement
* GUI visualization

---


# ⚠️ Limitations

Current limitations include:

* Only P3 ASCII PPM images are supported
* JPEG and PNG are not supported
* No physical camera input
* CPU-based image processing
* No GPU acceleration
* No DSP acceleration
* No SIMD/NEON optimization
* Windows-specific GUI
* Basic Sobel-based edge detector
* No advanced feature descriptors

---

# 🔮 Future Improvements

Possible future improvements include:

* JPEG/PNG support
* Real camera input
* Canny edge detection
* Harris corner detection
* Hough line detection
* Gaussian filtering
* Noise reduction
* Multithreaded processing
* SIMD optimization
* ARM NEON optimization
* Linux support
* Embedded camera integration
* Hardware acceleration
* Additional computer-vision algorithms

---

# 💡 Interview Concepts

This project demonstrates practical understanding of several important image-processing concepts.

### Why grayscale?

Grayscale reduces a three-channel RGB image to a single intensity channel, making gradient and edge calculations simpler.

### What is convolution?

Convolution applies a kernel to a local neighborhood of pixels to calculate a new value.

### What is the Sobel operator?

The Sobel operator estimates image intensity gradients and is commonly used for detecting edges.

### Why are there two Sobel kernels?

One kernel calculates the horizontal gradient (`Gx`) and the other calculates the vertical gradient (`Gy`).

### What is edge magnitude?

Edge magnitude combines the horizontal and vertical gradients:

```text
Magnitude = sqrt(Gx² + Gy²)
```

### What does thresholding do?

Thresholding determines whether an edge magnitude is strong enough to be classified as an edge.

### How is the bounding box calculated?

The program tracks the minimum and maximum X and Y coordinates among detected edge pixels.

### How is performance measured?

The project uses C++ timing facilities around the Sobel processing and thresholding operations.

### How could this be optimized for embedded hardware?

Possible approaches include reducing memory copies, improving cache usage, processing pixels in parallel, using SIMD/NEON instructions, and using hardware-specific acceleration where available.

---

# 📌 Project Status

**Completed — Portfolio Project**

Current functionality:

* [x] PPM image loading
* [x] RGB to grayscale conversion
* [x] Sobel Gx
* [x] Sobel Gy
* [x] Edge magnitude
* [x] Thresholding
* [x] Edge statistics
* [x] Bounding-box calculation
* [x] Processing-time measurement
* [x] Windows GUI
* [x] Image saving
* [x] Self-tests

---

# 👨‍💻 Author

**Nagesh Kumar**

B.Tech Computer Science and Engineering

GitHub:

[https://github.com/Nageshkumar01](https://github.com/Nageshkumar01)

---

# 🔗 Repository

[https://github.com/Nageshkumar01/image-edge-detector](https://github.com/Nageshkumar01/image-edge-detector)

---

## 📄 License

This project is created for educational and portfolio purposes.
