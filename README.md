````markdown
# C++ Image Edge and Feature Detector

A C++11-based computer vision project that implements **Sobel edge detection from scratch** and provides a simple Windows GUI for visualizing the original image, detected edges, image statistics, and edge bounding box.

The project is implemented without OpenCV or other external image-processing libraries.

---

## 📸 Project Demo

### Edge Detection GUI

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/238d5a21-6f3f-4c0a-8a8f-a322e39c936e" />


### Original Image

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/2c4e0708-4050-4736-8399-f995c933edaa" />


### Detected Edges

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/4308a097-6a39-4421-bcf5-46aec4a7e9d0" />

---

## 🚀 Project Overview

Edge detection is an important basic operation in computer vision and image processing.

Edges usually represent locations where image intensity changes significantly, such as:

- Object boundaries
- Shapes
- Lines
- Corners
- Texture changes

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
````

---

# ✨ Features

* P3 ASCII PPM image loading
* RGB to grayscale conversion
* Manual Sobel edge detection
* Horizontal gradient calculation
* Vertical gradient calculation
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

# 🛠️ Technologies Used

* **C++11**
* **MinGW GCC 6.3.0**
* **Windows 11**
* **Visual Studio Code**
* **Windows Win32 API**
* **GDI**
* **P3 ASCII PPM**

No OpenCV, CMake, Qt, CUDA, or other external libraries are required.

---

# 📁 Project Structure

```text
image-edge-detector/
│
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

# 🔍 How the Algorithm Works

## 1. RGB to Grayscale

The original image contains three color channels:

```text
R = Red
G = Green
B = Blue
```

For edge detection, the image is first converted into a single grayscale intensity value.

The project uses a weighted RGB-to-grayscale conversion:

```text
Gray = 0.299R + 0.587G + 0.114B
```

This produces a single intensity value for each pixel.

---

# 2. Sobel Edge Detection

The Sobel operator uses two 3×3 convolution kernels.

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

For every pixel, the surrounding 3×3 neighborhood is multiplied by the corresponding kernel values.

The results are summed to calculate:

```text
Gx
Gy
```

---

# 3. Edge Magnitude

After calculating the horizontal and vertical gradients, the edge strength is calculated using:

```text
Magnitude = sqrt(Gx² + Gy²)
```

A larger magnitude indicates a stronger intensity change.

For example:

```text
Small magnitude  → weak edge
Large magnitude  → strong edge
```

---

# 4. Thresholding

The calculated edge magnitude is compared against a configurable threshold.

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

This converts the grayscale edge-strength image into a binary edge image.

The threshold can be changed from the GUI.

---

# 5. Edge Statistics

The program calculates several statistics after edge detection.

### Resolution

Example:

```text
1920 x 1080
```

### Total Pixels

```text
width × height
```

### Edge Pixels

Number of pixels whose edge magnitude is greater than or equal to the selected threshold.

### Edge Percentage

```text
Edge Percentage =
(Edge Pixels / Total Pixels) × 100
```

### Average Edge Strength

The average magnitude of the detected edge pixels.

### Processing Time

The program measures the time required for the Sobel processing and thresholding stage.

---

# 6. Edge Bounding Box

The program also calculates the bounding box around detected edge pixels.

The bounding box contains:

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

This provides a simple way to describe the region containing detected edges.

---

# 🖥️ GUI

The project uses the Windows **Win32 API and GDI** for the graphical interface.

The GUI provides:

```text
+----------------------------------------------------------+
|             C++ IMAGE EDGE DETECTOR                      |
+----------------------------------------------------------+
|                                                          |
|   ORIGINAL IMAGE             EDGE IMAGE                 |
|                                                          |
|   +---------------+          +---------------+           |
|   |               |          |               |           |
|   |    ORIGINAL   |   --->   |     EDGES     |           |
|   |               |          |               |           |
|   +---------------+          +---------------+           |
|                                                          |
|   Resolution:                                            |
|   Edge Pixels:                                           |
|   Edge Percentage:                                       |
|   Average Edge Strength:                                 |
|   Processing Time:                                       |
|   Bounding Box:                                          |
|                                                          |
|   Threshold: [ 100 ]                                     |
|                                                          |
|   [ Load Image ] [ Detect Edges ] [ Save Result ]        |
|                                                          |
+----------------------------------------------------------+
<img width="1408" height="768" alt="Gemini_Generated_Image_ddtnedddtnedddtn" src="https://github.com/user-attachments/assets/7071f0d7-eed7-4d48-95f6-e978328dcff1" />

