// Part of eGameBoard: the lists of what is on the map (buildings, walkers, banners...).
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

void eGameBoard::registerAttackingGod(const eCityId cid,
                                      eCharacter* const c) {
    const auto cc = boardCityWithId(cid);
    if(!c) return;
    cc->registerAttackingGod(c);
    updateMusic();
}

void eGameBoard::registerSoldierBanner(const stdsptr<eSoldierBanner>& b) {
    if(b->militaryAid()) return;
    const auto cid = b->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->registerSoldierBanner(b);
}

bool eGameBoard::unregisterSoldierBanner(const stdsptr<eSoldierBanner>& b) {
    eVectorHelpers::remove(mSelectedBanners, b.get());
    const auto cid = b->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->unregisterSoldierBanner(b);
}

void eGameBoard::registerCharacter(eCharacter* const c) {
    mCharacters.push_back(c);
}

bool eGameBoard::unregisterCharacter(eCharacter* const c) {
    bool updateMusic = false;
    for(const auto& cc : mCitiesOnBoard) {
        const bool r = cc->unregisterAttackingGod(c);
        updateMusic = updateMusic || r;
    }
    eVectorHelpers::remove(mSelectedTriremes, static_cast<eTrireme*>(c));
    if(updateMusic) this->updateMusic();
    return eVectorHelpers::remove(mCharacters, c);
}

void eGameBoard::registerCharacterAction(eCharacterAction* const ca) {
    mCharacterActions.push_back(ca);
}

bool eGameBoard::unregisterCharacterAction(eCharacterAction* const ca) {
    return eVectorHelpers::remove(mCharacterActions, ca);
}

void eGameBoard::registerSoldier(eSoldier* const c) {
    mSoldiers.push_back(c);
}

bool eGameBoard::unregisterSoldier(eSoldier* const c) {
    return eVectorHelpers::remove(mSoldiers, c);
}

void eGameBoard::registerBuilding(eBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return;
    mAllBuildings.push_back(b);
    const auto bt = b->type();
    if(eBuilding::sTimedBuilding(bt)) {
        mTimedBuildings.push_back(b);
    }
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    if(!city) return;
    city->registerBuilding(b);
    scheduleAppealMapUpdate(cid);
}

bool eGameBoard::unregisterBuilding(eBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    eVectorHelpers::remove(mAllBuildings, b);
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    if(!city) return false;
    city->unregisterBuilding(b);
    eVectorHelpers::remove(mTimedBuildings, b);
    scheduleAppealMapUpdate(cid);
    return true;
}

bool eGameBoard::unregisterCommonHouse(eSmallHouse* const ch) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = ch->cityId();
    const auto city = boardCityWithId(cid);
    return city->unregisterCommonHouse(ch);
}

void eGameBoard::registerEmplBuilding(eEmployingBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerEmplBuilding(b);
}

bool eGameBoard::unregisterEmplBuilding(eEmployingBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    return city->unregisterEmplBuilding(b);
}

