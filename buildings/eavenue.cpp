#include "eavenue.h"

#include "textures/egametextures.h"
#include "engine/egameboard.h"

#include <vector>

eAvenue::eAvenue(eGameBoard& board, const eCityId cid,
                 const eBuildingType type) :
    eBuilding(board, type, 1, 1, cid) {
    eGameTextures::loadAvenue();
}

bool eAvenue::sAvenueType(const eBuildingType t) {
    return t == eBuildingType::avenue || t == eBuildingType::boulevard;
}

int eAvenue::provide(const eProvide p, const int n) {
    const auto t = centerTile();
    if(!t) return eBuilding::provide(p, n);
    int rem = n;
    for(const int x : {-1, 0, 1}) {
        for(const int y : {-1, 0, 1}) {
            const auto tt = t->tileRel<eTile>(x, y);
            if(!tt) continue;
            if(const auto b = tt->underBuilding()) {
                const auto tp = b->type();
                if(sAvenueType(tp)) continue;
                const int r = b->provide(p, rem);
                rem -= r;
                if(rem <= 0) return n;
            }
        }
    }
    return n - rem;
}

namespace {
    // zeus.exe table 0x005E92B8, tried first to last (0x004A6920). For
    // each neighbour, in the order 0x00419190 reads them (y-1, x+1 y-1,
    // x+1, x+1 y+1, y+1, x-1 y+1, x-1, x-1 y-1; in eZeus's default view:
    // top right, right, bottom right, bottom, bottom left, left, top left,
    // top): 1 = road or Avenue/Boulevard there, 0 = neither, 2 = either.
    // Then the SPR_BOUL_ROAD picture for the default view. Rows 8 and 9
    // are the same in the original, so left + right corners use row 46.
    struct eAvenueRow {
        char fN[8];
        int fImg;
    };

    const eAvenueRow gAvenueRows[] = {
        {{1, 0, 1, 0, 1, 0, 1, 0}, 0},
        {{1, 1, 1, 0, 1, 0, 1, 0}, 1},
        {{1, 0, 1, 1, 1, 0, 1, 0}, 2},
        {{1, 0, 1, 0, 1, 1, 1, 0}, 3},
        {{1, 0, 1, 0, 1, 0, 1, 1}, 4},
        {{1, 1, 1, 1, 1, 0, 1, 0}, 7},
        {{1, 0, 1, 1, 1, 1, 1, 0}, 8},
        {{1, 0, 1, 0, 1, 1, 1, 1}, 5},
        {{1, 1, 1, 0, 1, 0, 1, 1}, 6},
        {{1, 1, 1, 0, 1, 0, 1, 1}, 9},
        {{1, 0, 1, 1, 1, 0, 1, 1}, 10},
        {{1, 1, 1, 1, 1, 1, 1, 0}, 14},
        {{1, 0, 1, 1, 1, 1, 1, 1}, 12},
        {{1, 1, 1, 0, 1, 1, 1, 1}, 11},
        {{1, 1, 1, 1, 1, 0, 1, 1}, 13},
        {{1, 1, 1, 1, 1, 1, 1, 1}, 15},
        {{0, 2, 1, 1, 1, 1, 1, 2}, 23},
        {{0, 2, 1, 0, 1, 0, 1, 2}, 26},
        {{0, 2, 1, 0, 1, 1, 1, 2}, 24},
        {{0, 2, 1, 1, 1, 0, 1, 2}, 25},
        {{1, 2, 0, 2, 1, 1, 1, 1}, 27},
        {{1, 2, 0, 2, 1, 0, 1, 0}, 30},
        {{1, 2, 0, 2, 1, 0, 1, 1}, 28},
        {{1, 2, 0, 2, 1, 1, 1, 0}, 29},
        {{1, 1, 1, 2, 0, 2, 1, 1}, 31},
        {{1, 0, 1, 2, 0, 2, 1, 0}, 34},
        {{1, 1, 1, 2, 0, 2, 1, 0}, 32},
        {{1, 0, 1, 2, 0, 2, 1, 1}, 33},
        {{1, 1, 1, 1, 1, 2, 0, 2}, 19},
        {{1, 0, 1, 0, 1, 2, 0, 2}, 22},
        {{1, 0, 1, 1, 1, 2, 0, 2}, 20},
        {{1, 1, 1, 0, 1, 2, 0, 2}, 21},
        {{0, 2, 0, 2, 1, 1, 1, 2}, 39},
        {{0, 2, 0, 2, 1, 0, 1, 2}, 40},
        {{1, 2, 0, 2, 0, 2, 1, 1}, 41},
        {{1, 2, 0, 2, 0, 2, 1, 0}, 42},
        {{1, 1, 1, 2, 0, 2, 0, 2}, 35},
        {{1, 0, 1, 2, 0, 2, 0, 2}, 36},
        {{0, 2, 1, 1, 1, 2, 0, 2}, 37},
        {{0, 2, 1, 0, 1, 2, 0, 2}, 38},
        {{1, 2, 0, 2, 1, 2, 0, 2}, 43},
        {{0, 2, 1, 2, 0, 2, 1, 2}, 44},
        {{0, 2, 0, 2, 0, 2, 1, 2}, 48},
        {{1, 2, 0, 2, 0, 2, 0, 2}, 45},
        {{0, 2, 1, 2, 0, 2, 0, 2}, 46},
        {{0, 2, 0, 2, 1, 2, 0, 2}, 47},
        {{2, 2, 2, 2, 2, 2, 2, 2}, 15}
    };