```

The GUI is intentionally simple so that the image-processing algorithm remains the main focus.

---

# 🧪 Self-Test

The project includes a simple self-test mode without GoogleTest.

Run:

```cmd
image-edge-detector.exe --test
```

The tests check the core functionality such as:

* Image creation
* Image loading
* Grayscale conversion
* Sobel processing
* Thresholding
* Edge statistics
* Bounding-box calculation

This provides a simple way to verify the processing logic without opening the GUI.

---

# ⚙️ Performance Measurement

The project uses C++ timing facilities to measure processing time.

The measured operation includes the main edge-processing work:

```text
Sobel Processing
       +
Thresholding
```

The project does not use hard-coded performance values.

Actual processing time depends on:

* Image resolution
* CPU
* Compiler
* Operating system
* Current system load

---

# 🖼️ Input Format

The current implementation uses:

```text
P3 ASCII PPM
```

Example:

```text
P3
8 8
255
255 0 0
...
```

The project intentionally uses PPM so that image loading can be implemented without external libraries.

JPEG and PNG are not currently supported.

---

# 💾 Output

Processed edge images are saved as PPM files.

Example:

```text
data/output/edges.ppm
data/output/edges2.ppm
```

The output represents the detected edges after thresholding.

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

Expected compiler:

```text
g++ (MinGW.org GCC-6.3.0-1) 6.3.0
```

---

## Compile

Open Windows CMD and navigate to the project directory:

```cmd
cd "C:\project\Camera ROle Qualcomm\Project3\image-edge-detector"
```

Compile:

```cmd
g++ -std=c++11 -Wall -Wextra -Iinclude main.cpp self_test.cpp src\image.cpp src\edge_detector.cpp gui\gui.cpp -o image-edge-detector.exe -lgdi32 -luser32 -lcomdlg32
```

If compilation succeeds, the executable will be created:

```text
image-edge-detector.exe
```

---

# ▶️ Run the GUI

Run:

```cmd
image-edge-detector.exe
```

The Windows GUI should open.

The typical workflow is:

```text
Load Image
     ↓
Select PPM Image
     ↓
Set Threshold
     ↓
Detect Edges
     ↓
View Statistics
     ↓
Save Result
```

---

# 🧪 Run Tests

To run the built-in self-tests:

```cmd
image-edge-detector.exe --test
```

The program should execute the tests and report their results in the terminal.

---

# 🎯 Why I Built This

I built this project to understand fundamental computer vision and image-processing concepts using C++ without relying on high-level image-processing frameworks.

The project focuses on understanding what happens at the pixel level during edge detection.

Through this project I worked with:

* Image representation
* Grayscale conversion
* Convolution
* Sobel filters
* Gradient calculation
* Thresholding
* Edge statistics
* Bounding-box calculation
* Performance measurement
* GUI visualization


# ⚠️ Limitations

Current limitations include:

* Only P3 ASCII PPM images are supported
* No JPEG or PNG support
* No physical camera input
* Sobel processing is CPU-based
* No GPU acceleration
* No DSP acceleration
* No SIMD/NEON optimization
* Windows-specific GUI
* Basic Sobel edge detector
* No advanced feature descriptors

---

# 🔮 Future Improvements

Possible future improvements:

* JPEG/PNG support
* Real camera input
* Canny edge detection
* Harris corner detection
* Hough line detection
* Additional convolution filters
* Noise reduction
* Gaussian blur
* Multi-scale edge detection
* Multithreaded processing
* SIMD optimization
* ARM NEON optimization
* Linux support
* Embedded camera integration
* Hardware acceleration

---

# 📚 Interview Concepts Demonstrated

This project provides practical examples of:

### Grayscale

Converting a three-channel RGB image into a single intensity channel.

### Convolution

Applying a kernel over a local neighborhood of pixels.

### Sobel Operator

Using two kernels to calculate horizontal and vertical intensity gradients.

### Edge Magnitude

Combining the horizontal and vertical gradients to estimate edge strength.

### Thresholding

Converting continuous edge strength into a binary edge image.

### Bounding Box

Finding the minimum and maximum X/Y coordinates of detected edge pixels.

### Performance Measurement

Using C++ timing facilities to measure processing time.

---


# 📌 Project Status

**Completed — Portfolio Project**

The project currently includes:

* [x] PPM image loading
* [x] Grayscale conversion
* [x] Sobel Gx calculation
* [x] Sobel Gy calculation
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

## 🔗 Repository

**GitHub:**
[https://github.com/Nageshkumar01/image-edge-detector](https://github.com/Nageshkumar01/image-edge-detector)

---

## 📄 License

This project is created for educational and portfolio purposes.
