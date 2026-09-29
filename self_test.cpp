// self_test.cpp
// Minimal test runner: each Check() prints PASS/FAIL and counts failures.
#include "self_test.h"

#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>

#include "edge_detector.h"
#include "image.h"

namespace {

int g_checks   = 0;
int g_failures = 0;

void Check(bool condition, const char* description) {
    ++g_checks;
    if (condition) {
        std::printf("  [PASS] %s\n", description);
    } else {
        ++g_failures;
        std::printf("  [FAIL] %s\n", description);
    }
}

bool Near(double a, double b, double eps) { return std::fabs(a - b) <= eps; }

// 5x5 image with a step edge: 0 for coordinate < 2, 255 for coordinate >= 2.
// vertical=true  -> the value depends on x (a vertical edge)
// vertical=false -> the value depends on y (a horizontal edge)
GrayImage MakeStepImage(bool vertical) {
    GrayImage img;
    img.width = 5;
    img.height = 5;
    img.data.resize(25);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            const int c = vertical ? x : y;
            img.data[y * 5 + x] = (c >= 2) ? 255 : 0;
        }
    }
    return img;
}

GrayImage MakeFlatImage(int w, int h, unsigned char value) {
    GrayImage img;
    img.width = w;
    img.height = h;
    img.data.assign(static_cast<size_t>(w) * h, value);
    return img;
}

void TestPpmLoading() {
    std::printf("[Test 1] PPM loading\n");

    {
        std::istringstream ss(
            "P3\n# a comment line\n2 2\n255\n"
            "255 0 0   0 255 0\n"
            "0 0 255   255 255 255\n");
        RgbImage img;
        std::string err;
        const bool ok = ParsePPM(ss, img, err);
        Check(ok, "valid 2x2 P3 file parses");
        const bool good = ok && img.width == 2 && img.height == 2 && img.data.size() == 12;
        Check(good, "resolution is 2 x 2 and 12 samples were stored");
        Check(good && img.data[0] == 255 && img.data[1] == 0 && img.data[2] == 0,
              "pixel (0,0) is red");
        Check(good && img.data[3] == 0 && img.data[4] == 255 && img.data[5] == 0,
              "pixel (1,0) is green");
        Check(good && img.data[6] == 0 && img.data[7] == 0 && img.data[8] == 255,
              "pixel (0,1) is blue");
        Check(good && img.data[9] == 255 && img.data[10] == 255 && img.data[11] == 255,
              "pixel (1,1) is white");
    }
    {
        std::istringstream ss("P3 # c1\n1 1 # c2\n255\n10 20 30");
        RgbImage img;
        std::string err;
        const bool ok = ParsePPM(ss, img, err);
        Check(ok && img.data.size() == 3 && img.data[2] == 30,
              "comments inside the header and no trailing newline are handled");
    }
    {
        std::istringstream ss("P3\n1 1\n15\n15 0 5\n");
        RgbImage img;
        std::string err;
        const bool ok = ParsePPM(ss, img, err);
        Check(ok && img.data.size() == 3 && img.data[0] == 255 && img.data[1] == 0 &&
              img.data[2] == 85, "maxval 15 is rescaled to 0..255");
    }
    {
        std::istringstream ss("P6\n1 1\n255\nabc");
        RgbImage img;
        std::string err;
        Check(!ParsePPM(ss, img, err), "binary P6 file is rejected");
    }
    {
        std::istringstream ss("P3\n2 2\n255\n1 2 3 4 5 6\n");
        RgbImage img;
        std::string err;
        Check(!ParsePPM(ss, img, err), "truncated pixel data is rejected");
    }
    {
        std::istringstream ss("P3\n1 1\n255\n300 0 0\n");
        RgbImage img;
        std::string err;
        Check(!ParsePPM(ss, img, err), "value above maxval is rejected");
    }
    {
        // Save -> load round trip through a real file.
        RgbImage a;
        a.width = 3;
        a.height = 2;
        const unsigned char vals[18] = {0, 1, 2, 10, 20, 30, 100, 110, 120,
                                        200, 210, 220, 250, 251, 252, 255, 254, 253};
        a.data.assign(vals, vals + 18);
        std::string err;
        const char* tmpName = "selftest_tmp.ppm";
        const bool saved = SavePPM(tmpName, a, err);
        RgbImage b;
        const bool loaded = saved && LoadPPM(tmpName, b, err);
        Check(saved, "SavePPM writes a file");
        Check(loaded && b.width == 3 && b.height == 2 && b.data == a.data,
              "save -> load round trip returns identical pixels");
        std::remove(tmpName);
    }
}

