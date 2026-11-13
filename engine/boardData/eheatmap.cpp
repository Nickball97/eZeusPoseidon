#include "eheatmap.h"

#include "etilehelper.h"

#include <algorithm>
#include <cassert>
#include <cmath>

void eHeatMap::add(const eHeatMap& other) {
    assert(mDX == other.mDX);
    assert(mDY == other.mDY);
    assert(mWidth == other.mWidth);
    assert(mHeight == other.mHeight);
    for(int x = 0; x < mWidth; x++) {
        auto& tRow = mMap[x];
        const auto& oRow = other.mMap[x];
        for(int y = 0; y < mHeight; y++) {
            auto& tCell = tRow[y];
            const auto& oCell = oRow[y];

            if(tCell.fEnabled || oCell.fEnabled) {
                tCell.fEnabled = true;
                tCell.fAppeal += oCell.fAppeal;
            } else {
                tCell.fEnabled = false;
                tCell.fAppeal = 0;
            }
        }
    }
}

void eHeatMap::multiply(const eHeatMap& other) {
    assert(mDX == other.mDX);
    assert(mDY == other.mDY);
    assert(mWidth == other.mWidth);
    assert(mHeight == other.mHeight);
    for(int x = 0; x < mWidth; x++) {
        auto& tRow = mMap[x];
        const auto& oRow = other.mMap[x];
        for(int y = 0; y < mHeight; y++) {
            auto& tCell = tRow[y];
            const auto& oCell = oRow[y];

            if(tCell.fEnabled && oCell.fEnabled) {
                tCell.fEnabled = true;
                tCell.fAppeal *= oCell.fAppeal;
            } else {
                tCell.fEnabled = false;
                tCell.fAppeal = 0;
            }
        }
    }
}

void eHeatMap::reset() {
    initialize(mDX, mDY, mWidth, mHeight);
}

void eHeatMap::initialize(const int dx, const int dy,
                          const int w, const int h) {
    mMap.clear();
    mDX = dx;
    mDY = dy;
    mWidth = w;
    mHeight = h;
    if(w > 0) mMap.reserve(w);
    for(int i = 0; i < w; i++) {
        auto& row = mMap.emplace_back();
        if(h > 0) row.reserve(h);
        for(int j = 0; j < h; j++) {
            row.emplace_back();
        }
    }
}

void eHeatMap::addHeat(const eHeat& a,
                       const SDL_Rect& tileRect) {
    addHeat(a, tileRect.x, tileRect.y,
               tileRect.w, tileRect.h);
}

void eHeatMap::addHeat(const eHeat& a,
                       const int ax, const int ay,
                       const int sw, const int sh) {
    const int r = a.fRange;
    if(r <= 0) return;
    const double v = a.fValue;
    if(a.fStep > 0) {
        // Original rule (exe 0x004A4BF0/0x004A4C60, documented "verified"
        // in zeus-poseidon-arm64 docs/engine/terrain.md 7.1): ring r = 1 ..
        // min(range, 6) around the footprint (square rings) gets the
        // current value, which changes by fStepSize after every fStep
        // rings. The footprint itself gets nothing, and every tile stays
        // within -100..100 after each addition.
        const int rr = std::min(r, 6);
        for(int x = ax - rr; x < ax + sw + rr; x++) {
            for(int y = ay - rr; y < ay + sh + rr; y++) {
                const int ddx = x < ax ? ax - x :
                                (x >= ax + sw ? x - (ax + sw - 1) : 0);
                const int ddy = y < ay ? ay - y :
                                (y >= ay + sh ? y - (ay + sh - 1) : 0);
                const int dist = std::max(ddx, ddy);
                if(dist == 0 || dist > rr) continue;
                const int steps = (dist - 1)/a.fStep;
                const double val = v + steps*a.fStepSize;
                int dtx;
                int dty;
                eTileHelper::tileIdToDTileId(x, y, dtx, dty);
                addHeat(dtx, dty, 2*val); // addHeat() halves its input
                if(dtx >= mDX && dty >= mDY &&
                   dtx < mWidth + mDX && dty < mHeight + mDY) {
                    auto& tile = mMap[dtx - mDX][dty - mDY];
                    tile.fAppeal = std::clamp(tile.fAppeal, -100., 100.);
                }
            }
        }
        return;
    }
    for(int x = ax - r; x <= ax + sw + r; x++) {
        for(int y = ay - r; y <= ay + sh + r; y++) {
            const double cx = ax + 0.5*sw;
            const double cy = ay + 0.5*sh;
            const double dx = std::max(abs(x - cx) - 0.5*sw, 0.);
            const double dy = std::max(abs(y - cy) - 0.5*sh, 0.);
            const double dist = std::sqrt(dx * dx + dy * dy);
            if(dist > r) continue;
            const double mult = (r - dist)/r;
            int dtx;
            int dty;
            eTileHelper::tileIdToDTileId(x, y, dtx, dty);
            addHeat(dtx, dty, mult*v);
        }
    }
}

void eHeatMap::addHeat(const int x, const int y,
                       const double a) {
    if(x < mDX || y < mDY ||
       x >= mWidth + mDX || y >= mHeight + mDY) return;
    auto& tile = mMap[x - mDX][y - mDY];
    tile.fEnabled = true;
    tile.fAppeal += a/2;
}

void eHeatMap::setOutsideRange(const int x, const int y) {
    if(x < mDX || y < mDY ||
       x >= mWidth + mDX || y >= mHeight + mDY) return;
    auto& tile = mMap[x - mDX][y - mDY];
    tile.fOutsideRange = true;
}

bool eHeatMap::enabled(const int x, const int y) const {
    const auto& t = mMap[x - mDX][y - mDY];
    return t.fEnabled && !t.fOutsideRange;
}

double eHeatMap::heat(const int x, const int y) const {
    return mMap[x - mDX][y - mDY].fAppeal;
}

void eHeatMap::set(const int x, const int y,
                   const bool e, const double h) {
    auto& tile = mMap[x - mDX][y - mDY];
    tile.fEnabled = e;
    tile.fAppeal = h;
}
