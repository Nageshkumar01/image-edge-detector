// gui.cpp
// Simple Win32 + GDI front end (ANSI API, no external libraries).
//
// Structure:
//   - One global AppState-style set of variables (image, gray, edges, stats).
//   - WndProc handles WM_CREATE (make controls), WM_COMMAND (buttons),
//     WM_PAINT (draw images + statistics) and WM_DESTROY.
//   - Images are converted to a 24-bit BGR "DIB" buffer that StretchDIBits can draw.

#include "gui.h"

#include <commdlg.h>

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "edge_detector.h"
#include "image.h"

namespace {

// ---------------------------------------------------------------- layout ----
const int CLIENT_W = 980;
const int CLIENT_H = 700;

const RECT PANEL_ORIGINAL = { 30,  80, 470, 410 };  // 440 x 330
const RECT PANEL_EDGES    = { 510, 80, 950, 410 };  // 440 x 330

const int STATS_X = 30;
const int STATS_Y = 430;
const int STATS_LINE_HEIGHT = 21;

enum ControlId {
    ID_BTN_LOAD = 101,
    ID_BTN_DETECT,
    ID_BTN_SAVE,
    ID_EDIT_THRESHOLD
};

// ----------------------------------------------------------- app state ------
struct DisplayBitmap {
    int width;
    int height;
    int stride;                       // bytes per row, multiple of 4 (GDI rule)
    std::vector<unsigned char> pixels; // BGR, top-down
    DisplayBitmap() : width(0), height(0), stride(0) {}
};

HWND g_hEdit = NULL;
HFONT g_fontUi    = NULL;   // stock font, do not delete
HFONT g_fontTitle = NULL;
HFONT g_fontMono  = NULL;

RgbImage  g_original;
GrayImage g_gray;
GrayImage g_edges;
EdgeStats g_stats;
bool g_hasResult = false;

DisplayBitmap g_bmpOriginal;
DisplayBitmap g_bmpEdges;

std::string g_path;
std::string g_status = "Load a P3 .ppm image to begin.";

// -------------------------------------------------------------- helpers -----
std::string Fixed(double v, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << v;
    return ss.str();
}

std::string ToStr(long v) {
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

std::string BaseName(const std::string& path) {
    const size_t pos = path.find_last_of("\\/");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

// Convert image data into the layout GDI expects: 24-bit, BGR order,
// each row padded to a multiple of 4 bytes.
void BuildFromRgb(DisplayBitmap& bmp, const RgbImage& img) {
    bmp.width  = img.width;
    bmp.height = img.height;
    bmp.stride = ((img.width * 3 + 3) / 4) * 4;
    bmp.pixels.assign(static_cast<size_t>(bmp.stride) * img.height, 0);

    for (int y = 0; y < img.height; ++y) {
        const unsigned char* src = &img.data[static_cast<size_t>(y) * img.width * 3];
        unsigned char* dst = &bmp.pixels[static_cast<size_t>(y) * bmp.stride];
        for (int x = 0; x < img.width; ++x) {
            dst[3 * x]     = src[3 * x + 2];  // B
            dst[3 * x + 1] = src[3 * x + 1];  // G
            dst[3 * x + 2] = src[3 * x];      // R
        }
    }
}

void BuildFromGray(DisplayBitmap& bmp, const GrayImage& img) {
    bmp.width  = img.width;
    bmp.height = img.height;
    bmp.stride = ((img.width * 3 + 3) / 4) * 4;
    bmp.pixels.assign(static_cast<size_t>(bmp.stride) * img.height, 0);

    for (int y = 0; y < img.height; ++y) {
        const unsigned char* src = &img.data[static_cast<size_t>(y) * img.width];
        unsigned char* dst = &bmp.pixels[static_cast<size_t>(y) * bmp.stride];
        for (int x = 0; x < img.width; ++x) {
            dst[3 * x] = dst[3 * x + 1] = dst[3 * x + 2] = src[x];
        }
    }
}

// Draw a bitmap inside a panel, keeping the aspect ratio.
void DrawImagePanel(HDC hdc, const RECT& panel, const DisplayBitmap& bmp,
                    const char* emptyText) {
    HBRUSH bg = CreateSolidBrush(RGB(45, 45, 48));
    FillRect(hdc, &panel, bg);
    DeleteObject(bg);

    if (bmp.width > 0 && bmp.height > 0 && !bmp.pixels.empty()) {
        const int pw = panel.right - panel.left;
        const int ph = panel.bottom - panel.top;
        const double sx = static_cast<double>(pw) / bmp.width;
        const double sy = static_cast<double>(ph) / bmp.height;
        const double scale = sx < sy ? sx : sy;

        int dw = static_cast<int>(bmp.width * scale);
        int dh = static_cast<int>(bmp.height * scale);
        if (dw < 1) dw = 1;
        if (dh < 1) dh = 1;
        const int dx = panel.left + (pw - dw) / 2;
        const int dy = panel.top + (ph - dh) / 2;

        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof(bi));
        bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth       = bmp.width;
        bi.bmiHeader.biHeight      = -bmp.height;   // negative = top-down rows
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 24;
        bi.bmiHeader.biCompression = BI_RGB;

        // Nearest-neighbour when enlarging (keeps pixels sharp), HALFTONE when shrinking.
        SetStretchBltMode(hdc, scale >= 1.0 ? COLORONCOLOR : HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, NULL);
        StretchDIBits(hdc, dx, dy, dw, dh,
                      0, 0, bmp.width, bmp.height,
                      &bmp.pixels[0], &bi, DIB_RGB_COLORS, SRCCOPY);
    } else {
        RECT r = panel;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(170, 170, 170));
        SelectObject(hdc, g_fontUi);
        DrawTextA(hdc, emptyText, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    FrameRect(hdc, &panel, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
}

void DrawStatsLine(HDC hdc, int row, const std::string& text) {
    TextOutA(hdc, STATS_X, STATS_Y + row * STATS_LINE_HEIGHT,
             text.c_str(), static_cast<int>(text.size()));
}

void PaintAll(HDC hdc, const RECT& client) {
    FillRect(hdc, &client, GetSysColorBrush(COLOR_BTNFACE));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    // Title.
    SelectObject(hdc, g_fontTitle);
    RECT title = { 0, 10, client.right, 50 };
    DrawTextA(hdc, "C++ IMAGE EDGE DETECTOR", -1, &title,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Panel captions and the arrow between them.
    SelectObject(hdc, g_fontUi);
    TextOutA(hdc, PANEL_ORIGINAL.left, 58, "ORIGINAL IMAGE", 14);
    TextOutA(hdc, PANEL_EDGES.left, 58, "EDGE IMAGE", 10);
    SelectObject(hdc, g_fontTitle);
    RECT arrow = { PANEL_ORIGINAL.right, PANEL_ORIGINAL.top,
                   PANEL_EDGES.left, PANEL_ORIGINAL.bottom };
    DrawTextA(hdc, "->", -1, &arrow, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Images.
    DrawImagePanel(hdc, PANEL_ORIGINAL, g_bmpOriginal, "No image loaded");
    DrawImagePanel(hdc, PANEL_EDGES, g_bmpEdges, "Press \"Detect Edges\"");

    // Statistics (monospaced so the columns line up).
    SelectObject(hdc, g_fontMono);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    const bool haveImage = !g_original.empty();

    DrawStatsLine(hdc, 0, "File:                   " + (g_path.empty() ? std::string("-") : BaseName(g_path)));
    DrawStatsLine(hdc, 1, "Resolution:             " +
        (haveImage ? ToStr(g_original.width) + " x " + ToStr(g_original.height) : std::string("-")));
    DrawStatsLine(hdc, 2, "Total Pixels:           " +
        (haveImage ? ToStr(static_cast<long>(g_original.width) * g_original.height) : std::string("-")));

    if (g_hasResult) {
        DrawStatsLine(hdc, 3, "Edge Pixels:            " + ToStr(g_stats.edgePixels));
        DrawStatsLine(hdc, 4, "Edge Percentage:        " + Fixed(g_stats.edgePercentage, 2) + " %");
        DrawStatsLine(hdc, 5, "Average Edge Strength:  " + Fixed(g_stats.averageEdgeStrength, 2) +
                              "  (mean magnitude of edge pixels)");
        DrawStatsLine(hdc, 6, "Processing Time:        " + Fixed(g_stats.processingTimeMs, 3) +
                              " ms  (Sobel + threshold)");
        if (g_stats.hasBoundingBox) {
            DrawStatsLine(hdc, 7, "Bounding Box:           X=" + ToStr(g_stats.minX) + ".." +
                                  ToStr(g_stats.maxX) + "  Y=" + ToStr(g_stats.minY) + ".." +
                                  ToStr(g_stats.maxY));
        } else {
            DrawStatsLine(hdc, 7, "Bounding Box:           none (no edge pixels)");
        }
    } else {
        DrawStatsLine(hdc, 3, "Edge Pixels:            -");
        DrawStatsLine(hdc, 4, "Edge Percentage:        -");
        DrawStatsLine(hdc, 5, "Average Edge Strength:  -");
        DrawStatsLine(hdc, 6, "Processing Time:        -");
        DrawStatsLine(hdc, 7, "Bounding Box:           -");
    }
    DrawStatsLine(hdc, 8, "Status:                 " + g_status);
}

// Reads the threshold text box. Returns false if it is not an integer in 0..MAX.
bool ReadThreshold(int& threshold) {
    char buf[32];
    GetWindowTextA(g_hEdit, buf, sizeof(buf));
    char* end = 0;
    const long v = std::strtol(buf, &end, 10);
    if (end == buf || *end != '\0' || v < 0 || v > MAX_SOBEL_MAGNITUDE) return false;
    threshold = static_cast<int>(v);
    return true;
}

// ------------------------------------------------------------ actions -------
void OnLoad(HWND hwnd) {
    char file[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFilter     = "PPM Images (*.ppm)\0*.ppm\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile       = file;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrInitialDir = "data\\input";
    ofn.lpstrTitle      = "Load P3 (ASCII) PPM image";
    ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameA(&ofn)) return;   // user pressed Cancel

    SetCursor(LoadCursor(NULL, IDC_WAIT));
    RgbImage img;
    std::string err;
    const bool ok = LoadPPM(file, img, err);
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    if (!ok) {
        MessageBoxA(hwnd, err.c_str(), "Could not load image", MB_OK | MB_ICONERROR);
        return;
    }

    g_original = img;
    RgbToGrayscale(g_original, g_gray);
    BuildFromRgb(g_bmpOriginal, g_original);

    // Clear any previous result.
    g_edges = GrayImage();
    g_stats = EdgeStats();
    g_bmpEdges = DisplayBitmap();
    g_hasResult = false;

    g_path = file;
    g_status = "Image loaded. Set the threshold and press Detect Edges.";
    InvalidateRect(hwnd, NULL, FALSE);
}

void OnDetect(HWND hwnd) {
    if (g_original.empty()) {
        MessageBoxA(hwnd, "Please load an image first.", "No image", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int threshold = 0;
    if (!ReadThreshold(threshold)) {
        MessageBoxA(hwnd, "The threshold must be an integer between 0 and 1443.",
                    "Invalid threshold", MB_OK | MB_ICONWARNING);
        return;
    }

    DetectEdges(g_gray, threshold, g_edges, g_stats);
    BuildFromGray(g_bmpEdges, g_edges);
    g_hasResult = true;
    g_status = "Edge detection finished (threshold = " + ToStr(threshold) + ").";
    InvalidateRect(hwnd, NULL, FALSE);
}

void OnSave(HWND hwnd) {
    if (!g_hasResult) {
        MessageBoxA(hwnd, "There is no edge image yet. Press Detect Edges first.",
                    "Nothing to save", MB_OK | MB_ICONINFORMATION);
        return;
    }

    char file[MAX_PATH] = "edges.ppm";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hwnd;
    ofn.lpstrFilter     = "PPM Images (*.ppm)\0*.ppm\0";
    ofn.lpstrFile       = file;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrInitialDir = "data\\output";   // suggested default: data\output\edges.ppm
    ofn.lpstrDefExt     = "ppm";
    ofn.lpstrTitle      = "Save edge image";
    ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetSaveFileNameA(&ofn)) return;

    RgbImage out;
    GrayToRgb(g_edges, out);
    std::string err;
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    const bool ok = SavePPM(file, out, err);
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    if (!ok) {
        MessageBoxA(hwnd, err.c_str(), "Could not save image", MB_OK | MB_ICONERROR);
        return;
    }
    g_status = std::string("Saved: ") + file;
    InvalidateRect(hwnd, NULL, FALSE);
}

// ----------------------------------------------------------- controls -------
HWND MakeControl(HWND parent, const char* cls, const char* text, DWORD style,
                 int x, int y, int w, int h, int id, DWORD exStyle = 0) {
    HWND c = CreateWindowExA(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style,
                             x, y, w, h, parent,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                             GetModuleHandleA(NULL), NULL);
    SendMessageA(c, WM_SETFONT, reinterpret_cast<WPARAM>(g_fontUi), TRUE);
    return c;
}

void CreateControls(HWND hwnd) {
    g_fontUi = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    g_fontTitle = CreateFontA(-26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    g_fontMono = CreateFontA(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    MakeControl(hwnd, "STATIC", "Threshold:", SS_LEFT, 30, 647, 75, 20, 0);
    g_hEdit = MakeControl(hwnd, "EDIT", "100", ES_NUMBER | ES_CENTER | WS_TABSTOP,
                          108, 642, 70, 26, ID_EDIT_THRESHOLD, WS_EX_CLIENTEDGE);
    SendMessageA(g_hEdit, EM_LIMITTEXT, 4, 0);
    MakeControl(hwnd, "STATIC", "(0 - 1443)", SS_LEFT, 186, 647, 80, 20, 0);

    MakeControl(hwnd, "BUTTON", "Load Image",   BS_PUSHBUTTON | WS_TABSTOP, 290, 638, 140, 34, ID_BTN_LOAD);
    MakeControl(hwnd, "BUTTON", "Detect Edges", BS_PUSHBUTTON | WS_TABSTOP, 445, 638, 140, 34, ID_BTN_DETECT);
    MakeControl(hwnd, "BUTTON", "Save Result",  BS_PUSHBUTTON | WS_TABSTOP, 600, 638, 140, 34, ID_BTN_SAVE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateControls(hwnd);
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam)) {
            case ID_BTN_LOAD:   OnLoad(hwnd);   break;
            case ID_BTN_DETECT: OnDetect(hwnd); break;
            case ID_BTN_SAVE:   OnSave(hwnd);   break;
            }
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;   // we repaint everything in WM_PAINT (avoids flicker)

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Double buffering: draw into memory, then copy to the window once.
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP buffer = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HGDIOBJ old = SelectObject(mem, buffer);
        PaintAll(mem, rc);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(buffer);
        DeleteDC(mem);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        DeleteObject(g_fontTitle);
        DeleteObject(g_fontMono);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

} // namespace

int RunGui(HINSTANCE hInstance, int nCmdShow) {
    const char* className = "EdgeDetectorWindow";

    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = className;
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Window class registration failed.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Fixed-size window: ask Windows how big the frame must be for our client area.
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    RECT r = { 0, 0, CLIENT_W, CLIENT_H };
    AdjustWindowRect(&r, style, FALSE);

    HWND hwnd = CreateWindowExA(0, className, "C++ Image Edge Detector", style,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                r.right - r.left, r.bottom - r.top,
                                NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBoxA(NULL, "Window creation failed.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return static_cast<int>(msg.wParam);
}
