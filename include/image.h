// image.h
// Basic image containers plus PPM (P3, ASCII) input/output and RGB -> grayscale.

#ifndef IMAGE_H
#define IMAGE_H

#include <istream>
#include <string>
#include <vector>

// 8-bit RGB image. Pixels are stored interleaved: R,G,B,R,G,B,...
// Pixel (x, y) starts at index (y * width + x) * 3.
struct RgbImage {
    int width;
    int height;
    std::vector<unsigned char> data;

    RgbImage() : width(0), height(0) {}
    bool empty() const { return width <= 0 || height <= 0 || data.empty(); }
};

// 8-bit grayscale image. Pixel (x, y) is at index y * width + x.
struct GrayImage {
    int width;
    int height;
    std::vector<unsigned char> data;

    GrayImage() : width(0), height(0) {}
    bool empty() const { return width <= 0 || height <= 0 || data.empty(); }
};

// Parse a P3 (ASCII) PPM from any input stream.
// Supports '#' comments anywhere in the file. If maxval != 255 the samples
// are rescaled to 0..255. Returns false and fills 'error' on failure.
bool ParsePPM(std::istream& in, RgbImage& img, std::string& error);

// Load a P3 PPM from a file path (wrapper around ParsePPM).
bool LoadPPM(const std::string& path, RgbImage& img, std::string& error);

// Save an RGB image as a P3 (ASCII) PPM with maxval 255.
bool SavePPM(const std::string& path, const RgbImage& img, std::string& error);

// Integer luma conversion (BT.601 weights scaled by 256):
//   gray = (77*R + 150*G + 29*B) >> 8
void RgbToGrayscale(const RgbImage& src, GrayImage& dst);

// Replicate a grayscale image into R=G=B (used for saving / displaying).
void GrayToRgb(const GrayImage& src, RgbImage& dst);

#endif // IMAGE_H
