// Part of eGameBoard: earthquakes, tidal waves, landslides and lava.
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

eOrientation randomOrientation() {
    std::vector<eOrientation> os{eOrientation::topRight,
                                 eOrientation::bottomRight,
                                 eOrientation::bottomLeft,
                                 eOrientation::topLeft};
    return os[eRand::rand() % os.size()];
}

void eGameBoard::earthquake(eTile* const startTile, const int size) {
    struct eQuakeEnd {
        eTile* fTile = nullptr;
        eOrientation fLastO = randomOrientation();
    };

    const auto quake = std::make_shared<eEarthquake>();
    mEarthquakes.push_back(quake);

    std::queue<eQuakeEnd> ends;
    quake->fStartTile = startTile;
    auto& tiles = quake->fTiles;
    tiles.push_back(startTile);
    ends.push({startTile});
//    std::vector<eOrientation> os{eOrientation::topRight,
//                                 eOrientation::bottomRight,
//                                 eOrientation::bottomLeft,
//                                 eOrientation::topLeft};
//    eRand::randomShuffle(os);
//    for(int i = 0; i < 2; i++) {
//        const auto o = os[i];
//        const auto tt = startTile->neighbour<eTile>(o);
//        ends.push({tt});
//        mEarthquake.push_back(tt);
//    }
    while((int)tiles.size() < size && !ends.empty()) {
        const auto t = ends.front();
        ends.pop();
        std::vector<eOrientation> os{eOrientation::topRight,
                                     eOrientation::bottomRight,
                                     eOrientation::bottomLeft,
                                     eOrientation::topLeft};
        eRand::randomShuffle(os);
        if(eRand::rand() % 7) {
            os.insert(os.begin(), t.fLastO);
        }
        for(const auto o : os) {
            const auto tt = t.fTile->neighbour<eTile>(o);
            if(!tt) continue;
            const auto terr = tt->terrain();
            if(terr == eTerrain::dry ||
               terr == eTerrain::fertile ||
               terr == eTerrain::forest ||
               terr == eTerrain::choppedForest) {
                ends.push({tt, o});
                tiles.push_back(tt);
                if(eRand::rand() % 5 == 0) {
                    ends.push({tt});
                }
                break;
            }
        }
    }
}

bool eGameBoard::duringEarthquake() const {
    return !mEarthquakes.empty();
}

void eGameBoard::earthquakeWaveCollapse(eTile* const t) {
    bool playSound = true;
    if(const auto ub = t->underBuilding()) {
        const auto type = ub->type();
        const bool s = eBuilding::sSanctuaryBuilding(type);
        if(s) {
        } else if(type == eBuildingType::ruins) {
            ub->erase();
        } else {
            if(const auto as = dynamic_cast<eAgoraSpace*>(ub)) {
                const auto a = as->agora();
                a->collapse();
            } else if(const auto v = dynamic_cast<eVendor*>(ub)) {
                const auto a = v->agora();
                a->collapse();
            } else if(const auto r = dynamic_cast<eRoad*>(ub)) {
                const auto a = r->underAgora();
                if(a) a->collapse();
                const auto g = r->underGatehouse();
                if(g) g->collapse();
                r->eBuilding::erase();
                playSound = a || g;
            } else {
                if(type == eBuildingType::park) {
                    scheduleTerrainUpdate();
                }
                ub->collapse();
            }
            if(playSound) eSounds::playCollapseSound();
        }
    }
    for(const auto& c : t->characters()) {
        c->killWithCorpse();
    }
}

void eGameBoard::progressEarthquakes() {
    if(mEarthquakes.empty()) return;
    eSounds::playEarthquakeSound();
    for(int i = 0; i < (int)mEarthquakes.size(); i++) {
        const auto& e = mEarthquakes[i];
        const auto st = e->fStartTile;
        const int stx = st->x();
        const int sty = st->y();
        const auto prcs = [this, e, stx, sty](const int x, const int y) {
            const int tx = stx + x;
            const int ty = sty + y;
            const auto t = tile(tx, ty);
            if(!t) return false;
            const bool r = eVectorHelpers::contains(e->fTiles, t);
            if(!r) return false;
            t->setTerrain(eTerrain::quake);
            t->scheduleNeighboursTerrainUpdate();
            const auto cid = t->cityId();
            const auto c = boardCityWithId(cid);
            if(c) c->incTerrainState();
            earthquakeWaveCollapse(t);
            eVectorHelpers::removeAll(e->fTiles, t);
            return false;
        };
        eIterateSquare::iterateSquare(e->fLastDim, prcs);
        e->fLastDim++;
        if(e->fTiles.empty() || e->fLastDim > 50) {
            eVectorHelpers::remove(mEarthquakes, e);
            i--;
        }
    }
}

