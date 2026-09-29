// gui.h
// Win32 + GDI front end. Only compiled on Windows.

#ifndef GUI_H
#define GUI_H

#include <windows.h>

// Creates the main window and runs the message loop until it is closed.
// Returns the process exit code.
int RunGui(HINSTANCE hInstance, int nCmdShow);

#endif // GUI_H
