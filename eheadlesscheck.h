#ifndef EHEADLESSCHECK_H
#define EHEADLESSCHECK_H

#include <cstddef>

// The safety net (developer build only; see mod-docs/safety-net.md).
//
// Runs an adventure with no window and nobody playing: it loads the
// adventure's first city, builds a small fixed test city, lets the game
// run for a number of years and writes down what the city looks like at
// the end. With the same seed the result is the same every time, so a
// change in the result means the game now behaves differently.
//
//   eZeus-check.exe --headless-check <adventure> <years>
//                   [--seed N] [--out file] [--no-city]
//                   [--reload-days N] [--threads]
//
// <adventure> is a folder in eZeus's own Adventures folder, or the name
// of a .pak file in the original game's Adventures folder.
//
// For hunting down why two runs differ (see mod-docs/safety-net.md):
//   --trace N, --trace-keep A B, --trace-steps A B, --fill N,
//   --rand-log file, and
//   eZeus-check.exe --headless-check --rand-diff fileA fileB

// true when the command line asks for a headless check
bool eHeadlessCheckRequested(const int argc, char** const argv);
// runs it; the return value is the program's exit code
int eHeadlessCheckRun(const int argc, char** const argv);

// eheadlessrandlog.cpp
// where the program sits in memory (changes from run to run)
void* eHeadlessProgramStart();
// compares two --rand-log files and prints the first place where the two
// runs asked for a random number from different code
int eHeadlessRandDiff(const char* const a, const char* const b);
// --save-who: prints the code path that writes the given bytes of the
// next save; eHeadlessSaveWatch is what eWriteTarget::sWatch is set to
void eHeadlessSaveWatchSet(const long long* const offsets, const int n);
void eHeadlessSaveWatch(const size_t len);

#endif // EHEADLESSCHECK_H
