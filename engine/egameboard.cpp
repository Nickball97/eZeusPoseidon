
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

eGameBoard::eGameBoard(eWorldBoard& world) :
    mWorld(world), mThreadPool(*this) {
    const auto types = eResourceTypeHelpers::extractResourceTypes(
                           eResourceType::allBasic);
    for(const auto type : types) {
        mPrices[type] = eResourceTypeHelpers::defaultPrice(type);
    }
}

eGameBoard::~eGameBoard() {
    mRegisterBuildingsEnabled = false;
    clear();
}

void eGameBoard::initialize(const int w, const int h) {
    waitUntilFinished();
    mThreadPool.initialize(w, h);

    clear();
    mTiles.reserve(w);
    for(int x = 0; x < w; x++) {
        auto& yArr = mTiles.emplace_back();
        yArr.reserve(h);
        for(int y = 0; y < h; y++) {
            int tx;
            int ty;
            eTileHelper::dtileIdToTileId(x, y, tx, ty);
            const auto tile = new eTile(tx, ty, x, y, *this);
            yArr.push_back(tile);
        }
    }
    mWidth = w;
    mHeight = h;

    mAppealMap.initialize(0, 0, w, h);

    updateNeighbours();
//    updateMarbleTiles();
    scheduleTerrainUpdate();
    for(const auto& c : mCitiesOnBoard) {
        c->clearTiles();
    }
}

void eGameBoard::resize(const int w, const int h) {
    waitUntilFinished();
    mThreadPool.initialize(w, h);

    mTiles.reserve(w);

    for(int x = w; x < mWidth; x++) {
        mTiles.pop_back();
    }
    for(int x = 0; x < w; x++) {
        auto& yArr = mTiles[x];
        for(int y = h; y < mHeight; y++) {
            yArr.pop_back();
        }
    }

    for(int x = 0; x < w; x++) {
        auto& yArr = x < mWidth ? mTiles[x] : mTiles.emplace_back();
        yArr.reserve(h);
        for(int y = 0; y < h; y++) {
            if(x < mWidth && y < mHeight) continue;
            int tx;
            int ty;
            eTileHelper::dtileIdToTileId(x, y, tx, ty);
            const auto tile = new eTile(tx, ty, x, y, *this);
            yArr.push_back(tile);
        }
    }
    mWidth = w;
    mHeight = h;

    mAppealMap.initialize(0, 0, w, h);

    updateNeighbours();
//    updateMarbleTiles();
    scheduleTerrainUpdate();
    updateTerritoryBorders();
}

void eGameBoard::clear() {
    for(const auto c : mCharacters) {
        c->kill();
    }
    emptyRubbish();
    for(const auto& c : mCitiesOnBoard) {
        c->clearHippodromes();
    }
    for(const auto& x : mTiles) {
        for(const auto y : x) {
            delete y;
        }
    }
    mTiles.clear();
    mWidth = 0;
    mHeight = 0;
    emptyRubbish();
    std::vector<stdsptr<eBoardCity>> cities;
    std::swap(mCitiesOnBoard, cities);
    cities.clear();
    mPlayersOnBoard.clear();
    mDefeatedBy.clear();
    mEarthquakes.clear();
    mTidalWaves.clear();
    mGoals.clear();
    emptyRubbish();
}

void eGameBoard::setWorldDirection(const eWorldDirection dir) {
    mDirection = dir;
    scheduleTerrainUpdate();
}

eTile* eGameBoard::rotateddtile(const int x, const int y) const {
    int rx;
    int ry;
    eTileHelper::rotatedDTileIdToDTileId(
            x, y, rx, ry, mDirection, mWidth, mHeight);
    return dtile(rx, ry);
}

int eGameBoard::rotatedWidth() const {
    if(mDirection == eWorldDirection::N) {
        return mWidth;
    } else if(mDirection == eWorldDirection::E) {
        return mHeight/2;
    } else if(mDirection == eWorldDirection::S) {
        return mWidth;
    } else { // if(mDirection == eWorldDirection::W) {
        return mHeight/2;
    }}

int eGameBoard::rotatedHeight() const {
    if(mDirection == eWorldDirection::N) {
        return mHeight;
    } else if(mDirection == eWorldDirection::E) {
        return 2*mWidth;
    } else if(mDirection == eWorldDirection::S) {
        return mHeight;
    } else { // if(mDirection == eWorldDirection::W) {
        return 2*mWidth;
    }
}

void eGameBoard::iterateOverAllTiles(const eTileAction& a) {
    const int height = rotatedHeight();
    const int width = rotatedWidth();
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            const auto t = rotateddtile(x, y);
            if(!t) continue;
            a(t);
        }
    }
}

void eGameBoard::scheduleAppealMapUpdate(const eCityId cid) {
    if(mEditorMode) return;
    mUpdateAppeal[cid].fV = true;
}

void eGameBoard::updateAppealMapIfNeeded() {
    for(auto& c : mUpdateAppeal) {
        bool& fV = c.second.fV;
        if(!fV) continue;
        fV = false;
        const auto cid = c.first;
        const auto finish = [this, cid](eHeatMap& map) {
            const auto c = boardCityWithId(cid);
            const auto& tiles = c->tiles();
            for(const auto t : tiles) {
                const int dx = t->dx();
                const int dy = t->dy();
                const bool e = map.enabled(dx, dy);
                const double h = map.heat(dx, dy);
                mAppealMap.set(dx, dy, e, h);
            }
        };
        const auto cc = boardCityWithId(cid);
        const auto rect = cc->tileBRect();
        const auto task = new eHeatMapTask(cid, rect,
                                           eHeatGetters::appeal, finish);
        mThreadPool.queueTask(task);
    }
}

void eGameBoard::setRegisterBuildingsEnabled(const bool e) {
    mRegisterBuildingsEnabled = e;
}

void eGameBoard::setButtonsVisUpdater(const eAction& u) {
    mButtonVisUpdater = u;
}

eBuilding* eGameBoard::buildingAt(const int x, const int y) const {
    const auto t = tile(x, y);
    if(!t) return nullptr;
    return t->underBuilding();
}

bool eGameBoard::isShutDown(const eCityId cid,
                            const eResourceType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->isShutDown(type);
}

