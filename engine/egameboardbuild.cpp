// Part of eGameBoard: placing buildings on the map.
// (Split from egameboard.cpp on 13 Nov 2026; nothing but the file changed.)

#include "buildings/sanctuaries/etempleobeliskbuilding.h"
#include "buildings/sanctuaries/etemplegrandstatuebuilding.h"
#include "egameboard.h"

#include "buildings/eagorabase.h"
#include "characters/echaracter.h"
#include "buildings/ebuilding.h"
#include "spawners/espawner.h"

#include "buildings/estoragebuilding.h"

#include "characters/esoldier.h"
#include "characters/esoldierbanner.h"
#include "characters/esheep.h"
#include "characters/egoat.h"

#include "buildings/etradepost.h"

#include "buildings/esmallhouse.h"

#include "buildings/epalace.h"
#include "buildings/epalacetile.h"

#include "buildings/ehorseranch.h"
#include "buildings/eavenue.h"

#include "engine/boardData/eheatmaptask.h"

#include "missiles/emissile.h"
#include "missiles/ewavemissile.h"
#include "missiles/elavamissile.h"
#include "missiles/edustmissile.h"

#include "etilehelper.h"

#include "emessages.h"

#include "spawners/emonsterpoint.h"
#include "spawners/elandinvasionpoint.h"

#include "buildings/eheatgetters.h"

#include "fileIO/ebuildingreader.h"
#include "fileIO/ebuildingwriter.h"

#include "eevent.h"
#include "emessageeventtype.h"

#include "gameEvents/egodattackevent.h"
#include "gameEvents/emonsterunleashedevent.h"
#include "gameEvents/einvasionevent.h"
#include "gameEvents/epaytributeevent.h"
#include "gameEvents/emakerequestevent.h"
#include "gameEvents/egifttoevent.h"
#include "gameEvents/egiftfromevent.h"
#include "gameEvents/ereceiverequestevent.h"
#include "gameEvents/erequestaidevent.h"
#include "gameEvents/eplayerconquesteventbase.h"
#include "gameEvents/etroopsrequestevent.h"
#include "gameEvents/earmyreturnevent.h"

#include "eeventdata.h"

#include "einvasionhandler.h"
#include "characters/actions/emonsteraction.h"

#include "evectorhelpers.h"
#include "egifthelpers.h"
#include "estringhelpers.h"

#include "buildings/eheroshall.h"
#include "buildings/emuseum.h"
#include "buildings/estadium.h"
#include "buildings/eagoraspace.h"
#include "buildings/evendor.h"
#include "buildings/eroad.h"
#include "buildings/egatehouse.h"
#include "eplague.h"
#include "audio/emusic.h"

#include "ecampaign.h"
#include "eiteratesquare.h"
#include "ecolonymonumentaction.h"
#include "textures/emarbletile.h"
#include "elanguage.h"
#include "enumbers.h"

#include "characters/actions/eanimalaction.h"
#include "buildings/eanimalbuilding.h"

#include "buildings/eplaceholder.h"
#include "buildings/sanctuaries/ezeussanctuary.h"
#include "buildings/sanctuaries/ehephaestussanctuary.h"
#include "buildings/sanctuaries/eartemissanctuary.h"
#include "buildings/sanctuaries/estairsrenderer.h"
#include "buildings/sanctuaries/etempletilebuilding.h"
#include "buildings/sanctuaries/etemplestatuebuilding.h"
#include "buildings/sanctuaries/etemplemonumentbuilding.h"
#include "buildings/sanctuaries/etemplealtarbuilding.h"
#include "buildings/sanctuaries/etemplebuilding.h"

#include "characters/etrireme.h"

#include "buildings/pyramids/epyramid.h"

#include <algorithm>
#include <set>
#include "buildings/ehousebase.h"
#include "buildings/eemployingbuilding.h"

#ifndef uint
#define uint unsigned int
#endif

