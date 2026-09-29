// image.cpp
#include "image.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <new>

namespace {

// Reads the next whitespace-separated token, skipping '#' comments
// (a comment runs from '#' to the end of the line).
bool ReadToken(std::istream& in, std::string& token) {
    token.clear();
    char c;

    // Skip leading whitespace and comments until the first token character.
    while (in.get(c)) {
        if (c == '#') {
            while (in.get(c) && c != '\n') {}
            continue;
        }
        if (!std::isspace(static_cast<unsigned char>(c))) {
            token.push_back(c);
            break;
        }
    }
    if (token.empty()) return false;

    // Read the rest of the token.
    while (in.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) break;
        if (c == '#') {
            while (in.get(c) && c != '\n') {}
            break;
        }
        token.push_back(c);
    }
    return true;
}

bool ReadInt(std::istream& in, long& value) {
    std::string tok;
    if (!ReadToken(in, tok)) return false;
    char* end = 0;
    value = std::strtol(tok.c_str(), &end, 10);
    return end != tok.c_str() && *end == '\0';
}

const long MAX_DIMENSION = 10000;

} // namespace

bool ParsePPM(std::istream& in, RgbImage& img, std::string& error) {
    std::string magic;
    if (!ReadToken(in, magic) || magic != "P3") {
        error = "Not a P3 (ASCII) PPM file. The first token must be \"P3\".";
        return false;
    }

    long w = 0, h = 0, maxval = 0;
    if (!ReadInt(in, w) || !ReadInt(in, h) || !ReadInt(in, maxval)) {
        error = "Invalid PPM header (expected: width height maxval).";
        return false;
    }
    if (w <= 0 || h <= 0 || w > MAX_DIMENSION || h > MAX_DIMENSION) {
        error = "Unsupported image size (each side must be 1..10000).";
        return false;
    }
    if (maxval <= 0 || maxval > 65535) {
        error = "Invalid maxval (must be 1..65535).";
        return false;
    }

    const size_t total = static_cast<size_t>(w) * static_cast<size_t>(h) * 3;

    RgbImage tmp;
    tmp.width  = static_cast<int>(w);
    tmp.height = static_cast<int>(h);
    try {
        tmp.data.resize(total);
    } catch (const std::bad_alloc&) {
        error = "Not enough memory for this image.";
        return false;
    }

    for (size_t i = 0; i < total; ++i) {
        long v = 0;
        if (!ReadInt(in, v)) {
            error = "Pixel data is truncated or contains a non-numeric value.";
            return false;
        }
        if (v < 0 || v > maxval) {
            error = "A pixel value is outside 0..maxval.";
            return false;
        }
        if (maxval == 255) {
            tmp.data[i] = static_cast<unsigned char>(v);
        } else {
            // Rescale to 0..255 with rounding.
            tmp.data[i] = static_cast<unsigned char>((v * 255 + maxval / 2) / maxval);
        }
    }

    img.width  = tmp.width;
    img.height = tmp.height;
    img.data.swap(tmp.data);
    return true;
}

bool LoadPPM(const std::string& path, RgbImage& img, std::string& error) {
    std::ifstream file(path.c_str());
    if (!file) {
        error = "Cannot open file: " + path;
        return false;
    }
    return ParsePPM(file, img, error);
}

bool SavePPM(const std::string& path, const RgbImage& img, std::string& error) {
    if (img.empty()) {
        error = "Nothing to save (image is empty).";
        return false;
    }
    std::ofstream out(path.c_str(), std::ios::out | std::ios::trunc);
    if (!out) {
        error = "Cannot create file: " + path;
        return false;
    }

    out << "P3\n# Written by C++ Image Edge Detector\n"
        << img.width << " " << img.height << "\n255\n";

    for (int y = 0; y < img.height; ++y) {
        const unsigned char* row = &img.data[static_cast<size_t>(y) * img.width * 3];
        for (int x = 0; x < img.width; ++x) {
            out << static_cast<int>(row[3 * x])     << ' '
                << static_cast<int>(row[3 * x + 1]) << ' '
                << static_cast<int>(row[3 * x + 2]);
            out << (x + 1 < img.width ? "  " : "\n");
        }
    }

    out.flush();
    if (!out) {
        error = "Write error while saving: " + path;
        return false;
    }
    return true;
}

void RgbToGrayscale(const RgbImage& src, GrayImage& dst) {
    dst.width  = src.width;
    dst.height = src.height;
    const size_t n = static_cast<size_t>(src.width) * static_cast<size_t>(src.height);
    dst.data.resize(n);  // one allocation, outside the pixel loop

    const unsigned char* rgb = src.data.data();
    unsigned char* gray = dst.data.data();
    for (size_t i = 0; i < n; ++i) {
        const int r = rgb[3 * i];
        const int g = rgb[3 * i + 1];
        const int b = rgb[3 * i + 2];
        // 77 + 150 + 29 = 256, so ">> 8" divides by 256 and the result stays <= 255.
        gray[i] = static_cast<unsigned char>((77 * r + 150 * g + 29 * b) >> 8);
    }
}

void GrayToRgb(const GrayImage& src, RgbImage& dst) {
    dst.width  = src.width;
    dst.height = src.height;
    const size_t n = static_cast<size_t>(src.width) * static_cast<size_t>(src.height);
    dst.data.resize(n * 3);

    const unsigned char* gray = src.data.data();
    unsigned char* rgb = dst.data.data();
    for (size_t i = 0; i < n; ++i) {
        rgb[3 * i]     = gray[i];
        rgb[3 * i + 1] = gray[i];
        rgb[3 * i + 2] = gray[i];
    }
}