bool eGameBoard::isShutDown(const eCityId cid,
                            const eBuildingType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->isShutDown(type);
}

std::vector<eBuilding*> eGameBoard::buildings(
        const eCityId cid, const eBuildingValidator& v) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->buildings(v);
}

std::vector<eBuilding*> eGameBoard::buildings(
        const eCityId cid, const eBuildingType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->buildings(type);
}

int eGameBoard::countBuildings(
        const eCityId cid, const eBuildingValidator& v) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->countBuildings(v);
}

int eGameBoard::countBuildings(
        const eCityId cid, const eBuildingType t) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->countBuildings(t);
}

bool eGameBoard::hasBuilding(
        const eCityId cid, const eBuildingType t) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->hasBuilding(t);
}

int eGameBoard::countAllowed(
        const eCityId cid, const eBuildingType t) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->countAllowed(t);
}

eBuilding* eGameBoard::randomBuilding(
        const eCityId cid, const eBuildingValidator& v) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->randomBuilding(v);
}

std::vector<eBuilding*> eGameBoard::commemorativeBuildings(
        const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->commemorativeBuildings();
}

bool eGameBoard::supportsBuilding(const eCityId cid,
                                  const eBuildingMode mode) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->supportsBuilding(mode);
}

bool eGameBoard::availableBuilding(const eCityId cid,
                                   const eBuildingType type,
                                   const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->availableBuilding(type, id);
}

void eGameBoard::built(const eCityId cid,
                       const eBuildingType type,
                       const int id) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->built(type, id);
}

void eGameBoard::destroyed(const eCityId cid,
                           const eBuildingType type,
                           const int id) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->destroyed(type, id);
}

void eGameBoard::allow(const eCityId cid,
                       const eBuildingType type,
                       const int id) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->allow(type, id);
}

void eGameBoard::disallow(const eCityId cid,
                          const eBuildingType type,
                          const int id) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->disallow(type, id);
}

void eGameBoard::updateButtonsVisibility() {
    if(mButtonVisUpdater) mButtonVisUpdater();
}

bool eGameBoard::supportsResource(
        const eCityId cid, const eResourceType rt) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->supportsResource(rt);
}

eResourceType eGameBoard::supportedResources(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return eResourceType::none;
    return c->supportedResources();
}

int eGameBoard::wonGames(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->wonGames();
}

int eGameBoard::horses() const {
    int h = 0;
    for(const auto b : mTimedBuildings) {
        if(b->type() != eBuildingType::horseRanch) continue;
        const auto r = static_cast<eHorseRanch*>(b);
        h += r->horseCount();
    }
    return h;
}

void eGameBoard::planAction(ePlannedAction* const a) {
    mPlannedActions.emplace_back(a);
}

void eGameBoard::restockMarbleTiles() {
    for(const auto& mt : mMarbleTiles) {
        mt.restock();
    }
}

bool eGameBoard::eMarbleTiles::contains(eTile* const tile) const {
    return eVectorHelpers::contains(fTiles, tile);
}

void eGameBoard::eMarbleTiles::add(eTile* const tile) {
    fTiles.push_back(tile);
}

void eGameBoard::eMarbleTiles::addWithNeighbours(eTile* const tile) {
    for(int x = -1; x <= 1; x++) {
        for(int y = -1; y <= 1; y++) {
            const auto n = tile->tileRel<eTile>(x, y);
            const auto terr = n->terrain();
            if(terr != eTerrain::marble) return;
            const bool c = contains(n);
            if(c) continue;
            add(n);
            addWithNeighbours(n);
        }
    }
}

void eGameBoard::eMarbleTiles::restock() const {
    int maxLevel = 0;
    for(const auto t : fTiles) {
        if(t->resource() > 0) return;
        const int l = t->marbleLevel();
        if(l > maxLevel) maxLevel = l;
    }
    for(const auto t : fTiles) {
        const int l = t->marbleLevel();
        if(l == maxLevel) {
            if(maxLevel == 2) {
                t->setResource(99999);
            } else {
                const bool e = eMarbleTile::edge(t);
                if(!e) t->setResource(1);
            }
        }
    }
}

void eGameBoard::updateMarbleTiles() {
    mMarbleTiles.clear();
    iterateOverAllTiles([&](eTile* const t) {
        const auto terr = t->terrain();
        if(terr != eTerrain::marble) return;
        for(const auto& mt : mMarbleTiles) {
            const bool c = mt.contains(t);
            if(c) return;
        }
        auto& mt = mMarbleTiles.emplace_back();
        mt.addWithNeighbours(t);
    });
}

void eGameBoard::restockBlackMarbleTiles() {
    for(const auto& mt : mMarbleTiles) {
        mt.restock();
    }
}

void eGameBoard::updateBlackMarbleTiles() {
    mBlackMarbleTiles.clear();
    iterateOverAllTiles([&](eTile* const t) {
        const auto terr = t->terrain();
        if(terr != eTerrain::blackMarble) return;
        for(const auto& mt : mBlackMarbleTiles) {
            const bool c = mt.contains(t);
            if(c) return;
        }
        auto& mt = mBlackMarbleTiles.emplace_back();
        mt.addWithNeighbours(t);
    });
}

void eGameBoard::allowHero(const eCityId cid, const eHeroType heroType,
                           const std::string& reason) {
    const auto hallType = eHerosHall::sHeroTypeToHallType(heroType);
    allow(cid, hallType);
    const auto& inst = eMessages::instance;
    const auto hm = inst.heroMessages(heroType);
    if(!hm) return;
    eEventData ed(cid);
    auto msg = hm->fHallAvailable;
    eStringHelpers::replaceAll(msg.fFull.fText, "[reason_phrase]",
                               reason.empty() ? msg.fNoReason : reason);
    showMessage(ed, msg);
}

eBuilding* eGameBoard::buildingWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto b : mAllBuildings) {
        const int bio = b->ioID();
        if(bio == id) return b;
    }
    return nullptr;
}

eCharacter* eGameBoard::characterWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto c : mCharacters) {
        const int bio = c->ioID();
        if(bio == id) return c;
    }
    return nullptr;
}

