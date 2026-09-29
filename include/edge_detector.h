// edge_detector.h
// Manual Sobel edge detection + thresholding + statistics + bounding box.

#ifndef EDGE_DETECTOR_H
#define EDGE_DETECTOR_H

#include "image.h"

// Largest possible Sobel magnitude for 8-bit input:
// Gx and Gy are each at most 4*255 = 1020, so sqrt(1020^2 + 1020^2) ~= 1442.6.
const int MAX_SOBEL_MAGNITUDE = 1443;

struct EdgeStats {
    int  width;
    int  height;
    long totalPixels;          // width * height
    long edgePixels;           // pixels whose magnitude >= threshold
    double edgePercentage;     // 100 * edgePixels / totalPixels
    double averageEdgeStrength;// mean magnitude over the edge pixels only
    double processingTimeMs;   // Sobel + thresholding (+ stats accumulation)

    bool hasBoundingBox;       // false when no edge pixel was found
    int  minX, minY, maxX, maxY; // inclusive pixel coordinates

    EdgeStats()
        : width(0), height(0), totalPixels(0), edgePixels(0),
          edgePercentage(0.0), averageEdgeStrength(0.0), processingTimeMs(0.0),
          hasBoundingBox(false), minX(0), minY(0), maxX(0), maxY(0) {}
};

// Runs Sobel on 'gray', writes a binary edge image into 'edges'
// (255 = edge, 0 = no edge) and fills 'stats'.
// Border pixels (first/last row and column) are never marked as edges because
// the 3x3 kernel does not fit there.
void DetectEdges(const GrayImage& gray, int threshold,
                 GrayImage& edges, EdgeStats& stats);

#endif // EDGE_DETECTOR_H
