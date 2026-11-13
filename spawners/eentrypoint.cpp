#include "eentrypoint.h"

#include "engine/egameboard.h"
#include "characters/esettler.h"
#include "characters/actions/esettleraction.h"

#include "enumbers.h"

#include <algorithm>

eEntryPoint::eEntryPoint(const int id,
                         eTile* const tile,
                         eGameBoard& board) :
    eSpawner(eBannerTypeS::entryPoint, id, tile,
             __INT_MAX__, 500, board) {}

void eEntryPoint::incTime(const int by) {
    auto& board = eEntryPoint::board();
    const auto tile = this->tile();
    const auto cid = tile->cityId();
    // Original immigration (exe 0x00567450): newcomers per cycle (16 a
    // month) by city mood: 71+ 12, 61-70 9, 50-60 6, below 50 none; they
    // arrive in groups of 4
    const int mood = board.popularity(cid);
    int perCycle = 0;
    if(mood >= 71) perCycle = 12;
    else if(mood >= 61) perCycle = 9;
    else if(mood >= 50) perCycle = 6;
    const int cycle = (365*eNumbers::sDayLength)/(12*16);
    mImmigrationClosed = perCycle == 0;
    setSpawnPeriod(perCycle > 0 ? (4*cycle)/perCycle : cycle);
    eSpawner::incTime(by);
}

void eEntryPoint::spawn(eTile* const tile) {
    auto& board = eEntryPoint::board();
    const auto cid = tile->cityId();
    const auto& ivs = board.invasionHandlers(cid);
    if(!ivs.empty()) return;
    if(mImmigrationClosed) return;
    const auto limit = board.immigrationLimit(cid);
    if(limit != eImmigrationLimitedBy::none &&
       limit != eImmigrationLimitedBy::lackOfVacancies) {
        return;
    }
    const auto popData = board.populationData(cid);
    if(!popData) return;
    const int v = popData->vacancies();
    const int s = popData->settlers();
    if(s >= v) return;
    const auto b = e::make_shared<eSettler>(board);
    b->setBothCityIds(cid);
    b->setVisible(false);
    b->changeTile(tile);
    const auto a = e::make_shared<eSettlerAction>(b.get());
    a->setNumberPeople(std::min(4, v - s));
    b->setAction(a);
}

void eSS_spawnFinish::call() {}

eRiverEntryPoint::eRiverEntryPoint(const int id, eTile * const tile,
                                   eGameBoard &board) :
    eBanner(eBannerTypeS::riverEntryPoint, id, tile, board) {}