eCharacterAction* eGameBoard::characterActionWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto ca : mCharacterActions) {
        const int bio = ca->ioID();
        if(bio == id) return ca;
    }
    return nullptr;
}

eBanner* eGameBoard::bannerWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto b : mBanners) {
        const int bio = b->ioID();
        if(bio == id) return b;
    }
    return nullptr;
}

eSoldierBanner* eGameBoard::soldierBannerWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto b : mAllSoldierBanners) {
        const int bio = b->ioID();
        if(bio == id) return b;
    }
    return nullptr;
}

eGameEvent* eGameBoard::eventWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto e : mAllGameEvents) {
        const int eio = e->ioID();
        if(eio == id) return e;
    }
    return nullptr;
}

eInvasionHandler* eGameBoard::invasionHandlerWithIOID(const int id) const {
    for(const auto& c : mCitiesOnBoard) {
        const auto i = c->invasionHandlerWithIOID(id);
        if(i) return i;
    }
    return nullptr;
}

eTile* eGameBoard::monsterTile(const eCityId cid, const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->monsterTile(id);
}

eTile* eGameBoard::landInvasionTile(const eCityId cid, const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->landInvasionTile(id);
}

eTile* eGameBoard::seaInvasionTile(const eCityId cid, const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->seaInvasionTile(id);
}

eTile* eGameBoard::invasionTile(const eCityId cid, const int id) const {
    if(id > 7) return seaInvasionTile(cid, id);
    return landInvasionTile(cid, id);
}

eTile* eGameBoard::disasterTile(const eCityId cid, const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->disasterTile(id);
}

eTile *eGameBoard::landSlideTile(const eCityId cid, const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->landSlideTile(id);
}

int eGameBoard::addResource(const eCityId cid,
                            const eResourceType type,
                            const int count) {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->addResource(type, count);
}

int eGameBoard::spaceForResource(const eCityId cid,
                                 const eResourceType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->spaceForResource(type);
}

int eGameBoard::maxSingleSpaceForResource(
        const eCityId cid,
        const eResourceType type,
        eStorageBuilding** b) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->maxSingleSpaceForResource(type, b);
}

int eGameBoard::maxMonumentSpaceForResource(
        const eCityId cid,
        eMonument** b) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->maxMonumentSpaceForResource(b);
}

void eGameBoard::waitUntilFinished() {
    while(!mThreadPool.finished()) {
        mThreadPool.waitFinished();
        mThreadPool.handleFinished();
    }
}

int eGameBoard::price(const eResourceType type) const {
    const auto it = mPrices.find(type);
    if(it == mPrices.end()) return 0;
    return it->second;
}

void eGameBoard::incPrice(const eResourceType type, const int by) {
    mPrices[type] += by;
}

void eGameBoard::changeWage(const int per) {
    const double mult = 1 + per/100.;
    mWageMultiplier *= mult;
}

void eGameBoard::updateTerritoryBorders() {
    iterateOverAllTiles([](eTile* const tile) {
        tile->updateTerritoryBorder();
    });
    for(const auto& c : mCitiesOnBoard) {
        c->updateTiles();
    }
}

void eGameBoard::assignAllTerritory(const eCityId cid) {
    iterateOverAllTiles([cid](eTile* const tile) {
        tile->setCityId(cid);
    });
    updateTerritoryBorders();
}

std::vector<eCityId> eGameBoard::personPlayerCitiesOnBoard() const {
    return playerCitiesOnBoard(personPlayer());
}

ePlayerId eGameBoard::cityIdToPlayerId(const eCityId cid) const {
    return mWorld.cityIdToPlayerId(cid);
}

eTeamId eGameBoard::cityIdToTeamId(const eCityId cid) const {
    const auto pid = cityIdToPlayerId(cid);
    return playerIdToTeamId(pid);
}

eTeamId eGameBoard::playerIdToTeamId(const ePlayerId pid) const {
    return mWorld.playerIdToTeamId(pid);
}

void eGameBoard::moveCityToPlayer(const eCityId cid, const ePlayerId pid) {
    const auto oldPid = cityIdToPlayerId(cid);
    const auto c = boardCityWithId(cid);
    if(c) {
        if(oldPid == ePlayerId::neutralAggresive ||
           oldPid == ePlayerId::neutralFriendly) {
            mActiveCitiesOnBoard.push_back(c);
            c->acquired();
        }
        c->sendAllReinforcementsHome();
    }
    return mWorld.moveCityToPlayer(cid, pid);
}

void eGameBoard::setPlayerTeam(const ePlayerId pid, const eTeamId tid) {
    mWorld.setPlayerTeam(pid, tid);
}

std::vector<eCityId> eGameBoard::playerCities(const ePlayerId pid) const {
    return mWorld.playerCities(pid);
}

eCityId eGameBoard::currentCityId() const {
    return mWorld.currentCityId();
}

stdsptr<eWorldCity> eGameBoard::currentCity() const {
    return mWorld.currentCity();
}

eCityId eGameBoard::playerCapital(const ePlayerId pid) const {
    return mWorld.playerCapital(pid);
}

std::vector<eCityId> eGameBoard::playerCitiesOnBoard(const ePlayerId pid) const {
    std::vector<eCityId> result;
    for(const auto& c : mCitiesOnBoard) {
        const auto cid = c->id();
        const auto ppid = cityIdToPlayerId(cid);
        if(pid == ppid) {
            result.push_back(cid);
        }
    }
    return result;
}

ePlayerId eGameBoard::personPlayer() const {
    return mWorld.personPlayer();
}

eCityId eGameBoard::personPlayerCapital() const {
    const auto ppid = personPlayer();
    return playerCapital(ppid);
}

eBoardCity* eGameBoard::boardCityWithId(const eCityId cid) const {
    for(const auto& c : mCitiesOnBoard) {
        if(c->id() == cid) return c.get();
    }
    return nullptr;
}

SDL_Rect eGameBoard::boardCityTileBRect(const eCityId cid) const {
    for(const auto& c : mCitiesOnBoard) {
        if(c->id() == cid) return c->tileBRect();
    }
    return SDL_Rect{0, 0, 0, 0};
}

