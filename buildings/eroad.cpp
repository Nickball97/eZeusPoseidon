#include "eroad.h"
#include <cmath>

#include "textures/egametextures.h"

#include "engine/egameboard.h"
#include "eagorabase.h"
#include "eavenue.h"
#include "egatehouse.h"
#include "ehippodromepiece.h"
#include "elanguage.h"

namespace {
// The original's road pieces (zeus.exe tables 0x00637098 for plain roads
// and 0x006371F0 for paved and rutted ones, read by 0x004A6920). fN gives
// the 8 tiles around, from the top-right going clockwise (top-right,
// right, bottom-right, bottom, bottom-left, left, top-left, top):
// 1 = a road, 0 = no road, 2 = either. The first piece that fits gives the
// image (as seen facing north; eZeus turns the neighbours instead).
struct eRoadPiece {
    char fN[8];
    int fImg;
};

const eRoadPiece gPlainRoad[] = {
    {{1, 2, 1, 2, 1, 2, 1, 2}, 17},
    {{1, 2, 1, 2, 1, 2, 0, 2}, 13},
    {{0, 2, 1, 2, 1, 2, 1, 2}, 14},
    {{1, 2, 0, 2, 1, 2, 1, 2}, 15},
    {{1, 2, 1, 2, 0, 2, 1, 2}, 16},
    {{1, 2, 1, 2, 0, 2, 0, 2}, 4},
    {{0, 2, 1, 2, 1, 2, 0, 2}, 5},
    {{0, 2, 0, 2, 1, 2, 1, 2}, 6},
    {{1, 2, 0, 2, 0, 2, 1, 2}, 7},
    {{1, 2, 0, 2, 1, 2, 0, 2}, 0},
    {{0, 2, 1, 2, 0, 2, 1, 2}, 1},
    {{1, 2, 0, 2, 0, 2, 0, 2}, 8},
    {{0, 2, 1, 2, 0, 2, 0, 2}, 9},
    {{0, 2, 0, 2, 1, 2, 0, 2}, 10},
    {{0, 2, 0, 2, 0, 2, 1, 2}, 11},
    {{0, 2, 0, 2, 0, 2, 0, 2}, 12},
    {{2, 2, 2, 2, 2, 2, 2, 2}, 12}
};

// paved and rutted roads also have pieces for wide road areas
const eRoadPiece gWideRoad[] = {
    {{1, 0, 1, 0, 1, 0, 1, 0}, 17},
    {{1, 0, 1, 0, 1, 2, 0, 2}, 13},
    {{1, 1, 1, 1, 1, 2, 0, 2}, 18},
    {{1, 0, 1, 1, 1, 2, 0, 2}, 26},
    {{1, 1, 1, 0, 1, 2, 0, 2}, 30},
    {{0, 2, 1, 0, 1, 0, 1, 2}, 14},
    {{0, 2, 1, 1, 1, 1, 1, 2}, 19},
    {{0, 2, 1, 0, 1, 1, 1, 2}, 31},
    {{0, 2, 1, 1, 1, 0, 1, 2}, 27},
    {{1, 2, 0, 2, 1, 0, 1, 0}, 15},
    {{1, 2, 0, 2, 1, 1, 1, 1}, 20},
    {{1, 2, 0, 2, 1, 0, 1, 1}, 32},
    {{1, 2, 0, 2, 1, 1, 1, 0}, 28},
    {{1, 0, 1, 2, 0, 2, 1, 0}, 16},
    {{1, 1, 1, 2, 0, 2, 1, 1}, 21},
    {{1, 1, 1, 2, 0, 2, 1, 0}, 33},
    {{1, 0, 1, 2, 0, 2, 1, 1}, 29},
    {{1, 1, 1, 2, 0, 0, 0, 2}, 22},
    {{0, 2, 1, 1, 1, 2, 0, 0}, 23},
    {{0, 0, 0, 2, 1, 1, 1, 2}, 24},
    {{1, 2, 0, 0, 0, 2, 1, 1}, 25},
    {{1, 0, 1, 0, 1, 1, 1, 1}, 34},
    {{1, 1, 1, 0, 1, 0, 1, 1}, 35},
    {{1, 1, 1, 1, 1, 0, 1, 0}, 36},
    {{1, 0, 1, 1, 1, 1, 1, 0}, 37},
    {{1, 0, 1, 0, 1, 0, 1, 1}, 38},
    {{1, 1, 1, 0, 1, 0, 1, 0}, 39},
    {{1, 0, 1, 1, 1, 0, 1, 0}, 40},
    {{1, 0, 1, 0, 1, 1, 1, 0}, 41},
    {{1, 1, 1, 1, 1, 0, 1, 1}, 42},
    {{1, 1, 1, 1, 1, 1, 1, 0}, 43},
    {{1, 0, 1, 1, 1, 1, 1, 1}, 44},
    {{1, 1, 1, 0, 1, 1, 1, 1}, 45},
    {{1, 1, 1, 0, 1, 1, 1, 0}, 46},
    {{1, 0, 1, 1, 1, 0, 1, 1}, 47},
    {{1, 2, 1, 2, 0, 2, 0, 2}, 4},
    {{0, 2, 1, 2, 1, 2, 0, 2}, 5},
    {{0, 2, 0, 2, 1, 2, 1, 2}, 6},
    {{1, 2, 0, 2, 0, 2, 1, 2}, 7},
    {{1, 2, 0, 2, 1, 2, 0, 2}, 0},
    {{0, 2, 1, 2, 0, 2, 1, 2}, 1},
    {{1, 2, 0, 2, 0, 2, 0, 2}, 8},
    {{0, 2, 1, 2, 0, 2, 0, 2}, 9},
    {{0, 2, 0, 2, 1, 2, 0, 2}, 10},
    {{0, 2, 0, 2, 0, 2, 1, 2}, 11},
    {{0, 0, 0, 0, 0, 0, 0, 0}, 12},
    {{1, 1, 1, 1, 1, 1, 1, 1}, 48},
    {{2, 2, 2, 2, 2, 2, 2, 2}, 12}
};
}

