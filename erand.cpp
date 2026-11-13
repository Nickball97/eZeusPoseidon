#include "erand.h"

#include "elimits.h"

#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

std::random_device eRand::sDev;
std::mt19937 eRand::sRng(sDev());
std::uniform_int_distribution<int> eRand::sDist(0, __INT_MAX__);

namespace {
// see eRand::setLog
std::FILE* gLog = nullptr;
std::uintptr_t gLogBase = 0;

inline void logCaller(const void* const returnAddress) {
    const auto ra = reinterpret_cast<std::uintptr_t>(returnAddress);
    const auto v = static_cast<std::uint32_t>(ra - gLogBase);
    std::fwrite(&v, sizeof(v), 1, gLog);
}
}

void eRand::seed(const unsigned int s) {
    sRng.seed(s);
    sDist.reset();
}

void eRand::setLog(std::FILE* const file, const void* const programStart) {
    gLog = file;
    gLogBase = reinterpret_cast<std::uintptr_t>(programStart);
}

void eRand::logMark(const unsigned int mark) {
    if(!gLog) return;
    const std::uint32_t v[2] = {0xFFFFFFFFu, mark};
    std::fwrite(v, sizeof(v), 1, gLog);
}

void eRand::logShuffle() {
    if(!gLog) return;
#ifdef _MSC_VER
    logCaller(_ReturnAddress());
#else
    logCaller(__builtin_return_address(0));
#endif
}

int eRand::rand() {
    if(gLog) {
#ifdef _MSC_VER
        logCaller(_ReturnAddress());
#else
        logCaller(__builtin_return_address(0));
#endif
    }
    return sDist(sRng);
}