eBoardPlayer* eGameBoard::boardPlayerWithId(const ePlayerId pid) const {
    for(const auto& p : mPlayersOnBoard) {
        if(p->id() == pid) return p.get();
    }
    return nullptr;
}

std::vector<eCityId> eGameBoard::citiesOnBoard() const {
    std::vector<eCityId> result;
    for(const auto& c : mCitiesOnBoard) {
        result.push_back(c->id());
    }
    return result;
}

std::vector<ePlayerId> eGameBoard::playersOnBoard() const {
    std::vector<ePlayerId> result;
    for(const auto& c : mCitiesOnBoard) {
        const auto cid = c->id();
        const auto pid = cityIdToPlayerId(cid);
        if(pid == ePlayerId::neutralFriendly ||
           pid == ePlayerId::neutralAggresive) continue;
        if(eVectorHelpers::contains(result, pid)) continue;
        result.push_back(pid);
    }
    return result;
}

std::string eGameBoard::cityName(const eCityId cid) const {
    return mWorld.cityName(cid);
}

std::vector<eCityId> eGameBoard::allyCidsNotOnBoard(const ePlayerId pid) const {
    std::vector<eCityId> result;
    const auto tid = playerIdToTeamId(pid);
    const auto& cities = mWorld.cities();
    for(const auto& c : cities) {
        const bool onBoard = c->isOnBoard();
        if(onBoard) continue;
        const auto cid = c->cityId();
        const auto cpid = cityIdToPlayerId(cid);
        if(cpid == pid) continue;
        const auto ctid = playerIdToTeamId(cpid);
        if(tid != ctid) continue;
        result.push_back(cid);
    }
    return result;
}

std::vector<eCityId> eGameBoard::enemyCidsOnBoard(const eTeamId ptid) const {
    std::vector<eCityId> result;
    for(const auto c : mActiveCitiesOnBoard) {
        const auto cid = c->id();
        const auto ctid = cityIdToTeamId(cid);
        if(ctid == eTeamId::neutralFriendly) continue;
        if(ctid == eTeamId::neutralAggresive) continue;
        if(ctid == ptid) continue;
        result.push_back(cid);
    }
    return result;
}

void eGameBoard::updatePlayersOnBoard() {
    const auto ps = mPlayersOnBoard;
    for(const auto& p : ps) {
        const auto pid = p->id();
        const auto cids = playerCitiesOnBoard(pid);
        if(cids.empty()) removePlayerFromBoard(pid);
    }
    for(const auto& c : mCitiesOnBoard) {
        const auto cid = c->id();
        const auto pid = cityIdToPlayerId(cid);
        if(pid == ePlayerId::neutralAggresive ||
           pid == ePlayerId::neutralFriendly) continue;
        const auto p = boardPlayerWithId(pid);
        if(p) continue;
        addPlayerToBoard(pid);
    }
}

eBoardPlayer* eGameBoard::addPlayerToBoard(const ePlayerId pid) {
    const auto p = std::make_shared<eBoardPlayer>(pid, *this);
    mPlayersOnBoard.push_back(p);
    return p.get();
}

void eGameBoard::removePlayerFromBoard(const ePlayerId pid) {
    for(auto it = mPlayersOnBoard.begin(); it < mPlayersOnBoard.end(); it++) {
        const auto p = it->get();
        if(p->id() != pid) continue;
        mPlayersOnBoard.erase(it);
        return;
    }
}

eBoardCity* eGameBoard::addCityToBoard(const eCityId cid) {
    const auto c = std::make_shared<eBoardCity>(cid, *this);
    mCitiesOnBoard.push_back(c);
    const auto pid = cityIdToPlayerId(cid);
    if(pid != ePlayerId::neutralFriendly &&
       pid != ePlayerId::neutralAggresive) {
        mActiveCitiesOnBoard.push_back(c.get());
    }
    mThreadPool.addBoard(cid);
    return c.get();
}

void eGameBoard::removeCityFromBoard(const eCityId cid) {
    for(auto it = mActiveCitiesOnBoard.begin(); it < mActiveCitiesOnBoard.end(); it++) {
        const auto c = *it;
        if(c->id() != cid) continue;
        mActiveCitiesOnBoard.erase(it);
        break;
    }
    for(auto it = mCitiesOnBoard.begin(); it < mCitiesOnBoard.end(); it++) {
        const auto c = it->get();
        if(c->id() != cid) continue;
        mCitiesOnBoard.erase(it);
        break;
    }
    mThreadPool.removeBoard(cid);
}

void eGameBoard::startPlague(eSmallHouse* const h) {
    if(!h) return;
    const auto cid = h->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->startPlague(h);
    eEventData ed(cid);
    ed.fTile = h->centerTile();
    event(eEvent::plague, ed);
}

stdsptr<ePlague> eGameBoard::plagueForHouse(eSmallHouse* const h) {
    if(!h) return nullptr;
    const auto cid = h->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->plagueForHouse(h);
}

void eGameBoard::healPlague(const stdsptr<ePlague>& p) {
    if(!p) return;
    const auto cid = p->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->healPlague(p);
}

void eGameBoard::healHouse(eSmallHouse* const h) {
    const auto p = plagueForHouse(h);
    if(p) {
        if(p->houseCount() == 1) healPlague(p);
        else p->healHouse(h);
    } else {
        h->setPlague(false);
    }
}

eGameBoard::ePlagues eGameBoard::plagues(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->plagues();
}

stdsptr<ePlague> eGameBoard::nearestPlague(
        const eCityId cid,
        const int tx, const int ty,
        int& dist) const {
    dist = 100000;
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->nearestPlague(tx, ty, dist);
}

void eGameBoard::updateMusic() {
    bool battle = false;
    const auto cids = personPlayerCitiesOnBoard();
    for(const auto cid : cids) {
        const auto c = boardCityWithId(cid);
        if(!c) continue;
        const bool i = c->hasActiveInvasions();
        if(i) {
            battle = true;
            break;
        }
        const bool a = !c->attackingGods().empty();
        if(a) {
            battle = true;
            break;
        }

        for(const auto m : c->monsters()) {
            const auto a = m->action();
            if(const auto ma = dynamic_cast<eMonsterAction*>(a)) {
                const auto stage = ma->stage();
                if(stage == eMonsterAttackStage::none ||
                   stage == eMonsterAttackStage::wait) {
                    continue;
                }
                battle = true;
                break;
            }
        }
        if(battle) break;
    }
    if(battle) {
        eMusic::playRandomBattleMusic();
    } else {
        eMusic::playRandomMusic();
    }
}