eRoad::eRoad(eGameBoard& board, const eCityId cid) :
    eBuilding(board, eBuildingType::road, 1, 1, cid) {}

void eRoad::erase() {
    if(mAboveHippodrome) {
        eBuilding::erase();
        mAboveHippodrome->erase();
    } else if(isBridge()) {
        auto& board = getBoard();
        const auto cid = cityId();
        const bool a = board.hasActiveInvasions(cid);
        if(a) {
            board.showTip(cid, eLanguage::zeusText(19, 229)); // can't demolish during invasion
            return;
        }
        std::vector<eTile*> tiles;
        bridgeConnectedTiles(tiles);
        for(const auto t : tiles) {
            const auto c = t->characters();
            if(!c.empty()) {
                board.showTip(cid, eLanguage::zeusText(19, 24)); // can't demolish water crossing with people
                return;
            }
        }
        for(const auto t : tiles) {
            const auto b = t->underBuilding();
            if(!b) continue;
            b->eBuilding::erase();
        }
    } else {
        if(mRoadblock) setRoadblock(false);
        else eBuilding::erase();
    }
}

std::shared_ptr<eTexture> eRoad::getTexture(const eTileSize size) const {
    if(mAboveHippodrome) return nullptr;

    auto& board = getBoard();
    const auto dir = board.direction();

    const int sizeId = static_cast<int>(size);
    const auto& trrTexs = eGameTextures::terrain()[sizeId];
    const auto& builTexs = eGameTextures::buildings()[sizeId];

    if(mRoadblock) {
        eGameTextures::loadRoadblock();
        return builTexs.fRoadblock;
    }

    if(mUnderAgora) {
        const auto& coll = builTexs.fAgoraRoad;
        return coll.getTexture(seed() % coll.size());
    }

    if(mUnderGatehouse) {
        const auto& r = tileRect();
        const auto& rr = mUnderGatehouse->tileRect();
        const int dx = r.x - rr.x;
        const int dy = r.y - rr.y;
        if(dy == 0) {
            return mUnderGatehouse->getTextureEntranceBack(size).fTex;
        } else if(dy == 1) {
            return mUnderGatehouse->getTextureEntrance(size).fTex;
        } else if(dx == 0) {
            return mUnderGatehouse->getTextureEntranceBack(size).fTex;
        } else if(dx == 1) {
            return mUnderGatehouse->getTextureEntrance(size).fTex;
        }
    }

    const auto ti = centerTile();
    const auto tr = ti->topRightRotated(dir);
    const auto br = ti->bottomRightRotated(dir);
    const auto bl = ti->bottomLeftRotated(dir);
    const auto tl = ti->topLeftRotated(dir);

    const bool trRoad = !tr || tr->hasRoad();
    const bool brRoad = !br || br->hasRoad();
    const bool blRoad = !bl || bl->hasRoad();
    const bool tlRoad = !tl || tl->hasRoad();

    const auto& tbcoll = trrTexs.fToBeachRoad;
    const auto& bcoll = trrTexs.fBeachRoad;

    const bool bridge = ti->hasBridge();
    if(bridge) {
        eGameTextures::loadBridge();

        int texId = 10;

        const bool trb = tr && tr->hasBridge();
        const bool brb = br && br->hasBridge();
        const bool blb = bl && bl->hasBridge();
        const bool tlb = tl && tl->hasBridge();

        const bool trw = tr && (tr->hasWater() || tr->hasQuake());
        const bool brw = br && (br->hasWater() || br->hasQuake());
        const bool blw = bl && (bl->hasWater() || bl->hasQuake());
        const bool tlw = tl && (tl->hasWater() || tl->hasQuake());

        if((tlb || brb) || ((trw && !trb) || (blw && !blb))) {
            texId = 10;
        } else if((trb || blb) || ((tlw && !tlb) || (brw && !brb))) {
            texId = 11;
        }
        return builTexs.fBridge.getTexture(texId);
    }

    const auto isAvn = [](eTileBase* const t) {
        return t && eAvenue::sAvenueType(t->underBuildingType());
    };
    const bool avn = isAvn(tr) || isAvn(br) || isAvn(tl) || isAvn(bl);

    if(avn) {
        // zeus.exe picks the Avenue/Boulevard road picture from its table
        // (0x004193A0, 0x005E92B8); see eAvenue::sRoadImage
        eGameTextures::loadAvenue();
        const int id = eAvenue::sRoadImage(ti, dir, seed());
        return builTexs.fAvenueRoad.getTexture(id);
    }

    if(ti->terrain() == eTerrain::beach) {
        if(bl && bl->terrain() == eTerrain::dry) {
            return tbcoll.getTexture(0);
        } else if(tl && tl->terrain() == eTerrain::dry) {
            return tbcoll.getTexture(1);
        } else if(tr && tr->terrain() == eTerrain::dry) {
            return tbcoll.getTexture(2);
        } else if(br && br->terrain() == eTerrain::dry) {
            return tbcoll.getTexture(3);
        }


        if(tlRoad && blRoad && trRoad && brRoad) return bcoll.getTexture(17);
        if(trRoad && brRoad && blRoad) return bcoll.getTexture(13);
        if(tlRoad && blRoad && brRoad) return bcoll.getTexture(14);
        if(tlRoad && blRoad && trRoad) return bcoll.getTexture(15);
        if(tlRoad && brRoad && trRoad) return bcoll.getTexture(16);

        if(blRoad && trRoad) {
            if(seed() % 2) return bcoll.getTexture(0);
            else return bcoll.getTexture(2);
        }
        if(tlRoad && brRoad) {
            if(seed() % 2) return bcoll.getTexture(1);
            else return bcoll.getTexture(3);
        }

        if(trRoad && brRoad) return bcoll.getTexture(4);
        if(blRoad && brRoad) return bcoll.getTexture(5);
        if(blRoad && tlRoad) return bcoll.getTexture(6);
        if(tlRoad && trRoad) return bcoll.getTexture(7);
        if(trRoad) return bcoll.getTexture(8);
        if(brRoad) return bcoll.getTexture(9);
        if(blRoad) return bcoll.getTexture(10);
        if(tlRoad) return bcoll.getTexture(11);

        return bcoll.getTexture(12);

    } else {
        // the original's road looks by the tile's appeal (zeus.exe
        // 0x004A0190, 0x004A0450): below 0 rutted, 0-15 plain, 16-29
        // paved, 30 or more plaza stones
        const auto& b = getBoard();
        const auto t = centerTile();
        const double a = b.appeal(t->dx(), t->dy());

        if(a >= 30) {
            const auto& coll = trrTexs.fPlazaRoad;
            return coll.getTexture(seed() % coll.size());
        }

        const eRoadPiece* pieces;
        int nPieces;
        const eTextureCollection* coll;
        if(a < 16) {
            pieces = gPlainRoad;
            nPieces = sizeof(gPlainRoad)/sizeof(gPlainRoad[0]);
        } else {
            pieces = gWideRoad;
            nPieces = sizeof(gWideRoad)/sizeof(gWideRoad[0]);
        }
        if(a < 0) {
            coll = &trrTexs.fRuttedRoad;
        } else if(a < 16) {
            coll = &trrTexs.fRoad;
        } else {
            coll = &trrTexs.fPrettyRoad;
        }

        // the 8 tiles around, in the tables' order; off the map counts
        // as road
        const decltype(tr) ns[8] = {tr, ti->rightRotated(dir),
                                    br, ti->bottomRotated(dir),
                                    bl, ti->leftRotated(dir),
                                    tl, ti->topRotated(dir)};
        bool road[8];
        for(int i = 0; i < 8; i++) {
            road[i] = !ns[i] || ns[i]->hasRoad();
        }
        for(int p = 0; p < nPieces; p++) {
            const auto& pc = pieces[p];
            bool fits = true;
            for(int i = 0; i < 8 && fits; i++) {
                if(pc.fN[i] == 2) continue;
                fits = road[i] == (pc.fN[i] == 1);
            }
            if(fits) return coll->getTexture(pc.fImg);
        }
        return coll->getTexture(12);
    }
    return std::shared_ptr<eTexture>();
}

