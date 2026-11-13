#ifndef ERAND_H
#define ERAND_H

#include <random>
#include <algorithm>
#include <cstdio>

class eRand {
public:
    static int rand();
    // Restarts the sequence of random numbers from a fixed point, so a
    // run can be repeated exactly. Only the headless check calls it.
    static void seed(const unsigned int s);

    // Headless check, for hunting down why two runs differ: from now on
    // note in the file which place in the program asks for each random
    // number (as its distance from programStart). Two runs that behave the
    // same write the same file. nullptr turns it off; the game never
    // turns it on.
    static void setLog(std::FILE* const file, const void* const programStart);
    // a number of your own in the log, to tell where in the run it is
    static void logMark(const unsigned int mark);

    template <typename T>
    static void randomShuffle(std::vector<T>& vec);
private:
    static void logShuffle();

    static std::random_device sDev;
    static std::mt19937 sRng;
    static std::uniform_int_distribution<int> sDist;
};

template<typename T>
inline void eRand::randomShuffle(std::vector<T> &vec) {
    logShuffle();
    std::shuffle(vec.begin(), vec.end(), sRng);
}

#endif // ERAND_H