void eGameBoard::progressTidalWaves() {
    if(mTidalWaves.empty()) return;
    eSounds::playBeachSound();
    for(int i = 0; i < (int)mTidalWaves.size(); i++) {
        const auto& w = mTidalWaves[i];

        if(w->fLastId >= 0 && w->fLastId < (int)w->fTiles.size()) {
            const std::vector<eWaveDirection>& tiles = w->fTiles[w->fLastId];
            for(const auto& wd : tiles) {
                const auto t = wd.fTile;
                const auto o = wd.fO;
                std::vector<ePathPoint> path;
                eTile* from = nullptr;
                eTile* to = nullptr;
                if(w->fRegres) {
                    from = t;
                    to = t->neighbour<eTile>(!o);
                    t->setTerrain(wd.fSaved);
                } else {
                    from = t->neighbour<eTile>(!o);
                    to = t;
                    t->setTerrain(eTerrain::water);
                }
                earthquakeWaveCollapse(t);
                const auto cid = t->cityId();
                const auto c = boardCityWithId(cid);
                if(c) c->incTerrainState();

                t->scheduleNeighboursTerrainUpdate(2);
                path.push_back({(double)from->x(), (double)from->y(), 0.});
                path.push_back({(double)to->x(), (double)to->y(), 0.});
                const auto m = e::make_shared<eWaveMissile>(*this, path);
                m->incTime(0);
            }
        }

        if(w->fRegres) w->fLastId--;
        else w->fLastId++;
        if(w->fLastId < 0 || w->fLastId >= (int)w->fTiles.size()) {
            if(w->fPermanent || w->fRegres) {
                eVectorHelpers::remove(mTidalWaves, w);
                i--;
            } else {
                w->fLastId--;
                w->fRegres = true;
            }
        }
    }
}

void eGameBoard::sinkLand(const eCityId cid, const int amount) {
    std::vector<eTile*> shore;
    {
        std::vector<eTile*> used;
        std::function<void(eTile* const)> addShore;
        addShore = [&](eTile* const t) {
            const auto tcid = t->cityId();
            if(tcid != cid) return;
            const bool c = eVectorHelpers::contains(used, t);
            if(c) return;
            used.push_back(t);
            if(t->isShoreTile()) shore.push_back(t);
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                addShore(static_cast<eTile*>(n.second));
            }
        };
        const auto c = boardCityWithId(cid);
        if(!c) return;
        const auto& tiles = c->tiles();
        if(tiles.empty()) return;
        addShore(tiles[tiles.size()/2]);
    }
    const auto w = std::make_shared<eTidalWave>();
    {
        std::vector<eTile*> used;
        const auto addNeighs = [&](eTile* const t, const int dist) {
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                const auto tt = static_cast<eTile*>(n.second);
                if(tt->hasWater()) continue;
                const bool c = eVectorHelpers::contains(used, tt);
                if(c) continue;
                while((int)w->fTiles.size() < dist + 1) {
                    w->fTiles.emplace_back();
                }
                used.push_back(tt);
                w->fTiles[dist].push_back({tt, tt->terrain(), n.first});
            }
        };
        for(const auto s : shore) {
            addNeighs(s, 0);
        }
        for(int i = 0; i < (int)w->fTiles.size() && i < amount - 1; i++) {
            for(const auto t : w->fTiles[i]) {
                addNeighs(t.fTile, i + 1);
            }
        }
    }

    if(w->fTiles.empty()) return;
    w->fPermanent = true;
    mTidalWaves.push_back(w);
}