int eRoad::getHippodromeTextureId() const {
    auto& board = getBoard();
    const auto dir = board.direction();

    const auto ti = centerTile();
    const auto tr = ti->topRight();
    const auto br = ti->bottomRight();
    const auto bl = ti->bottomLeft();
    const auto tl = ti->topLeft();

    const bool trRoad = !tr || tr->hasRoad();
    const bool brRoad = !br || br->hasRoad();
    const bool blRoad = !bl || bl->hasRoad();
    const bool tlRoad = !tl || tl->hasRoad();

    if(mAboveHippodrome) {
        int texId1 = 0;
        int texId2 = 0;
        int texId3 = 0;
        int texId4 = 0;
        int texId5 = 0;
        int texId6 = 0;
        switch(dir) {
        case eWorldDirection::N: {
            texId1 = 8;
            texId2 = 10;
            texId3 = 13;
            texId4 = 11;
            texId5 = 9;
            texId6 = 12;
        } break;
        case eWorldDirection::E: {
            texId1 = 13;
            texId2 = 11;
            texId3 = 10;
            texId4 = 8;
            texId5 = 12;
            texId6 = 9;
        } break;
        case eWorldDirection::S: {
            texId1 = 10;
            texId2 = 8;
            texId3 = 11;
            texId4 = 13;
            texId5 = 9;
            texId6 = 12;
        } break;
        case eWorldDirection::W: {
            texId1 = 11;
            texId2 = 13;
            texId3 = 8;
            texId4 = 10;
            texId5 = 12;
            texId6 = 9;
        } break;
        }
        const auto& r = tileRect();
        const auto& rr = mAboveHippodrome->tileRect();
        if(r.y == rr.y + rr.h - 1) {
            if(trRoad) {
                return texId1;
            }
        }
        if(r.y == rr.y) {
            if(blRoad) {
                return texId2;
            }
        }
        if(r.x == rr.x + rr.w - 1) {
            if(tlRoad) {
                return texId3;
            }
        }
        if(r.x == rr.x) {
            if(brRoad) {
                return texId4;
            }
        }
        if(trRoad && blRoad) return texId5;
        if(tlRoad && brRoad) return texId6;
    }
    return 0;
}