eTile* eGameBoard::entryPoint(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->entryPoint();
}

eTile* eGameBoard::exitPoint(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->exitPoint();
}

eTile *eGameBoard::riverEntryPoint(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->riverEntryPoint();
}

eTile *eGameBoard::riverExitPoint(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->riverExitPoint();
}

void eGameBoard::editorClearBuildings() {
    for(const auto& c : mCitiesOnBoard) {
        c->editorClearBuildings();
    }
}

void eGameBoard::editorDisplayBuildings() {
    for(const auto& c : mCitiesOnBoard) {
        c->editorDisplayBuildings();
    }
}

void eGameBoard::saveEditorCityPlan() {
    for(const auto& c : mCitiesOnBoard) {
        c->saveEditorCityPlan();
    }
}

void eGameBoard::setCurrentDistrictId(const int id) {
    mCurrentDistrictId = id;
    for(const auto& c : mCitiesOnBoard) {
        c->setCurrentDistrictId(id);
    }
}

bool eGameBoard::atlantean(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->atlantean();
}

bool eGameBoard::setAtlantean(const eCityId cid, const bool a) {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    c->setAtlantean(a);
    return true;
}

eTile* eGameBoard::tile(const int x, const int y) const {
    int dtx;
    int dty;
    eTileHelper::tileIdToDTileId(x, y, dtx, dty);
    return dtile(dtx, dty);
}

eTile* eGameBoard::dtile(const int x, const int y) const {
    if(x < 0 || x >= mWidth) return nullptr;
    if(y < 0 || y >= mHeight) return nullptr;
    return mTiles[x][y];
}

double eGameBoard::taxRateF(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 1.;
    return city->taxRateF();
}

eTaxRate eGameBoard::taxRate(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return eTaxRate::normal;
    return city->taxRate();
}

eWageRate eGameBoard::wageRate(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return eWageRate::normal;
    return city->wageRate();
}

void eGameBoard::setTaxRate(const eCityId cid, const eTaxRate tr) {
    const auto city = boardCityWithId(cid);
    if(!city) return;
    return city->setTaxRate(tr);
}

void eGameBoard::setWageRate(const eCityId cid, const eWageRate wr) {
    const auto city = boardCityWithId(cid);
    if(!city) return;
    return city->setWageRate(wr);
}

int eGameBoard::taxesPaidThisYear(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->taxesPaidThisYear();
}

int eGameBoard::taxesPaidLastYear(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->taxesPaidThisYear();
}

int eGameBoard::peoplePaidTaxesThisYear(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->taxesPaidThisYear();
}

int eGameBoard::peoplePaidTaxesLastYear(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->taxesPaidThisYear();
}

int eGameBoard::philosophyResearchCoverage(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->philosophyResearchCoverage();
}

int eGameBoard::athleticsLearningCoverage(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->athleticsLearningCoverage();
}

int eGameBoard::dramaAstronomyCoverage(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->dramaAstronomyCoverage();
}

int eGameBoard::allCultureScienceCoverage(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->allCultureScienceCoverage();
}

int eGameBoard::taxesCoverage(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->taxesCoverage();
}

int eGameBoard::unrest(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->unrest();
}

int eGameBoard::popularity(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->popularity();
}

int eGameBoard::health(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->health();
}

eCityFinances eGameBoard::finances(const eCityId cid) const {
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return eCityFinances();
    return p->finances();
}

int eGameBoard::maintanance(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return 0;
    return city->maintanance();
}

int eGameBoard::drachmas(const ePlayerId pid) const {
    const auto player = boardPlayerWithId(pid);
    if(!player) return 0;
    return player->drachmas();
}

void eGameBoard::incDrachmas(const ePlayerId pid, const int by,
                             const eFinanceTarget t) {
    const auto player = boardPlayerWithId(pid);
    if(!player) return;
    return player->incDrachmas(by, t);
}

void eGameBoard::setDrachmas(const ePlayerId pid, const int to) {
    const auto player = boardPlayerWithId(pid);
    if(!player) return;
    return player->setDrachmas(to);
}

std::vector<eAgoraBase*> eGameBoard::agoras(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return {};
    return city->agoras();
}

