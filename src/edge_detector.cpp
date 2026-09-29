// edge_detector.cpp
#include "edge_detector.h"

#include <chrono>
#include <cmath>

void DetectEdges(const GrayImage& gray, int threshold,
                 GrayImage& edges, EdgeStats& stats) {
    const int w = gray.width;
    const int h = gray.height;
    const size_t total = static_cast<size_t>(w) * static_cast<size_t>(h);

    // Allocate/clear the output once, BEFORE the pixel loops and the timer.
    edges.width  = w;
    edges.height = h;
    edges.data.assign(total, static_cast<unsigned char>(0));

    stats = EdgeStats();
    stats.width       = w;
    stats.height      = h;
    stats.totalPixels = static_cast<long>(total);

    long   edgeCount = 0;
    double magnitudeSum = 0.0;
    int minX = w, minY = h, maxX = -1, maxY = -1;

    const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();

    // The 3x3 kernel needs a full neighbourhood, so only interior pixels are processed.
    if (w >= 3 && h >= 3) {
        const unsigned char* src = gray.data.data();
        unsigned char* dst = edges.data.data();

        for (int y = 1; y < h - 1; ++y) {
            // Pointers to the three rows of the 3x3 window.
            const unsigned char* rowPrev = src + static_cast<size_t>(y - 1) * w;
            const unsigned char* rowCur  = src + static_cast<size_t>(y)     * w;
            const unsigned char* rowNext = src + static_cast<size_t>(y + 1) * w;
            unsigned char* outRow = dst + static_cast<size_t>(y) * w;

            for (int x = 1; x < w - 1; ++x) {
                // Gx kernel: -1 0 1 / -2 0 2 / -1 0 1   (responds to vertical edges)
                const int gx = -rowPrev[x - 1] + rowPrev[x + 1]
                             - 2 * rowCur[x - 1] + 2 * rowCur[x + 1]
                             - rowNext[x - 1] + rowNext[x + 1];

                // Gy kernel: -1 -2 -1 / 0 0 0 / 1 2 1   (responds to horizontal edges)
                const int gy = -rowPrev[x - 1] - 2 * rowPrev[x] - rowPrev[x + 1]
                             + rowNext[x - 1] + 2 * rowNext[x] + rowNext[x + 1];

                // Edge magnitude = length of the gradient vector.
                const double magnitude = std::sqrt(static_cast<double>(gx * gx + gy * gy));

                // Thresholding: keep the pixel only if the gradient is strong enough.
                if (magnitude >= threshold) {
                    outRow[x] = 255;
                    ++edgeCount;
                    magnitudeSum += magnitude;

                    // Bounding box grows to include this edge pixel.
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
    }

    const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    stats.processingTimeMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();

    stats.edgePixels = edgeCount;
    if (total > 0) {
        stats.edgePercentage = 100.0 * static_cast<double>(edgeCount) /
                               static_cast<double>(total);
    }
    if (edgeCount > 0) {
        stats.averageEdgeStrength = magnitudeSum / static_cast<double>(edgeCount);
        stats.hasBoundingBox = true;
        stats.minX = minX;
        stats.minY = minY;
        stats.maxX = maxX;
        stats.maxY = maxY;
    }
}