void eGameBoard::addTidalWave(eTile* const startTile,
                              const bool permanent) {
    std::vector<eTile*> shore;
    {
        std::vector<eTile*> used;
        std::function<void(eTile* const)> addShore;
        addShore = [&](eTile* const t) {
            const bool c = eVectorHelpers::contains(used, t);
            if(c) return;
            used.push_back(t);
            if(t->isShoreTile()) shore.push_back(t);
            if(!t->tidalWaveZone()) return;
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                addShore(static_cast<eTile*>(n.second));
            }
        };
        addShore(startTile);
    }
    const auto w = std::make_shared<eTidalWave>();
    {
        std::vector<eTile*> used;
        const auto addNeighs = [&](eTile* const t, const int dist) {
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                const auto tt = static_cast<eTile*>(n.second);
                if(!tt->tidalWaveZone()) continue;
                if(tt->hasWater()) continue;
                const bool c = eVectorHelpers::contains(used, tt);
                if(c) continue;
                while((int)w->fTiles.size() < dist + 1) {
                    w->fTiles.emplace_back();
                }
                used.push_back(tt);
                w->fTiles[dist].push_back({tt, tt->terrain(), n.first});
            }
        };
        for(const auto s : shore) {
            addNeighs(s, 0);
        }
        for(int i = 0; i < (int)w->fTiles.size(); i++) {
            for(const auto t : w->fTiles[i]) {
                addNeighs(t.fTile, i + 1);
            }
        }
    }

    if(w->fTiles.empty()) return;
    w->fPermanent = permanent;
    mTidalWaves.push_back(w);
}

bool eGameBoard::duringTidalWave() const {
    return !mTidalWaves.empty();
}

void eGameBoard::addLandSlide(eTile * const startTile) {
    if(!startTile) return;
    const int baseAltitude = startTile->altitude();
    const int newAltitude = baseAltitude + 1;

    std::vector<eTile*> shore;
    {
        std::vector<eTile*> used;
        std::function<void(eTile* const)> addShore;
        addShore = [&](eTile* const t) {
            const bool c = eVectorHelpers::contains(used, t);
            if(c) return;
            used.push_back(t);
            if(t->isElevationTile()) shore.push_back(t);
            if(!t->landSlideZone()) return;
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                addShore(static_cast<eTile*>(n.second));
            }
        };
        addShore(startTile);
    }
    const auto w = std::make_shared<eLandSlide>();
    {
        std::vector<eTile*> used;
        using eTiles = std::vector<std::vector<eLandSlideDirection>>;
        const auto addNeighs = [&](eTile* const t, const int dist, eTiles& tiles,
                                   const bool downTiles, int& maxTileDist,
                                   int& minTileDist) {
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                const auto tt = static_cast<eTile*>(n.second);
                const int a = tt->altitude();
                if(downTiles && a != baseAltitude) continue;
                if(!downTiles && a != baseAltitude + 2) continue;
                if(downTiles && !tt->landSlideZone()) continue;
                const bool c = eVectorHelpers::contains(used, tt);
                if(c) continue;
                while((int)tiles.size() < dist + 1) {
                    tiles.emplace_back();
                }
                {
                    const int dx = tt->x() - startTile->x();
                    const int dy = tt->y() - startTile->y();
                    const int tileDist = dx*dx + dy*dy;
                    if(tileDist > maxTileDist) {
                        if(downTiles) {
                            maxTileDist = tileDist;
                        } else {
                            continue;
                        }
                    }
                    if(downTiles && tileDist < minTileDist) {
                        minTileDist = tileDist;
                    }
                }
                used.push_back(tt);
                tiles[dist].push_back({tt, newAltitude, n.first});
            }
        };
        int dMaxTileDist = 0;
        int dShoreMinTileDist = 0;
        eTiles downTiles;
        for(const auto s : shore) {
            addNeighs(s, 0, downTiles, true, dMaxTileDist, dShoreMinTileDist);
        }
        for(uint i = 0; i < downTiles.size(); i++) {
            for(const auto t : downTiles[i]) {
                int minTileDist = 0;
                addNeighs(t.fTile, i + 1, downTiles, true, dMaxTileDist, minTileDist);
            }
        }

        dMaxTileDist += dShoreMinTileDist;

        eTiles upTiles;
        for(const auto s : shore) {
            int minTileDist = 0;
            addNeighs(s, 0, upTiles, false, dMaxTileDist, minTileDist);
        }
        for(uint i = 0; i < downTiles.size(); i++) {
            if(i >= upTiles.size()) break;
            for(const auto t : upTiles[i]) {
                int minTileDist = 0;
                addNeighs(t.fTile, i + 1, upTiles, false, dMaxTileDist, minTileDist);
            }
        }

        const int dMax = downTiles.size();
        const int uMax = upTiles.size();
        const int iMax = std::max(dMax, uMax);
        for(int i = 0; i < iMax; i++) {
            auto& v = w->fTiles.emplace_back();
            if(i < dMax) {
                auto& dv = downTiles[i];
                for(const auto& d : dv) {
                    v.push_back(d);
                }
            }
            if(i < uMax) {
                auto& uv = upTiles[i];
                for(const auto& u : uv) {
                    v.push_back(u);
                }
            }
        }
    }

    if(w->fTiles.empty()) return;
    mLandSlides.push_back(w);
}

