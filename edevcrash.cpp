// Developer test builds only (EZEUS_DEV_CHEATS): on a crash, print the call
// stack with function names and source lines to stdout, so a crash seen
// while play-testing can be located. Needs the build's .pdb next to it.
#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>

#include <cstdio>

namespace {
LONG WINAPI eDevCrashHandler(EXCEPTION_POINTERS* const ep) {
    const auto rec = ep->ExceptionRecord;
    printf("\n=== CRASH: exception 0x%08lx at %p ===\n",
           rec->ExceptionCode, rec->ExceptionAddress);

    const auto proc = GetCurrentProcess();
    const auto thread = GetCurrentThread();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(proc, nullptr, TRUE);

    CONTEXT ctx = *ep->ContextRecord;
    STACKFRAME64 frame{};
    frame.AddrPC.Offset = ctx.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;

    alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + 256];
    const auto sym = reinterpret_cast<SYMBOL_INFO*>(buf);

    for(int i = 0; i < 40; i++) {
        if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64, proc, thread, &frame,
                        &ctx, nullptr, SymFunctionTableAccess64,
                        SymGetModuleBase64, nullptr)) break;
        const DWORD64 addr = frame.AddrPC.Offset;
        if(addr == 0) break;

        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;
        DWORD64 symOff = 0;
        const bool hasSym = SymFromAddr(proc, addr, &symOff, sym);

        IMAGEHLP_LINE64 line{};
        line.SizeOfStruct = sizeof(line);
        DWORD lineOff = 0;
        const bool hasLine = SymGetLineFromAddr64(proc, addr, &lineOff, &line);

        char module[MAX_PATH] = "?";
        const auto base = SymGetModuleBase64(proc, addr);
        if(base) {
            GetModuleFileNameA(reinterpret_cast<HMODULE>(base),
                               module, MAX_PATH);
        }
        const char* modName = module;
        for(const char* c = module; *c; c++) {
            if(*c == '\\' || *c == '/') modName = c + 1;
        }

        printf("#%02d %s!%s", i, modName, hasSym ? sym->Name : "?");
        if(hasLine) printf("  (%s:%lu)", line.FileName, line.LineNumber);
        printf("\n");
    }
    printf("=== END CRASH ===\n");
    fflush(stdout);
    return EXCEPTION_CONTINUE_SEARCH;
}
}

void eInstallDevCrashHandler() {
    // write output immediately so the log ends where the game stopped
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetUnhandledExceptionFilter(eDevCrashHandler);
}

#else

void eInstallDevCrashHandler() {}

#endif