void centerTile(const int minX, const int minY,
                const int sw, const int sh,
                int& tx, int& ty) {
    tx = minX;
    ty = minY;

    if(sw == 2 && sh == 2) {
        ty += 1;
    } else if(sw == 3 && sh == 3) {
        tx += 1;
        ty += 1;
    } else if(sw == 4 || sh == 4) {
        tx += 1;
        ty += 2;
    } else if(sw == 5 || sh == 5) {
        tx += 2;
        ty += 2;
    } else if(sw == 6 || sh == 6) {
        tx += 2;
        ty += 2;
    }
}

void eGameBoard::sBuildTiles(int& minX, int& minY,
                             int& maxX, int& maxY,
                             const int tx, const int ty,
                             const int sw, const int sh) {
    minX = tx;
    minY = ty;

    if(sw == 2 && sh == 2) {
        minY -= 1;
    } else if(sw == 3 && sh == 3) {
        minX -= 1;
        minY -= 1;
    } else if(sw == 4 || sh == 4) {
        minX -= 1;
        minY -= 2;
    } else if(sw == 5 || sh == 5) {
        minX -= 2;
        minY -= 2;
    } else if(sw == 6 || sh == 6) {
        minX -= 2;
        minY -= 2;
    }

    maxX = minX + sw;
    maxY = minY + sh;
}

bool eGameBoard::canBuildAvenue(eTile* const t, const eCityId cid,
                                const ePlayerId pid,
                                const bool forestAllowed) const {
    const int tx = t->x();
    const int ty = t->y();
    const bool cb = canBuildBase(tx, tx + 1, ty, ty + 1, forestAllowed, cid, pid);
    if(!cb) return false;
    const auto tr = t->topRight<eTile>();
    const auto br = t->bottomRight<eTile>();
    const auto bl = t->bottomLeft<eTile>();
    const auto tl = t->topLeft<eTile>();
    const auto tt = t->top<eTile>();
    const auto r = t->right<eTile>();
    const auto b = t->bottom<eTile>();
    const auto l = t->left<eTile>();
    const bool hr = (tr && tr->hasRoad()) ||
                    (br && br->hasRoad()) ||
                    (bl && bl->hasRoad()) ||
                    (tl && tl->hasRoad()) ||
                    (tt && tt->hasRoad()) ||
                    (r && r->hasRoad()) ||
                    (b && b->hasRoad()) ||
                    (l && l->hasRoad());
    return hr;
}