    bool connects(eTile* const t) {
        return t && (t->hasRoad() || t->hasAvenue());
    }

    // the matched row's picture, without the variant
    int matchImage(eTile* const t, const eWorldDirection dir) {
        const bool n[8] = {
            connects(t->topRightRotated<eTile>(dir)),
            connects(t->rightRotated<eTile>(dir)),
            connects(t->bottomRightRotated<eTile>(dir)),
            connects(t->bottomRotated<eTile>(dir)),
            connects(t->bottomLeftRotated<eTile>(dir)),
            connects(t->leftRotated<eTile>(dir)),
            connects(t->topLeftRotated<eTile>(dir)),
            connects(t->topRotated<eTile>(dir))
        };
        for(const auto& row : gAvenueRows) {
            bool fits = true;
            for(int i = 0; i < 8 && fits; i++) {
                const char r = row.fN[i];
                if(r == 2) continue;
                fits = n[i] == (r == 1);
            }
            if(fits) return row.fImg;
        }
        return 15;
    }

    // 0x00418A20: the tree picture for a matched picture, -1 for none
    int treeArt(const int img) {
        switch(img) {
        case 31: return 0;
        case 23: return 9;
        case 27: return 18;
        case 19: return 27;
        case 11: return 36;
        case 12: return 37;
        case 13: return 38;
        case 14: return 39;
        case 35: return 40;
        case 37: return 41;
        case 39: return 42;
        case 41: return 43;
        default: return -1;
        }
    }

    // 0x004193A0: a tree tile with road on two opposite sides shows road
    bool betweenRoads(eTile* const t) {
        const auto tl = t->topLeft<eTile>();
        const auto br = t->bottomRight<eTile>();
        const auto tr = t->topRight<eTile>();
        const auto bl = t->bottomLeft<eTile>();
        const bool x = tl && tl->hasRoad() && br && br->hasRoad();
        const bool y = tr && tr->hasRoad() && bl && bl->hasRoad();
        return x || y;
    }

    // an Avenue/Boulevard tile's tree picture before the variant,
    // -1 if there is none or it shows road
    int baseArt(eTile* const t, const eWorldDirection dir) {
        if(!t || !t->hasAvenue()) return -1;
        const int art = treeArt(matchImage(t, dir));
        if(art == -1 || betweenRoads(t)) return -1;
        return art;
    }