bool eGameBoard::duringLandSlide() const {
    return !mLandSlides.empty();
}

void eGameBoard::progressLandSlide() {
    if(mLandSlides.empty()) return;
    eSounds::playEarthquakeSound();
    for(int i = 0; i < (int)mLandSlides.size(); i++) {
        const auto& w = mLandSlides[i];

        if(w->fLastId >= 0 && w->fLastId < (int)w->fTiles.size()) {
            const std::vector<eLandSlideDirection>& tiles = w->fTiles[w->fLastId];
            for(const auto& wd : tiles) {
                const auto t = wd.fTile;
                const auto o = wd.fO;
                std::vector<ePathPoint> path;
                eTile* from = nullptr;
                eTile* to = nullptr;
                from = t->neighbour<eTile>(!o);
                to = t;
                to->setAltitude(wd.fNewAltitude);
                t->setTerrain(eTerrain::dry);
                earthquakeWaveCollapse(t);
                const auto cid = t->cityId();
                const auto c = boardCityWithId(cid);
                if(c) c->incTerrainState();

                t->scheduleNeighboursTerrainUpdate(2);
                path.push_back({(double)from->x(), (double)from->y(), 0.});
                path.push_back({(double)to->x(), (double)to->y(), 0.});
                const auto m = e::make_shared<eDustMissile>(*this, path);
                m->incTime(0);
            }
        }

        w->fLastId++;
        if(w->fLastId < 0 || w->fLastId >= (int)w->fTiles.size()) {
            eVectorHelpers::remove(mLandSlides, w);
        }
    }
}

void eGameBoard::progressLavaFlow() {
    if(mLavaFlows.empty()) return;
    eSounds::playLavaSound();
    for(int i = 0; i < (int)mLavaFlows.size(); i++) {
        const auto& w = mLavaFlows[i];

        if(w->fLastId >= 0 && w->fLastId < (int)w->fTiles.size()) {
            const std::vector<eLavaDirection>& tiles = w->fTiles[w->fLastId];
            for(const auto& wd : tiles) {
                const auto t = wd.fTile;
                const auto o = wd.fO;
                std::vector<ePathPoint> path;
                eTile* from = nullptr;
                eTile* to = nullptr;
                from = t->neighbour<eTile>(!o);
                to = t;
                t->setTerrain(eTerrain::lava);
                earthquakeWaveCollapse(t);
                const auto cid = t->cityId();
                const auto c = boardCityWithId(cid);
                if(c) c->incTerrainState();

                t->scheduleNeighboursTerrainUpdate(2);
                path.push_back({(double)from->x(), (double)from->y(), 0.});
                path.push_back({(double)to->x(), (double)to->y(), 0.});
                const auto m = e::make_shared<eLavaMissile>(*this, path);
                m->incTime(0);
            }
        }

        w->fLastId++;
        if(w->fLastId < 0 || w->fLastId >= (int)w->fTiles.size()) {
            eVectorHelpers::remove(mLavaFlows, w);
        }
    }
}

void eGameBoard::addLavaFlow(eTile * const startTile) {
    const auto w = std::make_shared<eLavaFlow>();
    {
        std::vector<eTile*> used;
        std::function<void(eTile* const, const int)> addNeighs;
        addNeighs = [&](eTile* const t, const int dist) {
            const auto ns = t->neighbours(nullptr);
            for(const auto& n : ns) {
                const auto tt = static_cast<eTile*>(n.second);
                if(!tt->lavaZone()) continue;
                if(tt->hasLava()) continue;
                const bool c = eVectorHelpers::contains(used, tt);
                if(c) continue;
                while((int)w->fTiles.size() < dist + 1) {
                    w->fTiles.emplace_back();
                }
                used.push_back(tt);
                w->fTiles[dist].push_back({tt, n.first});
            }
        };
        addNeighs(startTile, 0);
        for(int i = 0; i < (int)w->fTiles.size(); i++) {
            for(const auto t : w->fTiles[i]) {
                addNeighs(t.fTile, i + 1);
            }
        }
    }

    if(w->fTiles.empty()) return;
    mLavaFlows.push_back(w);
}

bool eGameBoard::duringLavaFlow() const {
    return !mLavaFlows.empty();
}