// Avenue and Boulevard pieces, after the original game (zeus.exe):
// - Boulevard (0x0041BBA0, 0x0041B250, piece 0x0041AFB0): the drag must
//   start on a road; each road tile along the drag gets trees on both
//   sides, across the road's direction
// - Avenue (0x0041AEA0, 0x00419F90, piece 0x00419C90): trees on one side
//   only. Starting on the road puts them on its top-left side (x - 1) for
//   a road running along y, or its top-right side (y - 1) for one running
//   along x; starting next to a road puts them on the tile clicked
// - the drag runs along its longer direction from the start tile; the
//   side keeps to the same edge (lower or higher coordinate) if the drag
//   turns across the road's direction; a click without a drag builds
//   nothing
// - each piece needs a road tile and free tiles for the trees
// - a piece with no Avenue or Boulevard next to it along the line is taken
//   back and not charged (0x004195D0), so the shortest is two pieces
// Kept from eZeus: the trees can't go over another road (the original
// allows it), and a drag that doesn't start on or next to a road builds
// nothing (the original then picks a side by the drag direction).
std::vector<eGameBoard::eAvenuePiece> eGameBoard::avenuePieces(
        const int sx, const int sy, const int ex, const int ey,
        const eBuildingType type, const eCityId cid,
        const ePlayerId pid, const bool forestAllowed,
        const bool startOnly) const {
    std::vector<eAvenuePiece> result;
    const auto road = [&](const int x, const int y) {
        const auto t = tile(x, y);
        return t && t->hasRoad();
    };
    const bool boulevard = type == eBuildingType::boulevard;
    // o: 0 trees at x - 1, 1 at y - 1, 2 at x + 1, 3 at y + 1 of the road
    // tile; a Boulevard (0 or 1) has them on both sides
    int o = 0;
    int rx = sx; // the start piece's road tile
    int ry = sy;
    if(road(sx, sy)) {
        const bool e = road(sx + 1, sy);
        const bool w = road(sx - 1, sy);
        const bool n = road(sx, sy - 1);
        const bool s = road(sx, sy + 1);
        if(e && w) o = 1;
        else if(n && s) o = 0;
        else if(!e && !w) {
            if(!n && !s) return result;
            o = 0;
        } else o = 1;
    } else if(boulevard) {
        return result;
    } else if(road(sx - 1, sy)) {
        o = 2;
        rx = sx - 1;
    } else if(road(sx, sy - 1)) {
        o = 3;
        ry = sy - 1;
    } else if(road(sx + 1, sy)) {
        o = 0;
        rx = sx + 1;
    } else if(road(sx, sy + 1)) {
        o = 1;
        ry = sy + 1;
    } else {
        return result;
    }

    // the original measures the drag from the piece's own tile: the road
    // tile, or the trees for an Avenue started beside a road to its
    // top-left or top-right
    const bool treesStart = o == 2 || o == 3;
    const int dx = std::abs(ex - (treesStart ? sx : rx));
    const int dy = std::abs(ey - (treesStart ? sy : ry));
    if(!startOnly && dx < 1 && dy < 1) return result;
    // the cursor preview keeps the start's own direction
    const bool alongX = startOnly ? (o == 1 || o == 3) : dy < dx;
    if(!startOnly) {
        if(alongX) {
            if(o == 0) o = 1;
            else if(o == 2) o = 3;
        } else {
            if(o == 1) o = 0;
            else if(o == 3) o = 2;
        }
    }

    std::vector<std::pair<int, int>> sides;
    switch(o) {
    case 0: sides.push_back({-1, 0}); break;
    case 1: sides.push_back({0, -1}); break;
    case 2: sides.push_back({1, 0}); break;
    case 3: sides.push_back({0, 1}); break;
    }
    if(boulevard) {
        sides.push_back({-sides[0].first, -sides[0].second});
    }

    const int from = startOnly ? (alongX ? rx : ry) :
                     alongX ? std::min(rx, ex) : std::min(ry, ey);
    const int to = startOnly ? from :
                   alongX ? std::max(rx, ex) : std::max(ry, ey);
    // In the original a piece takes its road tile too: it is built only
    // where the road tile and the tree tiles have no building yet
    // (0x00419F90). eZeus keeps the road tile a road, so a road tile that
    // already has Avenue or Boulevard trees beside it, on either side
    // across the road, counts as taken: a road gets one Avenue or one
    // Boulevard, never two Avenues facing each other.
    const auto taken = [&](const int x, const int y) {
        const int ax = sides[0].first;
        const int ay = sides[0].second;
        for(const int d : {-1, 1}) {
            const auto t = tile(x + d*ax, y + d*ay);
            if(t && eAvenue::sAvenueType(t->underBuildingType())) return true;
        }
        return false;
    };
    for(int i = from; i <= to; i++) {
        const int x = alongX ? i : rx;
        const int y = alongX ? ry : i;
        if(!road(x, y)) continue;
        if(taken(x, y)) continue;
        eAvenuePiece p;
        p.fRoad = tile(x, y);
        bool ok = true;
        for(const auto& s : sides) {
            const int tx = x + s.first;
            const int ty = y + s.second;
            const auto t = tile(tx, ty);
            if(!t || !canBuildBase(tx, tx + 1, ty, ty + 1, forestAllowed,
                                   cid, pid, false, true)) {
                ok = false;
                break;
            }
            p.fTrees.push_back(t);
        }
        if(ok) result.push_back(p);
    }

    if(startOnly) return result;

    // take back pieces with nothing beside them along the line
    std::set<eTile*> newTrees;
    for(const auto& p : result) {
        for(const auto t : p.fTrees) newTrees.insert(t);
    }
    const int lx = alongX ? 1 : 0;
    const int ly = alongX ? 0 : 1;
    const auto joined = [&](eTile* const t) {
        for(const int d : {-1, 1}) {
            const auto n = tile(t->x() + d*lx, t->y() + d*ly);
            if(!n) continue;
            if(newTrees.count(n)) return true;
            if(eAvenue::sAvenueType(n->underBuildingType())) return true;
        }
        return false;
    };
    std::vector<eAvenuePiece> kept;
    for(const auto& p : result) {
        bool j = false;
        for(const auto t : p.fTrees) {
            if(joined(t)) {
                j = true;
                break;
            }
        }
        if(j) kept.push_back(p);
    }
    return kept;
}