    int seedByte(eTile* const t) {
        const auto b = t->underBuilding();
        return b ? (b->seed() & 0xff) : 0;
    }

    // 0x00418CF0 in the default view, for one tile of a straight side.
    // Variants 0, 1, 2 and 8 stand alone; 5 starts a longer stretch of
    // trees, 4, 6 and 7 continue it and 3 ends it.
    int sideVariant(const int r, const bool prevSame,
                    const int prevVar, const bool nextSame) {
        const int alone[] = {0, 1, 2, 8};
        const int one = alone[r & 3];
        if(prevSame && prevVar >= 4 && prevVar <= 7) {
            if(!nextSame) return 3;
            switch(r & 7) {
            case 0: return 4;
            case 1:
            case 7: return 6;
            case 2: return 7;
            default: return 3;
            }
        }
        if(!nextSame) return one;
        if(prevSame) {
            const int u = r % 9;
            if(u < 3 || u > 7) return one;
        }
        return 5;
    }
}

int eAvenue::sRoadImage(eTile* const t, const eWorldDirection dir,
                        const int seed) {
    if(!t) return 15;
    const int img = matchImage(t, dir);
    if(img == 15) return img + (seed & 3);
    return img;
}

int eAvenue::pickArt(const eWorldDirection dir) const {
    const auto t = centerTile();
    if(!t) return 100 + 15;
    const int img = matchImage(t, dir);
    int art = treeArt(img);
    if(art != -1 && betweenRoads(t)) art = -1;
    if(art == -1) {
        const int road = img == 15 ? img + (seed() & 3) : img;
        return 100 + road;
    }
    if(art >= 36) return art;
    // a straight side: the original scans the map from low x and y to
    // high, so the tile before this one is at x - 1 or y - 1
    const bool alongX = art == 0 || art == 9;
    const auto prevTile = [&](eTile* const c) {
        return alongX ? c->topLeftRotated<eTile>(dir) :
                        c->topRightRotated<eTile>(dir);
    };
    const auto nextTile = [&](eTile* const c) {
        return alongX ? c->bottomRightRotated<eTile>(dir) :
                        c->bottomLeftRotated<eTile>(dir);
    };
    std::vector<eTile*> chain{t};
    for(auto p = prevTile(t); p && chain.size() < 512 &&
        baseArt(p, dir) == art; p = prevTile(p)) {
        chain.push_back(p);
    }
    int var = -1;
    bool prevSame = false;
    for(int i = static_cast<int>(chain.size()) - 1; i >= 0; i--) {
        const auto c = chain[i];
        const bool nextSame = i > 0 || baseArt(nextTile(c), dir) == art;
        const int r = i == 0 ? (seed() & 0xff) : seedByte(c);
        var = sideVariant(r, prevSame, var, nextSame);
        prevSame = true;
    }
    return art + var;
}

std::shared_ptr<eTexture>
eAvenue::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings()[sizeId];
    const auto dir = getBoard().direction();
    const int dirId = static_cast<int>(dir);
    if(mArtRevision != eTile::sBuildingRevision || mArtDir != dirId) {
        mArt = pickArt(dir);
        mArtRevision = eTile::sBuildingRevision;
        mArtDir = dirId;
    }
    if(mArt >= 100) return blds.fAvenueRoad.getTexture(mArt - 100);
    if(mArt >= 36) return blds.fAvenue[4].getTexture(mArt - 36);
    return blds.fAvenue[mArt / 9].getTexture(mArt % 9);
}

eTile* eAvenue::road() const {
    const auto t = centerTile();
    const auto tl = t->topLeft<eTile>();
    if(tl && tl->hasRoad()) return tl;
    const auto br = t->bottomRight<eTile>();
    if(br && br->hasRoad()) return br;
    const auto bl = t->bottomLeft<eTile>();
    if(bl && bl->hasRoad()) return bl;
    const auto tr = t->topRight<eTile>();
    if(tr && tr->hasRoad()) return tr;
    return nullptr;
}