void TestGrayscale() {
    std::printf("[Test 2] Grayscale conversion\n");
    RgbImage rgb;
    rgb.width = 5;
    rgb.height = 1;
    const unsigned char vals[15] = {
        0, 0, 0,        // black
        255, 255, 255,  // white
        255, 0, 0,      // red
        0, 255, 0,      // green
        0, 0, 255       // blue
    };
    rgb.data.assign(vals, vals + 15);

    GrayImage gray;
    RgbToGrayscale(rgb, gray);

    Check(gray.width == 5 && gray.height == 1 && gray.data.size() == 5,
          "gray image has the same resolution");
    const bool good = gray.data.size() == 5;
    Check(good && gray.data[0] == 0,   "black -> 0");
    Check(good && gray.data[1] == 255, "white -> 255");
    Check(good && gray.data[2] == 76,  "pure red -> (77*255)>>8 = 76");
    Check(good && gray.data[3] == 149, "pure green -> (150*255)>>8 = 149");
    Check(good && gray.data[4] == 28,  "pure blue -> (29*255)>>8 = 28");
}

void TestSobelAndThreshold() {
    std::printf("[Test 3] Sobel + thresholding + statistics\n");

    // Vertical step edge: Gx = 1020, Gy = 0 at x=1 and x=2 (y=1..3) -> 6 edge pixels.
    {
        GrayImage img = MakeStepImage(true);
        GrayImage edges;
        EdgeStats st;
        DetectEdges(img, 100, edges, st);

        Check(st.totalPixels == 25 && st.width == 5 && st.height == 5,
              "resolution and total pixel count");
        Check(st.edgePixels == 6, "vertical step: 6 edge pixels");
        Check(Near(st.edgePercentage, 24.0, 1e-9), "edge percentage = 6/25 = 24%");
        Check(Near(st.averageEdgeStrength, 1020.0, 1e-6),
              "average edge strength = 1020 (hand-calculated)");
        Check(st.hasBoundingBox && st.minX == 1 && st.maxX == 2 &&
              st.minY == 1 && st.maxY == 3, "bounding box is X=1..2, Y=1..3");
        Check(edges.data.size() == 25 && edges.data[1 * 5 + 1] == 255,
              "edge pixel (1,1) is 255");
        Check(edges.data.size() == 25 && edges.data[1 * 5 + 3] == 0,
              "flat-region pixel (3,1) is 0");
        Check(edges.data.size() == 25 && edges.data[0] == 0 && edges.data[24] == 0,
              "border pixels are never marked as edges");
        Check(st.processingTimeMs >= 0.0, "processing time is not negative");
    }

    // Horizontal step edge: uses Gy instead of Gx.
    {
        GrayImage img = MakeStepImage(false);
        GrayImage edges;
        EdgeStats st;
        DetectEdges(img, 100, edges, st);
        Check(st.edgePixels == 6, "horizontal step: 6 edge pixels (Gy path)");
        Check(st.hasBoundingBox && st.minX == 1 && st.maxX == 3 &&
              st.minY == 1 && st.maxY == 2, "bounding box is X=1..3, Y=1..2");
        Check(Near(st.averageEdgeStrength, 1020.0, 1e-6), "Gy magnitude is also 1020");
    }

    // Threshold behaviour: magnitude >= threshold counts as an edge.
    {
        GrayImage img = MakeStepImage(true);
        GrayImage edges;
        EdgeStats st;
        DetectEdges(img, 1020, edges, st);
        Check(st.edgePixels == 6, "threshold 1020 keeps magnitude 1020");
        DetectEdges(img, 1021, edges, st);
        Check(st.edgePixels == 0 && !st.hasBoundingBox,
              "threshold 1021 removes everything and there is no bounding box");
    }

    // Flat image has zero gradient everywhere.
    {
        GrayImage img = MakeFlatImage(8, 6, 128);
        GrayImage edges;
        EdgeStats st;
        DetectEdges(img, 1, edges, st);
        Check(st.edgePixels == 0 && Near(st.edgePercentage, 0.0, 1e-12) &&
              Near(st.averageEdgeStrength, 0.0, 1e-12), "flat image: no edges");
    }

    // Images smaller than 3x3 must not crash.
    {
        GrayImage img = MakeFlatImage(2, 2, 50);
        GrayImage edges;
        EdgeStats st;
        DetectEdges(img, 10, edges, st);
        Check(st.edgePixels == 0 && edges.data.size() == 4, "2x2 image is handled safely");
    }
}

} // namespace

int RunSelfTests() {
    g_checks = 0;
    g_failures = 0;

    std::printf("=== Image Edge Detector self-test ===\n");
    TestPpmLoading();
    TestGrayscale();
    TestSobelAndThreshold();

    std::printf("\n%d checks, %d failed.\n", g_checks, g_failures);
    std::printf(g_failures == 0 ? "RESULT: ALL TESTS PASSED\n" : "RESULT: TESTS FAILED\n");
    return g_failures == 0 ? 0 : 1;
}