bool eGameBoard::buildAvenuePiece(const eAvenuePiece& p,
                                  const eBuildingType type,
                                  const eCityId cid, const ePlayerId pid,
                                  const bool editorDisplay) {
    for(const auto t : p.fTrees) {
        const int tx = t->x();
        const int ty = t->y();
        if(!canBuildBase(tx, tx + 1, ty, ty + 1, editorDisplay,
                         cid, pid, false, true)) {
            return false;
        }
    }
    bool first = true;
    for(const auto t : p.fTrees) {
        const int tx = t->x();
        const int ty = t->y();
        // one charge for the whole piece (zeus.exe counts pieces)
        buildBase(tx, ty, tx, ty, [this, cid, type]() {
            return e::make_shared<eAvenue>(*this, cid, type);
        }, pid, cid, editorDisplay || !first, false, true);
        first = false;
    }
    return true;
}

bool eGameBoard::canBuildBase(const int minX, const int maxX,
                              const int minY, const int maxY,
                              const bool forestAllowed,
                              const eCityId cid,
                              const ePlayerId pid,
                              const bool fertile,
                              const bool flat,
                              const int allowedWater,
                              const bool animalsMove) const {
    if(pid != cityIdToPlayerId(cid) && !mEditorMode) return false;
    int waterCount = 0;
    bool fertileFound = false;
    for(int x = minX; x < maxX; x++) {
        for(int y = minY; y < maxY; y++) {
            const auto t = tile(x, y);
            if(!t) return false;
            if(t->cityId() != cid) return false;
            if(const auto ub = t->underBuilding()) {
                const auto ubt = ub->type();
                const bool animal = ubt == eBuildingType::sheep ||
                                    ubt == eBuildingType::goat;
                if(!animal || !animalsMove) return false;
            }
            const auto& banners = t->banners();
            for(const auto& b : banners) {
                if(!b->buildable()) return false;
            }
            const auto ttt = t->terrain();
            if(fertile && ttt == eTerrain::fertile) {
                fertileFound = true;
            }

            if(ttt == eTerrain::water) {
                waterCount++;
                if(waterCount > allowedWater) return false;
            } else {
                const auto ttta = forestAllowed ?
                                      ttt & eTerrain::buildableAfterClear :
                                      ttt & eTerrain::buildable;
                if(!static_cast<bool>(ttta)) return false;
            }

            if(!t->walkableElev() && t->isElevationTile()) return false;

            if(!flat) {
                const auto& chars = t->characters();
                if(!chars.empty()) return false;
            }
        }
    }
    if(fertile) return fertileFound;
    return true;
}

bool eGameBoard::canBuild(const int tx, const int ty,
                          const int sw, const int sh,
                          const bool forestAllowed,
                          const eCityId cid,
                          const ePlayerId pid,
                          const bool fertile,
                          const bool flat) const {
    int minX;
    int minY;
    int maxX;
    int maxY;
    sBuildTiles(minX, minY, maxX, maxY, tx, ty, sw, sh);
    return canBuildBase(minX, maxX, minY, maxY,
                        forestAllowed, cid, pid,
                        fertile, flat);
}