void eGameBoard::incTime(const int by) {
    if(mEpisodeLost) return;
    updateUndo(by);
    const int dayLen = eNumbers::sDayLength;
    { // autosave
        const int time = mTime + by;
        bool nextMonth = false;
        bool nextYear = false;
        const int nd = time/dayLen;
        auto date = mDate;
        date.nextDays(nd, nextMonth, nextYear);
        if(nextYear && date.year() > mSavedYear) {
            mSavedYear = date.year();
            if(mAutosaver) mAutosaver();
        }
    }

    const int iMax = mPlannedActions.size() - 1;
    for(int i = iMax; i >= 0; i--) {
        const auto a = mPlannedActions[i];
        a->incTime(by, *this);
        if(a->finished()) {
            mPlannedActions.erase(mPlannedActions.begin() + i);
            delete a;
        }
    }

//    if(mTotalTime == 0) {
////        receiveRequest(mWorldBoard->cities()[0],
////                            eResourceType::fleece, 9, 0);
//        planGiftFrom(mWorldBoard->cities()[0],
//                     eResourceType::fleece, 16);
//    }

//    for(const auto& p : mPlagues) {
//        const int r = eRand::rand() % 5000;
//        const bool spread = r/by == 0;
//        if(spread) p->randomSpread();
//    }

    mProgressEarthquakes += by;
    const int earthquakeWait = eNumbers::sEarthquakeProgressPeriod;
    if(mProgressEarthquakes > earthquakeWait) {
        mProgressEarthquakes -= earthquakeWait;
        progressEarthquakes();
    }

    mProgressWaves += by;
    const int tidalWaveWait = 250;
    if(mProgressWaves > tidalWaveWait) {
        mProgressWaves -= tidalWaveWait;
        progressTidalWaves();
    }

    mProgressLavaFlows += by;
    const int lavaFlowWait = 250;
    if(mProgressLavaFlows > lavaFlowWait) {
        mProgressLavaFlows -= lavaFlowWait;
        progressLavaFlow();
    }

    mProgressLandSlides += by;
    const int landSlideWait = 250;
    if(mProgressLandSlides > landSlideWait) {
        mProgressLandSlides -= landSlideWait;
        progressLandSlide();
    }

    mSoldiersUpdate += by;
    const int sup = 1000;
    if(mSoldiersUpdate > sup) {
        mSoldiersUpdate -= sup;
        for(const auto& c : mCitiesOnBoard) {
            c->soldierBannersUpdate();
        }
    }

    for(const auto& c : mActiveCitiesOnBoard) {
        c->incTime(by);
    }

    for(const auto& p : mPlayersOnBoard) {
        p->incTime(by);
        if(mTotalTime == 0) { // start building AI cities
            p->nextMonth();
        }
    }
    mTime += by;
    mTotalTime += by;
    bool nextMonth = false;
    bool nextYear = false;
    const int nd = mTime/dayLen;
    mDate.nextDays(nd, nextMonth, nextYear);
    mTime -= nd*dayLen;

    if(nextYear) {
        for(auto& y : mYearlyProduction) {
            auto& p = y.second;
            p.fLastYear = p.fThisYear;
            p.fThisYear = 0;
            if(p.fLastYear > p.fBest) p.fBest = p.fLastYear;
        }

        for(const auto& c : mActiveCitiesOnBoard) {
            c->nextYear();
        }

        const auto ppcs = personPlayerCitiesOnBoard();
        for(const auto cid : ppcs) {
            auto& defs = mDefeatedBy[cid];
            for(const auto& cc : defs) {
                if(!cc->isRival()) continue;
                const auto rr = e::make_shared<eReceiveRequestEvent>(
                    cid, eGameEventBranch::root, *this);
                const auto type = cc->recTributeType();
                const int count = cc->recTributeCount();
                rr->initialize(0, type, count, cc, false);
                rr->setRequestType(eReceiveRequestType::tribute);
                rr->initializeDate(mDate);
                addRootGameEvent(rr);
            }
        }
    }
    if(nextMonth) {
        for(const auto& c : mActiveCitiesOnBoard) {
            c->nextMonth();
        }
        for(const auto& p : mPlayersOnBoard) {
            p->nextMonth();
        }

        const auto m = mDate.month();
        const int ng = std::abs(mDate.year() % 4);
        const auto game = static_cast<eGames>(ng);
        if(m == eMonth::june) {
            handleGamesBegin(game);
        } else if(m == eMonth::august) {
            handleGamesEnd(game);
        }
    }

    const int ect = 5000;
    mEmploymentCheckTime += by;
    if(mEmploymentCheckTime > ect) {
        mEmploymentCheckTime -= ect;
        const auto ppcs = personPlayerCitiesOnBoard();
        for(const auto cid : ppcs) {
            const auto c = boardCityWithId(cid);
            const auto& emplData = c->employmentData();
            const double employable = emplData.employable();
            const double jobVacs = emplData.totalJobVacancies();
            int emplState = 0;
            if(employable < jobVacs*0.75) {
                emplState = 1;
            } else if(employable > jobVacs*1.25) {
                emplState = -1;
            } else {
                emplState = 0;
            }
            auto& les = mLastEmploymentState[cid];
            if(les.fV != emplState) {
                const auto& inst = eMessages::instance;
                eEventData ed(cid);
                if(emplState == -1) { // unemployed
                    showMessage(ed, inst.fUnemployment);
                } else if(emplState == 1) { // employed
                    showMessage(ed, inst.fEmployees);
                }
            }
            les.fV = emplState;
        }
    }

    // cheat "Ambrosia": the episode is won when the next month starts
    if(nextMonth && mCheatVictory) {
        mCheatVictory = false;
        mGoalsFulfilled = true;
        mGoalsCheckTime = 5050;
    }
    if(nextMonth) {
        mWorld.nextMonth(this);
    }
    if(nextYear) {
        mWorld.nextYear();
        const auto ppid = personPlayer();
        const auto cs = mWorld.getTribute();
        for(const auto& c : cs) {
            if(c->conqueredByRival()) continue;
            tributeFrom(ppid, c, true);
        }
    }
    const auto chars = mCharacters;
    for(const auto c : chars) {
        if(c->isSoldier()) continue;
        c->incTime(by);
    }
    const auto solds = mSoldiers;
    for(const auto c : solds) {
        c->incTime(by);
    }
    const auto build = mTimedBuildings;
    for(const auto b : build) {
        b->incTime(by);
        if(nextMonth) b->nextMonth();
    }
    for(const auto s : mSpawners) {
        s->incTime(by);
    }

    const auto missiles = mMissiles;
    for(const auto m : missiles) {
        m->incTime(by);
    }

    const int goalsCheckWait = 5050;
    mGoalsCheckTime += by;
    if(mGoalsCheckTime > goalsCheckWait) {
        mGoalsCheckTime -= goalsCheckWait;
        const bool f = checkGoalsFulfilled();
        if(f || mGoalsFulfilled) {
            mGoalsFulfilled = true;
            const bool r = handleEpisodeCompleteEvents();
            if(!r && mEpisodeFinishedHandler) {
                mGoalsFulfilled = false;
                mEpisodeFinishedHandler();
            } else {
                mGoalsCheckTime = goalsCheckWait;
            }
        }
    }

    for(const auto& c : mActiveCitiesOnBoard) {
        c->incDistributeEmployees(by);
    }
}

void eGameBoard::step(const int ticks, const bool waitForTasks) {
    beginStep();
    incTime(ticks);
    endStep(waitForTasks);
}

void eGameBoard::beginStep() {
    scheduleDataUpdate();
    updateAppealMapIfNeeded();
    handleFinishedTasks();
}

void eGameBoard::endStep(const bool waitForTasks) {
    emptyRubbish();
    if(waitForTasks) waitUntilFinished();
}

