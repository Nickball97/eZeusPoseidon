#ifndef EHOUSEMODEL_H
#define EHOUSEMODEL_H

#include "engine/edifficulty.h"

// Original Zeus/Poseidon house data: the "ALL HOUSES" rows of
// Model\Zeus_Model_{VeryEasy,Easy,Normal,Hard,Impossible}.txt
// (= Beginner, Mortal, Hero, Titan, Olympian). The Citadel of Poseidon's
// housing_*.html tables are copies of these rows.
// Common levels: 0 hut, 1 shack, 2 hovel, 3 homestead, 4 tenement,
// 5 apartment, 6 townhouse. Elite levels: 0 abandoned, 1 residence,
// 2 mansion, 3 manor, 4 estate.
namespace eHouseModel {
    inline int diffId(const eDifficulty d) { return static_cast<int>(d); }

    // Appeal needed to reach level L (column b of the row below L).
    inline int smallEvolve(const eDifficulty d, const int l) {
        static const int s[5][7] = {
            {-99, -10, 0, 10, 10, 20, 30},
            {-99, -10, 0, 10, 10, 20, 30},
            {-99, -10, 0, 10, 10, 20, 30},
            {-99, -10, 0, 10, 10, 20, 40},
            {-99, 0, 10, 10, 20, 30, 40}};
        return s[diffId(d)][l];
    }
    // Appeal below which a house at level L devolves (column a).
    inline int smallDevolve(const eDifficulty d, const int l) {
        static const int s[5][7] = {
            {-99, -12, -2, -2, 5, 15, 25},
            {-99, -12, -2, -2, 5, 15, 25},
            {-99, -12, -2, -2, 5, 15, 25},
            {-99, -12, -2, -2, 5, 15, 35},
            {-99, -2, -2, 5, 15, 25, 35}};
        return s[diffId(d)][l];
    }
    inline int eliteEvolve(const eDifficulty d, const int l) {
        static const int s[5][5] = {
            {-99, 40, 50, 60, 70},
            {-99, 46, 56, 66, 76},
            {-99, 50, 60, 70, 80},
            {-99, 50, 60, 70, 80},
            {-99, 55, 65, 75, 85}};
        return s[diffId(d)][l];
    }
    inline int eliteDevolve(const eDifficulty d, const int l) {
        static const int s[5][5] = {
            {-99, 36, 46, 56, 66},
            {-99, 42, 52, 62, 72},
            {-99, 46, 56, 66, 76},
            {-99, 46, 56, 66, 76},
            {-99, 50, 60, 70, 80}};
        return s[diffId(d)][l];
    }

    // Culture/science points needed to be at level L (column c).
    inline int smallCulture(const int l) {
        static const int s[7] = {0, 0, 15, 15, 35, 35, 45};
        return s[l];
    }
    inline int eliteCulture(const eDifficulty d, const int l) {
        if(d == eDifficulty::beginner) {
            static const int s[5] = {0, 40, 50, 60, 70};
            return s[l];
        }
        static const int s[5] = {0, 50, 60, 70, 80};
        return s[l];
    }

    // Crime risk increment (column o). Elite houses: -20 (no crime).
    inline int smallCrime(const eDifficulty d, const int l) {
        static const int s[5][7] = {
            {5, 5, 3, 3, 0, 0, 0},
            {10, 10, 5, 5, 0, 0, 0},
            {15, 15, 10, 10, 5, 5, 0},
            {20, 20, 15, 15, 10, 10, 5},
            {25, 20, 20, 15, 15, 10, 10}};
        return s[diffId(d)][l];
    }
    // Crime risk base (column p): a house's crime never drops below it.
    inline int smallCrimeBase(const eDifficulty d, const int l) {
        static const int s[5][7] = {
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {5, 5, 0, 0, 0, 0, 0},
            {10, 5, 5, 0, 0, 0, 0},
            {10, 10, 10, 5, 5, 0, 0}};
        return s[diffId(d)][l];
    }
    // Disease risk increment (column u).
    inline int smallDisease(const eDifficulty d, const int l) {
        static const int s[5][7] = {
            {20, 15, 15, 10, 10, 5, 5},
            {20, 15, 15, 10, 10, 5, 5},
            {20, 15, 15, 10, 10, 5, 5},
            {25, 25, 25, 20, 20, 15, 15},
            {30, 25, 25, 20, 20, 15, 15}};
        return s[diffId(d)][l];
    }
}

#endif // EHOUSEMODEL_H