bool eGameBoard::canBuildTowerOnWalls(const int tx, const int ty,
                                      const eCityId cid,
                                      const ePlayerId pid) const {
    if(pid != cityIdToPlayerId(cid) && !mEditorMode) return false;
    int minX;
    int minY;
    int maxX;
    int maxY;
    sBuildTiles(minX, minY, maxX, maxY, tx, ty, 2, 2);
    for(int x = minX; x < maxX; x++) {
        for(int y = minY; y < maxY; y++) {
            const auto t = tile(x, y);
            if(!t) return false;
            if(t->cityId() != cid) return false;
            if(t->isElevationTile()) return false;
            const auto ub = t->underBuilding();
            if(!ub) return false;
            if(ub->type() != eBuildingType::wall) return false;
            if(ub->isOnFire()) return false;
        }
    }
    return true;
}

void eGameBoard::eraseWallsUnderTower(const int tx, const int ty) {
    int minX;
    int minY;
    int maxX;
    int maxY;
    sBuildTiles(minX, minY, maxX, maxY, tx, ty, 2, 2);
    for(int x = minX; x < maxX; x++) {
        for(int y = minY; y < maxY; y++) {
            const auto t = tile(x, y);
            if(!t) continue;
            const auto ub = t->underBuilding();
            if(!ub || ub->type() != eBuildingType::wall) continue;
            ub->erase();
        }
    }
}

bool eGameBoard::buildBase(const int minX, const int minY,
                           const int maxX, const int maxY,
                           const eBuildingCreator& bc,
                           const ePlayerId pid,
                           const eCityId cid,
                           const bool editorDisplay,
                           const bool fertile,
                           const bool flat,
                           const int allowWater) {
    const int sw = maxX - minX + 1;
    const int sh = maxY - minY + 1;
    const bool cb = canBuildBase(minX, maxX + 1, minY, maxY + 1,
                                 editorDisplay,
                                 cid, pid, fertile, flat,
                                 allowWater, true);
    if(!cb) return false;
    if(!bc) return false;
    const auto b = bc();
    if(!b) return false;
    const bool isRoad = b->type() == eBuildingType::road;
    if(!isRoad) {
        for(int x = minX; x <= maxX; x++) {
            for(int y = minY; y <= maxY; y++) {
                const auto t = tile(x, y);
                if(!t) return false;
                if(t->isElevationTile()) return false;
            }
        }
    }
    int tx;
    int ty;
    centerTile(minX, minY, sw, sh, tx, ty);
    const auto tile = this->tile(tx, ty);
    if(!tile) return false;
    // sheep and goats in the way wander off (eZeus issue 4)
    struct eMovedAnimal { eBuildingType fType; int fX; int fY; };
    std::vector<eMovedAnimal> movedAnimals;
    for(int x = minX; x <= maxX; x++) {
        for(int y = minY; y <= maxY; y++) {
            const auto t = this->tile(x, y);
            if(!t) continue;
            const auto ub = t->underBuilding();
            if(!ub) continue;
            const auto ubt = ub->type();
            if(ubt != eBuildingType::sheep &&
               ubt != eBuildingType::goat) continue;
            const auto& r = ub->tileRect();
            movedAnimals.push_back({ubt, r.x, r.y});
            ub->erase();
        }
    }
    b->setCenterTile(tile);
    b->setTileRect({minX, minY, sw, sh});
    for(int x = minX; x <= maxX; x++) {
        for(int y = minY; y <= maxY; y++) {
            const auto t = this->tile(x, y);
            if(t) {
                t->setUnderBuilding(b);
                b->addUnderBuilding(t);
            }
        }
    }
    for(const auto& m : movedAnimals) {
        rebuildAnimalNear(m.fType, m.fX, m.fY, pid, cid, editorDisplay);
    }

    if(!editorDisplay) {
        const auto diff = difficulty(pid);
        const int cost = eDifficultyHelpers::buildingCost(diff, b->type());
        incDrachmas(pid, -cost, eFinanceTarget::construction);
        if(mUndoRecording) {
            mUndoPending.fBuildings.emplace_back(b.get());
            mUndoPending.fCost += cost;
        }
    }
    return true;
}