void eRoad::setUnderAgora(eAgoraBase* const a) {
    mUnderAgora = a;
}

void eRoad::setUnderGatehouse(eGatehouse* const g) {
    mUnderGatehouse = g;
}

void eRoad::setAboveHippodrome(eHippodromePiece * const h) {
    mAboveHippodrome = h;
}

bool eRoad::isBridge() const {
    const auto t = centerTile();
    if(!t) return false;
    return t->hasBridge();
}

bool eRoad::isRoadblock() const {
    return mRoadblock;
}

void eRoad::setRoadblock(const bool rb) {
    const auto t = centerTile();
    if(t) t->setRoadblock(rb);
    mRoadblock = rb;
}

void eRoad::bridgeConnectedTiles(std::vector<eTile*>& tiles) const {
    eTile* const startTile = centerTile();
    if(!startTile) return;
    if(!startTile->hasBridge()) return;
    tiles.push_back(startTile);
    auto tl = startTile->topLeft<eTile>();
    auto tr = startTile->topRight<eTile>();
    auto br = startTile->bottomRight<eTile>();
    auto bl = startTile->bottomLeft<eTile>();
    while(tl) {
        const bool hb = tl->hasBridge();
        if(hb) tiles.push_back(tl);
        else break;
        tl = tl->topLeft<eTile>();
    }
    while(tr) {
        const bool hb = tr->hasBridge();
        if(hb) tiles.push_back(tr);
        else break;
        tr = tr->topRight<eTile>();
    }
    while(br) {
        const bool hb = br->hasBridge();
        if(hb) tiles.push_back(br);
        else break;
        br = br->bottomRight<eTile>();
    }
    while(bl) {
        const bool hb = bl->hasBridge();
        if(hb) tiles.push_back(bl);
        else break;
        bl = bl->bottomLeft<eTile>();
    }
}

