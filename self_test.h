// self_test.h
// Tiny built-in test runner (no external framework). Run with: program.exe --test

#ifndef SELF_TEST_H
#define SELF_TEST_H

// Returns 0 if every check passed, 1 otherwise (usable as a process exit code).
int RunSelfTests();

#endif // SELF_TEST_H