bool eGameBoard::build(const int tx, const int ty,
                       const int sw, const int sh,
                       const eCityId cid,
                       const ePlayerId pid,
                       const bool editorDisplay,
                       const eBuildingCreator& bc,
                       const bool fertile,
                       const bool flat) {
    const auto tile = this->tile(tx, ty);
    if(!tile) return false;
    int minX;
    int minY;
    int maxX;
    int maxY;
    sBuildTiles(minX, minY, maxX, maxY,
               tx, ty, sw, sh);
    return buildBase(minX, minY, maxX - 1, maxY - 1,
                     bc, pid, cid, editorDisplay, fertile, flat);
}

bool eGameBoard::buildAnimal(eTile* const tile,
                             const eBuildingType type,
                             const eAnimalCreator& creator,
                             const eCityId cid,
                             const ePlayerId pid,
                             const bool editorDisplay) {
    const int tx = tile->x();
    const int ty = tile->y();
    {   // an animal never pushes another animal away
        int minX;
        int minY;
        int maxX;
        int maxY;
        sBuildTiles(minX, minY, maxX, maxY, tx, ty, 1, 2);
        for(int x = minX; x < maxX; x++) {
            for(int y = minY; y < maxY; y++) {
                const auto t = this->tile(x, y);
                if(t && t->underBuilding()) return false;
            }
        }
    }
    const bool cb = canBuild(tx, ty, 1, 2, editorDisplay, cid, pid, true, true);
    if(!cb) return false;
    const auto sh = creator(*this);
    sh->changeTile(tile);
    const auto o = static_cast<eOrientation>(eRand::rand() % 8);
    sh->setOrientation(o);
    const auto w = eWalkableObject::sCreateFertile();
    const auto a = e::make_shared<eAnimalAction>(sh.get(), tx, ty, w);
    sh->setAction(a);

    return build(tx, ty, 1, 2, cid, pid, editorDisplay, [this, sh, type, cid]() {
        return e::make_shared<eAnimalBuilding>(
                    *this, sh.get(), type, cid);
    }, true, true);
}

void eGameBoard::removeAllBuildings() {
    for(const auto& b : mAllBuildings) {
        b->erase();
    }
}

bool eGameBoard::buildPyramid(const int minX, const int maxX,
                              const int minY, const int maxY,
                              const eBuildingType type,
                              const bool rotate,
                              const eCityId cid,
                              const ePlayerId pid,
                              const bool editorDisplay) {
    (void)rotate;
    const bool cb = canBuildBase(minX, maxX, minY, maxY,
                                 editorDisplay, cid, pid);
    if(!cb) return false;

    int sw;
    int sh;
    ePyramid::sDimensions(type, sw, sh);
    const auto p = e::make_shared<ePyramid>(*this, type, sw, sh, cid);
    p->setTileRect({minX, minY, sw, sh});

    const auto mint = this->tile(minX, minY);
    const int a = mint->altitude();
    p->setAltitude(a);

    const auto ct = this->tile((minX + maxX)/2, (minY + maxY)/2);
    p->setCenterTile(ct);

    const auto c = boardCityWithId(cid);
    const auto levels = c->pyramidLevels(type);
    p->initialize(levels);

    if(!editorDisplay) {
        const auto diff = difficulty(pid);
        const int cost = eDifficultyHelpers::buildingCost(diff, type);
        incDrachmas(pid, -cost, eFinanceTarget::construction);
        const int m = eBuilding::sInitialMarbleCost(type);
        takeResource(cid, eResourceType::marble, m);
    }

    built(cid, type);

    return true;
}