void eRoad::write(eWriteStream &dst) const {
    eBuilding::write(dst);
    dst << mRoadblock;
    dst.writeBuilding(mUnderAgora);
    dst.writeBuilding(mUnderGatehouse);
    dst.writeBuilding(mAboveHippodrome);
    dst << mCharacterAltitude;
}

void eRoad::read(eReadStream &src) {
    eBuilding::read(src);
    auto& board = getBoard();
    src >> mRoadblock;
    src.readBuilding(&board, [this](eBuilding* const bb) {
        setUnderAgora(static_cast<eAgoraBase*>(bb));
    });
    src.readBuilding(&board, [this](eBuilding* const bb) {
        setUnderGatehouse(static_cast<eGatehouse*>(bb));
    });
    src.readBuilding(&board, [this](eBuilding* const bb) {
        setAboveHippodrome(static_cast<eHippodromePiece*>(bb));
    });
    src >> mCharacterAltitude;
}

bool eRoad::largeBridge() const {
    const auto ti = centerTile();
    if(!ti || !ti->hasBridge() || !ti->hasWater()) return false;
    const int len = ti->bridgeLength();
    // Long crossings are Large Water Crossings, and Atlantean cities build
    // Bridges (manuals; 5+ tiles is large, per Nicolás). Very short
    // Atlantean crossings keep the small deck: the end pieces need 4 tiles.
    if(len < 4) return false;
    const bool atl = getBoard().atlantean(cityId());
    return atl || len > 4;
}

std::shared_ptr<eTexture> eRoad::bridgePiece(const eTileSize size) const {
    if(!largeBridge()) return nullptr;
    const auto ti = centerTile();
    auto& board = getBoard();
    const auto dir = board.direction();
    const int sizeId = static_cast<int>(size);
    const auto& builTexs = eGameTextures::buildings()[sizeId];

    const auto tr = ti->topRightRotated<eTile>(dir);
    const auto bl = ti->bottomLeftRotated<eTile>(dir);
    const bool trb = tr && tr->hasBridge();
    const bool blb = bl && bl->hasBridge();
    // crossing running up-left to down-right on screen, or up-right to
    // down-left
    const bool tlbrAxis = !(trb || blb);

    const int len = ti->bridgeLength();
    int k = 0; // tiles to the upper end
    auto t = ti;
    while(true) {
        const auto n = tlbrAxis ? t->topLeftRotated<eTile>(dir) :
                                  t->topRightRotated<eTile>(dir);
        if(!n || !n->hasBridge()) break;
        t = n;
        k++;
    }

    // Pieces as in the original (Nicolás's screenshots). Each end piece
    // covers two tiles and is drawn from the lower one so the tiles drawn
    // later don't cover it: the upper end from the second tile, the lower
    // end from the last tile. Middle pieces cover their own tile.
    int id;
    if(k == 0 || k == len - 2) return nullptr;
    if(k == 1) id = tlbrAxis ? 0 : 7;
    else if(k == len - 1) id = tlbrAxis ? 3 : 4;
    else id = tlbrAxis ? 1 : 5;
    const bool middle = id == 1 || id == 5;

    if(board.atlantean(cityId())) {
        eGameTextures::loadPoseidonBridge();
        if(middle) {
            // the Atlantean Bridge leaves out the pillars in the middle,
            // where ships pass under: deck only 296 / 301, with a pillar
            // 297 / 300
            const double c = 0.5*(len - 1);
            const bool centre = std::abs(k - c) < 1.;
            if(tlbrAxis) id = centre ? 1 : 2;
            else id = centre ? 6 : 5;
        }
        return builTexs.fPoseidonBridge.getTexture(id);
    }
    eGameTextures::loadBridge();
    if(middle && k % 2) id++;
    return builTexs.fBridge.getTexture(id);
}
