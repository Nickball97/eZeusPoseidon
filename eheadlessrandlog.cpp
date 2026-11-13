// Part of the safety net (developer build only): reading the logs written
// with --rand-log. Kept apart from eheadlesscheck.cpp because it needs
// windows.h, whose names clash with the game's.
#include "eheadlesscheck.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>

void* eHeadlessProgramStart() {
    return GetModuleHandleA(nullptr);
}

namespace {

bool readLog(const char* const path, std::vector<std::uint32_t>& log) {
    const auto f = std::fopen(path, "rb");
    if(!f) return false;
    std::uint32_t buf[4096];
    for(;;) {
        const auto n = std::fread(buf, sizeof(buf[0]), 4096, f);
        if(n == 0) break;
        log.insert(log.end(), buf, buf + n);
    }
    std::fclose(f);
    return true;
}

// the function and source line a logged place belongs to
std::string place(const std::uint32_t rva) {
    const auto proc = GetCurrentProcess();
    const auto base = reinterpret_cast<DWORD64>(GetModuleHandleA(nullptr));
    const DWORD64 addr = base + rva;

    alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + 256];
    const auto sym = reinterpret_cast<SYMBOL_INFO*>(buf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = 255;
    DWORD64 symOff = 0;
    const bool hasSym = SymFromAddr(proc, addr, &symOff, sym);

    IMAGEHLP_LINE64 line{};
    line.SizeOfStruct = sizeof(line);
    DWORD lineOff = 0;
    // the logged address is just after the call
    const bool hasLine = SymGetLineFromAddr64(proc, addr - 1, &lineOff, &line);

    std::string r = hasSym ? sym->Name : "?";
    if(hasLine) {
        const char* name = line.FileName;
        for(const char* c = line.FileName; *c; c++) {
            if(*c == '\\' || *c == '/') name = c + 1;
        }
        r += "  (" + std::string(name) + ":" +
             std::to_string(line.LineNumber) + ")";
    }
    return r;
}

void printEntries(const std::vector<std::uint32_t>& log,
                  const size_t from, const size_t to) {
    for(size_t i = from; i < to && i < log.size(); i++) {
        const auto v = log[i];
        if(v == 0xFFFFFFFFu && i + 1 < log.size()) {
            std::printf("    -- step %u --\n", log[i + 1]);
            i++;
            continue;
        }
        std::printf("    %s\n", place(v).c_str());
    }
}

}

int eHeadlessRandDiff(const char* const a, const char* const b) {
    std::vector<std::uint32_t> la;
    std::vector<std::uint32_t> lb;
    if(!readLog(a, la) || !readLog(b, lb)) {
        std::printf("Could not read the logs.\n");
        return 2;
    }
    size_t i = 0;
    const size_t n = la.size() < lb.size() ? la.size() : lb.size();
    while(i < n && la[i] == lb[i]) i++;
    if(i == n && la.size() == lb.size()) {
        std::printf("The two logs are the same (%zu entries).\n", n);
        return 0;
    }
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    // the step the difference falls in
    std::uint32_t step = 0;
    for(size_t k = i; k > 0; k--) {
        if(la[k - 1] == 0xFFFFFFFFu && k < la.size()) {
            step = la[k];
            break;
        }
    }
    std::printf("First difference at entry %zu of %zu / %zu, in step %u "
                "(day %u).\n", i, la.size(), lb.size(), step, step*100/350);
    const size_t from = i > 12 ? i - 12 : 0;
    std::printf("  the same in both, just before:\n");
    printEntries(la, from, i);
    std::printf("  then, first run:\n");
    printEntries(la, i, i + 8);
    std::printf("  then, second run:\n");
    printEntries(lb, i, i + 8);
    return 1;
}

#else

void* eHeadlessProgramStart() { return nullptr; }

int eHeadlessRandDiff(const char* const, const char* const) {
    std::printf("Only available on Windows.\n");
    return 2;
}

#endif

// ---- which code writes a given byte of a save (--save-who) ----

namespace {
std::vector<long long> gWatchOffsets;
long long gWatchPos = 0;
}

void eHeadlessSaveWatchSet(const long long* const offsets, const int n) {
    gWatchOffsets.assign(offsets, offsets + n);
    gWatchPos = 0;
}

#ifdef _WIN32

void eHeadlessSaveWatch(const size_t len) {
    const long long from = gWatchPos;
    const long long to = gWatchPos + static_cast<long long>(len);
    gWatchPos = to;
    for(const auto o : gWatchOffsets) {
        if(o < from || o >= to) continue;
        static bool init = false;
        if(!init) {
            init = true;
            SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME |
                          SYMOPT_DEFERRED_LOADS);
            SymInitialize(GetCurrentProcess(), nullptr, TRUE);
        }
        std::printf("save byte %lld (in a piece of %zu bytes at %lld) "
                    "is written by:\n", o, len, from);
        void* frames[14];
        const auto n = CaptureStackBackTrace(1, 14, frames, nullptr);
        const auto base = reinterpret_cast<DWORD64>(GetModuleHandleA(nullptr));
        for(int i = 0; i < n; i++) {
            const auto addr = reinterpret_cast<DWORD64>(frames[i]);
            if(addr < base) continue;
            const auto p = place(static_cast<std::uint32_t>(addr - base));
            std::printf("    %s\n", p.c_str());
            if(p.find("eHeadlessCheckRun") != std::string::npos) break;
        }
        std::fflush(stdout);
    }
}

#else

void eHeadlessSaveWatch(const size_t len) {
    gWatchPos += static_cast<long long>(len);
}

#endif