bool eGameBoard::buildSanctuary(const int minX, const int maxX,
                                const int minY, const int maxY,
                                const eBuildingType type,
                                const bool rotate,
                                const eCityId cid,
                                const ePlayerId pid,
                                const bool editorDisplay) {
    const bool cb = canBuildBase(minX, maxX, minY, maxY,
                                 editorDisplay, cid, pid);
    if(!cb) return false;

    // Atlantean cities build the Citadel of Poseidon (HeavenGames:
    // Promontory in Greek adventures, Citadel in Atlantean ones)
    const bool citadel = type == eBuildingType::templePoseidon &&
                         atlantean(cid);
    const auto h = eSanctBlueprints::sSanctuaryBlueprint(type, rotate,
                                                         citadel);

    const int sw = h->fW;
    const int sh = h->fH;

    const auto b = eSanctuary::sCreate(type, sw, sh, *this, cid);
    b->setRotated(rotate);
    const auto god = b->godType();

    if(!editorDisplay) {
        const auto diff = difficulty(pid);
        const int cost = citadel ? eSanctuary::sCitadelCost :
                                   eDifficultyHelpers::buildingCost(diff, type);
        incDrachmas(pid, -cost, eFinanceTarget::construction);
        const int m = citadel ? eSanctuary::sCitadelInitialMarble :
                                eBuilding::sInitialMarbleCost(type);
        takeResource(cid, eResourceType::marble, m);
    }

    built(cid, type);

    const auto mint = this->tile(minX, minY);
    const int a = mint->altitude();
    b->setAltitude(a);

    const SDL_Rect sanctRect{minX, minY, sw, sh};
    b->setTileRect(sanctRect);
    const auto ct = this->tile((minX + maxX)/2, (minY + maxY)/2);
    b->setCenterTile(ct);

    for(const auto& tv : h->fTiles) {
        for(const auto& t : tv) {
            const int tx = minX + t.fX;
            const int ty = minY + t.fY;
            const auto tile = this->tile(tx, ty);
            eGodType statueType;
            switch(t.fType) {
            case eSanctEleType::aphroditeStatue:
                statueType = eGodType::aphrodite;
                break;
            case eSanctEleType::apolloStatue:
                statueType = eGodType::apollo;
                break;
            case eSanctEleType::aresStatue:
                statueType = eGodType::ares;
                break;
            case eSanctEleType::artemisStatue:
                statueType = eGodType::artemis;
                break;
            case eSanctEleType::athenaStatue:
                statueType = eGodType::athena;
                break;
            case eSanctEleType::atlasStatue:
                statueType = eGodType::atlas;
                break;
            case eSanctEleType::demeterStatue:
                statueType = eGodType::demeter;
                break;
            case eSanctEleType::dionysusStatue:
                statueType = eGodType::dionysus;
                break;
            case eSanctEleType::hadesStatue:
                statueType = eGodType::hades;
                break;
            case eSanctEleType::hephaestusStatue:
                statueType = eGodType::hephaestus;
                break;
            case eSanctEleType::heraStatue:
                statueType = eGodType::hera;
                break;
            case eSanctEleType::hermesStatue:
                statueType = eGodType::hermes;
                break;
            case eSanctEleType::poseidonStatue:
                statueType = eGodType::poseidon;
                break;
            case eSanctEleType::zeusStatue:
                statueType = eGodType::zeus;
                break;
            default:
                statueType = god;
            }

            switch(t.fType) {
            case eSanctEleType::copper:
            case eSanctEleType::silver:
            case eSanctEleType::oliveTree:
            case eSanctEleType::vine:
            case eSanctEleType::orangeTree: {
                build(tile->x(), tile->y(), 1, 1, cid, pid, editorDisplay,
                      [this, cid]() { return e::make_shared<ePlaceholder>(*this, cid); });
                b->addSpecialTile(tile);
            } break;
            case eSanctEleType::defaultStatue:
            case eSanctEleType::aphroditeStatue:
            case eSanctEleType::apolloStatue:
            case eSanctEleType::aresStatue:
            case eSanctEleType::artemisStatue:
            case eSanctEleType::athenaStatue:
            case eSanctEleType::atlasStatue:
            case eSanctEleType::demeterStatue:
            case eSanctEleType::dionysusStatue:
            case eSanctEleType::hadesStatue:
            case eSanctEleType::hephaestusStatue:
            case eSanctEleType::heraStatue:
            case eSanctEleType::hermesStatue:
            case eSanctEleType::poseidonStatue:
            case eSanctEleType::zeusStatue: {
                const auto tt = e::make_shared<eTempleStatueBuilding>(
                                   statueType, t.fId, *this, cid);
                tt->setMonument(b.get());
                this->build(tx, ty, 1, 1, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
            } break;
            case eSanctEleType::monument: {
                const auto tt = e::make_shared<eTempleMonumentBuilding>(
                                    god, t.fId, *this, cid);
                tt->setMonument(b.get());
                const int d = rotate ? 1 : 0;
                this->build(tx - d, ty + d, 2, 2, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
            } break;
            case eSanctEleType::obelisk: {
                const auto tt = e::make_shared<eTempleObeliskBuilding>(
                                    t.fId, *this, cid);
                tt->setMonument(b.get());
                this->build(tx, ty, 1, 1, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
            } break;
            case eSanctEleType::grandStatue: {
                const auto tt = e::make_shared<eTempleGrandStatueBuilding>(
                                    *this, cid);
                tt->setMonument(b.get());
                this->build(tx, ty, 6, 6, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
            } break;
            case eSanctEleType::altar: {
                const auto tt = e::make_shared<eTempleAltarBuilding>(
                                    *this, cid);
                tt->setMonument(b.get());
                const int d = rotate ? 1 : 0;
                this->build(tx - d, ty + d, 2, 2, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
            } break;
            case eSanctEleType::sanctuary: {
                const auto tb = e::make_shared<eTempleBuilding>(
                            t.fId, *this, cid);
                tb->setMonument(b.get());
                b->registerElement(tb);
                if(rotate) {
                    this->build(tx - 2, ty + 2, 4, 4, cid, pid, editorDisplay, [tb]() { return tb; });
                } else {
                    this->build(tx + 1, ty - 1, 4, 4, cid, pid, editorDisplay, [tb]() { return tb; });
                }
            } break;
            case eSanctEleType::tile: {
                const auto tt = e::make_shared<eTempleTileBuilding>(
                                    t.fId, *this, cid);
                tt->setMonument(b.get());
                this->build(tx, ty, 1, 1, cid, pid, editorDisplay, [tt]() { return tt; });
                b->registerElement(tt);
                if(t.fWarrior) b->addWarriorTile(tile);
            } break;
            case eSanctEleType::stairs: {
                tile->setSeed(t.fId);
                tile->setWalkableElev(true);
            } break;
            case eSanctEleType::none:
                break;
            }
        }
    }
    for(const auto& tv : h->fTiles) {
        for(const auto& t : tv) {
            const int tx = minX + t.fX;
            const int ty = minY + t.fY;
            const auto tile = this->tile(tx, ty);
            tile->setAltitude(tile->altitude() + t.fA);
            const auto trr = tile->terrain();
            const bool bldbl = static_cast<bool>(
                                   trr & eTerrain::buildable);
            if(!tile->underBuilding() && bldbl) {
                tile->setUnderBuilding(b);
                b->addUnderBuilding(tile);
            }
        }
    }

    if(!editorDisplay) b->applyOriginalMaterialTotals();
    b->buildingProgressed();

    return true;
}

eDistrictIdTmp::eDistrictIdTmp(eGameBoard& board) :
    mBoard(board), mTmpId(board.currentDistrictId()) {}

eDistrictIdTmp::~eDistrictIdTmp() {
    mBoard.setCurrentDistrictId(mTmpId);
}

void eGameBoard::rebuildAnimalNear(const eBuildingType type,
                                   const int x, const int y,
                                   const ePlayerId pid, const eCityId cid,
                                   const bool editorDisplay) {
    const auto creator = [type](eGameBoard& board) -> stdsptr<eCharacter> {
        if(type == eBuildingType::goat) return e::make_shared<eGoat>(board);
        return e::make_shared<eSheep>(board);
    };
    bool found = false;
    for(int k = 1; k < 12 && !found; k++) {
        eIterateSquare::iterateSquare(k, [&](const int dx, const int dy) {
            const auto t = tile(x + dx, y + dy);
            if(!t) return false;
            found = buildAnimal(t, type, creator, cid, pid, editorDisplay);
            return found;
        });
    }
    // moving an animal is free
    if(found && !editorDisplay) {
        const auto diff = difficulty(pid);
        const int cost = eDifficultyHelpers::buildingCost(diff, type);
        incDrachmas(pid, cost, eFinanceTarget::construction);
    }
}