void eGameBoard::incFrame() {
    mFrame++;
}

void eGameBoard::handleFinishedTasks() {
    mThreadPool.handleFinished();
}

void eGameBoard::scheduleDataUpdate() {
    mThreadPool.scheduleDataUpdate();
}

int eGameBoard::population(const ePlayerId pid) const {
    int result = 0;
    const auto cids = playerCitiesOnBoard(pid);
    for(const auto cid : cids) {
        result += population(cid);
    }
    return result;
}

int eGameBoard::population(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->population();
}

ePopulationData* eGameBoard::populationData(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return &c->populationData();
}

eHusbandryData* eGameBoard::husbandryData(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return &c->husbandryData();
}

eEmploymentData* eGameBoard::employmentData(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return &c->employmentData();
}

eEmploymentDistributor* eGameBoard::employmentDistributor(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return &c->employmentDistributor();
}

void eGameBoard::addShutDown(const eCityId cid, const eResourceType type) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->addShutDown(type);
}

void eGameBoard::removeShutDown(const eCityId cid, const eResourceType type) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->removeShutDown(type);
}

int eGameBoard::industryJobVacancies(const eCityId cid, const eResourceType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->industryJobVacancies(type);
}

void eGameBoard::distributeEmployees(
        const eCityId cid, const eSector s) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->distributeEmployees(s);
}

void eGameBoard::distributeEmployees(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->distributeEmployees();
}

void eGameBoard::payTaxes(const eCityId cid, const int d, const int people) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->payTaxes(d, people);
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->incDrachmas(d, eFinanceTarget::taxesIn);
}

void eGameBoard::setDifficulty(const eDifficulty d) {
    std::map<eDifficulty, eDifficulty> rivalDiff =
        {{eDifficulty::beginner, eDifficulty::olympian},
         {eDifficulty::mortal, eDifficulty::titan},
         {eDifficulty::hero, eDifficulty::hero},
         {eDifficulty::titan, eDifficulty::mortal},
         {eDifficulty::olympian, eDifficulty::beginner}};
    std::map<eDifficulty, eDifficulty> allyDiff =
        {{eDifficulty::beginner, eDifficulty::beginner},
         {eDifficulty::mortal, eDifficulty::mortal},
         {eDifficulty::hero, eDifficulty::hero},
         {eDifficulty::titan, eDifficulty::titan},
         {eDifficulty::olympian, eDifficulty::olympian}};
    const auto ppid = personPlayer();
    const auto ptid = playerIdToTeamId(ppid);
    for(const auto& p : mPlayersOnBoard) {
        const auto pid = p->id();
        eDifficulty pd;
        if(pid == ppid) {
            pd = d;
        } else {
            const auto tid = playerIdToTeamId(pid);
            if(tid == ptid) {
                pd = allyDiff[d];
            } else {
                pd = rivalDiff[d];
            }
        }
        setDifficulty(pid, pd);
    }
}

void eGameBoard::setDifficulty(const ePlayerId pid, const eDifficulty d) {
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->setDifficulty(d);
}

eDifficulty eGameBoard::personPlayerDifficulty() const {
    return difficulty(personPlayer());
}

eDifficulty eGameBoard::difficulty(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return eDifficulty::beginner;
    return p->difficulty();
}

void eGameBoard::setDate(const eDate& d) {
    mDate = d;
}

double eGameBoard::appeal(const int tx, const int ty) const {
    return mAppealMap.heat(tx, ty);
}

void eGameBoard::addRubbish(const stdsptr<eObject>& o) {
    mRubbish.push_back(o);
}

void eGameBoard::emptyRubbish() {
    while(!mRubbish.empty()) {
        std::vector<stdsptr<eObject>> r;
        std::swap(mRubbish, r);
    }
}

void eGameBoard::setRequestUpdateHandler(const eAction& ru) {
    mRequestUpdateHandler = ru;
}

void eGameBoard::setEventHandler(const eEventHandler& eh) {
    mEventHandler = eh;
}

void eGameBoard::event(const eEvent e, eEventData& ed) {
    if(mEventHandler) mEventHandler(e, ed);
}

void eGameBoard::setEpisodeFinishedHandler(const eAction& a) {
    mEpisodeFinishedHandler = a;
}

void eGameBoard::setAutosaver(const eAction& a) {
    mAutosaver = a;
}

void eGameBoard::setVisibilityChecker(const eVisibilityChecker& vc) {
    mVisibilityChecker = vc;
}

void eGameBoard::setTipShower(const eTipShower& ts) {
    mTipShower = ts;
}

void eGameBoard::showTip(const ePlayerCityTarget& target,
                         const std::string& tip) const {
    if(mTipShower) mTipShower(target, tip);
}

void eGameBoard::setEnlistForcesRequest(const eEnlistRequest& req) {
    mEnlistRequester = req;
}

bool eGameBoard::ifVisible(eTile* const tile, const eAction& func) const {
    if(!tile) return false;
    if(!mVisibilityChecker) return false;
    const bool r = mVisibilityChecker(tile);
    if(r) func();
    return r;
}

void eGameBoard::setMessageShower(const eMessageShower& msg) {
    mMsgShower = msg;
}

void eGameBoard::showMessage(eEventData& ed,
                             const eMessageType& msg) {
    mMsgShower(ed, msg);
}

void eGameBoard::updateNeighbours() {
    for(int x = 0; x < mWidth; x++) {
        for(int y = 0; y < mHeight; y++) {
            const auto t = mTiles[x][y];
            {
                const int dx = y % 2 == 0 ? -1 : 0;
                t->setTopLeft(dtile(x + dx, y - 1));
                t->setBottomLeft(dtile(x + dx, y + 1));
            }
            {
                const int dx = y % 2 == 0 ? 0 : 1;
                t->setTopRight(dtile(x + dx, y - 1));
                t->setBottomRight(dtile(x + dx, y + 1));
            }
        }
    }
}

void eGameBoard::updateResources(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->updateResources();
}

const eGameBoard::eResources* eGameBoard::resources(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return &c->resources();
}

ePalace* eGameBoard::palace(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->palace();
}

bool eGameBoard::hasPalace(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->hasPalace();
}

