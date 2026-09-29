// main.cpp
// Entry point.
//   edge_detector.exe          -> opens the GUI
//   edge_detector.exe --test   -> runs the built-in self tests in the console

#include <cstdio>
#include <cstring>

#include "self_test.h"

#ifdef _WIN32
#include <windows.h>
#include "gui.h"
#endif

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--test") == 0) {
            return RunSelfTests();
        }
    }

#ifdef _WIN32
    return RunGui(GetModuleHandleA(NULL), SW_SHOWDEFAULT);
#else
    std::printf("The GUI is only available on Windows. Use --test.\n");
    return 0;
#endif
}