void eGameBoard::registerTradePost(eTradePost* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerTradePost(b);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

bool eGameBoard::unregisterTradePost(eTradePost* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    const bool r = city->unregisterTradePost(b);
    if(r && mButtonVisUpdater) mButtonVisUpdater();
    return r;
}

bool eGameBoard::hasTradePost(const eCityId cid, const eWorldCity& city) {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->hasTradePost(city);
}

void eGameBoard::registerSpawner(eSpawner* const s) {
    mSpawners.push_back(s);
}

bool eGameBoard::unregisterSpawner(eSpawner* const s) {
    return eVectorHelpers::remove(mSpawners, s);;
}

void eGameBoard::registerStadium(eStadium* const s) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = s->cityId();
    const auto city = boardCityWithId(cid);
    city->registerStadium(s);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::unregisterStadium(const eCityId cid) {
    if(!mRegisterBuildingsEnabled) return;
    const auto city = boardCityWithId(cid);
    city->unregisterStadium();
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::registerMuseum(eMuseum* const s) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = s->cityId();
    const auto city = boardCityWithId(cid);
    city->registerMuseum(s);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::unregisterMuseum(const eCityId cid) {
    if(!mRegisterBuildingsEnabled) return;
    const auto city = boardCityWithId(cid);
    city->unregisterMuseum();
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::registerStorBuilding(eStorageBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerStorBuilding(b);
}

bool eGameBoard::unregisterStorBuilding(eStorageBuilding* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    return city->unregisterStorBuilding(b);
}

void eGameBoard::registerSanctuary(eSanctuary* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerSanctuary(b);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

bool eGameBoard::unregisterSanctuary(eSanctuary* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    const bool r = city->unregisterSanctuary(b);
    if(r && mButtonVisUpdater) mButtonVisUpdater();
    return r;
}

void eGameBoard::registerMonument(eMonument* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerMonument(b);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

bool eGameBoard::unregisterMonument(eMonument* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    const bool r = city->unregisterMonument(b);
    if(r && mButtonVisUpdater) mButtonVisUpdater();
    return r;
}

void eGameBoard::registerHeroHall(eHerosHall* const b) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    city->registerHeroHall(b);
}

bool eGameBoard::unregisterHeroHall(eHerosHall* const b) {
    if(!mRegisterBuildingsEnabled) return false;
    const auto cid = b->cityId();
    const auto city = boardCityWithId(cid);
    const bool r = city->unregisterHeroHall(b);
    if(r && mButtonVisUpdater) mButtonVisUpdater();
    return r;
}

void eGameBoard::registerMissile(eMissile* const m) {
    mMissiles.push_back(m);
}

bool eGameBoard::unregisterMissile(eMissile* const m) {
    return eVectorHelpers::remove(mMissiles, m);
}

bool eGameBoard::hasStadium(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return false;
    return city->hasStadium();
}

bool eGameBoard::hasMuseum(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return false;
    return city->hasMuseum();
}

eStadium* eGameBoard::stadium(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return nullptr;
    return city->stadium();
}

eMuseum* eGameBoard::museum(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return nullptr;
    return city->museum();
}

void eGameBoard::registerPalace(ePalace* const p) {
    if(!mRegisterBuildingsEnabled) return;
    const auto cid = p->cityId();
    const auto city = boardCityWithId(cid);
    if(!city) return;
    city->registerPalace(p);
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::unregisterPalace(const eCityId cid) {
    if(!mRegisterBuildingsEnabled) return;
    const auto city = boardCityWithId(cid);
    if(!city) return;
    city->unregisterPalace();
    if(mButtonVisUpdater) mButtonVisUpdater();
}

void eGameBoard::registerMonster(const eCityId cid, eMonster* const m) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->registerMonster(m);
}

void eGameBoard::unregisterMonster(const eCityId cid, eMonster* const m) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    return c->unregisterMonster(m);
    updateMusic();
}

eBanner* eGameBoard::banner(const eCityId cid,
                            const eBannerTypeS type,
                            const int id) const {
    const auto c = boardCityWithId(cid);
    if(!c) return nullptr;
    return c->banner(type, id);
}

void eGameBoard::registerBanner(eBanner* const b) {
    const auto t = b->tile();
    const auto cid = t->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    mBanners.push_back(b);
    return c->registerBanner(b);
}

void eGameBoard::unregisterBanner(eBanner* const b) {
    const auto t = b->tile();
    const auto cid = t->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    eVectorHelpers::remove(mBanners, b);
    return c->unregisterBanner(b);
}

void eGameBoard::registerAllSoldierBanner(eSoldierBanner* const b) {
    mAllSoldierBanners.push_back(b);
}

void eGameBoard::unregisterAllSoldierBanner(eSoldierBanner* const b) {
    eVectorHelpers::remove(mAllSoldierBanners, b);
    eVectorHelpers::remove(mSelectedBanners, b);
}