int eGameBoard::resourceCount(const eCityId cid,
                              const eResourceType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->resourceCount(type);
}

int eGameBoard::takeResource(const eCityId cid,
                             const eResourceType type,
                             const int count) {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->takeResource(type, count);
}

int eGameBoard::eliteHouses(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->eliteHouses();
}

int eGameBoard::maxSanctuaries(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->maxSanctuaries();
}

std::vector<eSanctuary*> eGameBoard::sanctuaries(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->sanctuaries();
}

eSanctuary* eGameBoard::sanctuary(const eCityId cid, const eGodType god) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->sanctuary(god);
}

std::vector<ePyramid*> eGameBoard::pyramids(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->pyramids();
}

ePyramid* eGameBoard::pyramid(const eCityId cid, const eBuildingType type) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->pyramid(type);
}

std::vector<eHerosHall*> eGameBoard::heroHalls(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->heroHalls();
}

eHerosHall* eGameBoard::heroHall(const eCityId cid, const eHeroType hero) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->heroHall(hero);
}

void eGameBoard::startEpisode(eEpisode* const e,
                              const eWC& lastPlayedColony) {
    if(mTimedBuildings.empty()) { // first episode
        for(const auto s : mSpawners) {
            const auto type = s->type();
            if(type == eBannerTypeS::boar ||
               type == eBannerTypeS::deer ||
               type == eBannerTypeS::wolf) {
                s->spawnMax();
            }
        }
    }
    mDefeatedBy.clear();

    for(const auto& c : mCitiesOnBoard) {
        c->clearAfterLastEpisode();
    }

    mGoals.clear();
    const auto& date = e->fStartDate;
    mSavedYear = date.year();
    setDate(date);
    for(const auto& d : e->fDrachmas) {
        setDrachmas(d.first, d.second);
    }
    mWageMultiplier = e->fWageMultiplier;
    mPrices = e->fPrices;

    for(const auto& c : mCitiesOnBoard) {
        c->startEpisode(e);
    }
    const auto& gs = e->fGoals;
    for(const auto& g : gs) {
        const auto gg = g->makeCopy();
        gg->initializeDate(*this);
        gg->update(*this);
        mGoals.push_back(gg);
    }
    if(lastPlayedColony) {
        const auto a = new eColonyMonumentAction(lastPlayedColony);
        planAction(a);
    }

    loadResources();
}

void eGameBoard::loadResources() {
    for(const auto& c : mCitiesOnBoard) {
        c->loadResources();
    }
}

bool eGameBoard::checkGoalsFulfilled() const {
    if(mGoals.empty()) return false;
    bool result = true;
    for(const auto& g : mGoals) {
        g->update(*this);
        const bool m = g->met();
        if(!m) result = false;
    }
    return result;
}

void eGameBoard::incPopulation(const eCityId cid, const int by) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->incPopulation(by);
}

void eGameBoard::topElevationExtremas(int& min, int& max) const {
    min = 10000;
    max = -10000;
    for(int x = 0; x < mWidth; x++) {
        const auto t = dtile(x, 0);
        const int a = t->altitude();
        min = std::min(min, a);
        max = std::max(max, a);
    }
}

void eGameBoard::rightElevationExtremas(int& min, int& max) const {
    min = 10000;
    max = -10000;
    for(int y = 0; y < mHeight; y++) {
        const auto t = dtile(mWidth - 1, y);
        const int a = t->altitude();
        min = std::min(min, a);
        max = std::max(max, a);
    }
}

void eGameBoard::bottomElevationExtremas(int& min, int& max) const {
    min = 10000;
    max = -10000;
    for(int x = 0; x < mWidth; x++) {
        const auto t = dtile(x, mHeight - 1);
        const int a = t->altitude();
        min = std::min(min, a);
        max = std::max(max, a);
    }
}

void eGameBoard::leftElevationExtremas(int& min, int& max) const {
    min = 10000;
    max = -10000;
    for(int y = 0; y < mHeight; y++) {
        const auto t = dtile(0, y);
        const int a = t->altitude();
        min = std::min(min, a);
        max = std::max(max, a);
    }
}

void eGameBoard::minMaxAltitude(int& min, int& max) const {
    min = 10000;
    max = -10000;
    for(int x = 0; x < mWidth; x++) {
        for(int y = 0; y < mHeight; y++) {
            const auto t = dtile(x, y);
            const int a = t->altitude();
            if(a > max) max = a;
            if(a < min) min = a;
        }
    }
}

bool eGameBoard::landTradeShutdown(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->landTradeShutdown();
}

void eGameBoard::setLandTradeShutdown(
        const eCityId cid, const bool s) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->setLandTradeShutdown(s);
}

bool eGameBoard::seaTradeShutdown(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->seaTradeShutdown();
}

void eGameBoard::setSeaTradeShutdown(
        const eCityId cid, const bool s) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->setSeaTradeShutdown(s);
}

void eGameBoard::setEpisodeLost() const {
    mEpisodeLost = true;
}

int eGameBoard::tradingPartners() const {
    int n = 0;
    const auto ppid = personPlayer();
    const auto& wrld = world();
    for(const auto& c : wrld.cities()) {
        if(c->isRival()) continue;
        if(c->isCurrentCity()) continue;
        if(!c->active()) continue;
        if(!c->visible()) continue;
        const auto tradeCid = c->cityId();
        const auto tradePid = cityIdToPlayerId(tradeCid);
        const auto tradeTid = playerIdToTeamId(tradePid);
        const auto tid = playerIdToTeamId(ppid);
        if(eTeamIdHelpers::isEnemy(tradeTid, tid)) continue;
        n++;
    }
    return n;
}

eImmigrationLimitedBy eGameBoard::immigrationLimit(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return eImmigrationLimitedBy::none;
    return c->immigrationLimit();
}

int eGameBoard::bestYearlyProduction(const eResourceType type) const {
    const auto it = mYearlyProduction.find(type);
    if(it == mYearlyProduction.end()) return 0;
    return it->second.fBest;
}

void eGameBoard::incProduced(const eResourceType type,
                             const int by) {
    mYearlyProduction[type].fThisYear += by;
}
