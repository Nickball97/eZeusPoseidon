// Part of eBoardCity: the lists of what the city has (buildings, banners, monsters...).
// (Split from eboardcity.cpp on 13 Nov 2026; nothing but the file changed.)
#include "eboardcity.h"

#include "buildings/ebuilding.h"
#include "characters/echaracter.h"
#include "buildings/eemployingbuilding.h"
#include "buildings/estoragebuilding.h"
#include "buildings/esmallhouse.h"
#include "buildings/sanctuaries/esanctuary.h"
#include "engine/edifficulty.h"
#include "enumbers.h"
#include "buildings/eelitehousing.h"
#include "buildings/eheroshall.h"
#include "buildings/eagorabase.h"
#include "buildings/etradepost.h"
#include "buildings/eurchinquay.h"
#include "buildings/efishery.h"
#include "buildings/epier.h"
#include "buildings/eaestheticsbuilding.h"
#include "buildings/ehorseranch.h"
#include "buildings/ehorseranchenclosure.h"
#include "buildings/etriremewharf.h"
#include "buildings/pyramids/epyramid.h"

#include "evectorhelpers.h"

#include "gameEvents/einvasionevent.h"
#include "gameEvents/eplayerconquestevent.h"
#include "gameEvents/emonsterinvasioneventbase.h"
#include "gameEvents/egodvisitevent.h"
#include "gameEvents/earmyreturnevent.h"
#include "engine/emilitaryaid.h"
#include "spawners/ebanner.h"
#include "characters/esoldierbanner.h"
#include "einvasionhandler.h"

#include "elanguage.h"
#include "emessages.h"
#include "eeventdata.h"

#include "eplague.h"

#include "engine/eepisode.h"
#include "egameboard.h"

#include "eiteratesquare.h"
#include "etilehelper.h"

#include "spawners/espawner.h"
#include "buildings/ehippodrome.h"
#include "characters/gods/egod.h"
#include "epathfinder.h"
#include "eevent.h"

#include <algorithm>


void eBoardCity::registerBuilding(eBuilding* const b) {
    b->setDistrictId(mCurrentDistrictId);
    mAllBuildings.push_back(b);
    mAllBuildingsState++;
    const auto type = b->type();
    // a building put up while its type is cursed is cursed too: the
    // original keeps one curse counter per type (zeus.exe 0x00EF60B0)
    if(eGod::sCurseTarget(eGodType::zeus, type)) {
        for(const auto o : mAllBuildings) {
            if(o == b || o->type() != type || !o->cursed()) continue;
            b->setBlessed(-1., o->blessTime());
            break;
        }
    }
    if(eBuilding::sTimedBuilding(type)) {
        mTimedBuildings.push_back(b);
    }
    switch(type) {
    case eBuildingType::road:
        mRoadState++;
        break;
    case eBuildingType::hedgeMaze:
    case eBuildingType::dolphinSculpture:
    case eBuildingType::orrery:
    case eBuildingType::spring:
    case eBuildingType::topiary:
        m3x3AestheticBuildings.push_back(b);
        break;
    case eBuildingType::granary:
    case eBuildingType::warehouse:
    case eBuildingType::tradePost:
    case eBuildingType::horseRanch:
    case eBuildingType::horseRanchEnclosure:
    case eBuildingType::chariotFactory:
    case eBuildingType::corral:
    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
    case eBuildingType::wheatFarm:
    case eBuildingType::fishery:
    case eBuildingType::olivePress:
    case eBuildingType::growersLodge:
    case eBuildingType::orangeTendersLodge:
    case eBuildingType::winery:
    case eBuildingType::timberMill:
    case eBuildingType::masonryShop:
    case eBuildingType::foundry:
    case eBuildingType::armory:
    case eBuildingType::refinery:
    case eBuildingType::blackMarbleWorkshop:
    case eBuildingType::sculptureStudio:
    case eBuildingType::dairy:
    case eBuildingType::urchinQuay:
    case eBuildingType::cardingShed:
    case eBuildingType::huntingLodge:
        mBuildingsWithResource.push_back(b);
        break;
    case eBuildingType::eliteHousing:
    case eBuildingType::commonHouse:
        mHouses.push_back(static_cast<eHouseBase*>(b));
        break;
    case eBuildingType::orangeTree:
    case eBuildingType::oliveTree:
    case eBuildingType::vine:
        mTreesAndVines.push_back(b);
        break;
    case eBuildingType::sheep:
    case eBuildingType::goat:
    case eBuildingType::cattle:
        mAnimalBuildings.push_back(b);
        mAnimalBuildingsSurroundingUpdate = true;
        break;
    case eBuildingType::templeAltar:
    case eBuildingType::temple:
    case eBuildingType::templeMonument:
    case eBuildingType::templeGrandStatue:
    case eBuildingType::templeObelisk:
    case eBuildingType::templeStatue:
    case eBuildingType::templeTile:

    case eBuildingType::pyramidPart:
    case eBuildingType::pyramidWall:
    case eBuildingType::pyramidTop:
    case eBuildingType::pyramidStatue:
    case eBuildingType::pyramidMonument:
    case eBuildingType::pyramidTile:
    case eBuildingType::pyramidAltar:
    case eBuildingType::pyramidTemple:
    case eBuildingType::pyramidObservatory:
    case eBuildingType::pyramidMuseum:
        mSanctBuildings.push_back(b);
        break;
    default:
        break;
    }
    if(type == eBuildingType::commemorative ||
       type == eBuildingType::godMonument) {
        mCommemorativeBuildings.push_back(b);
    }
}

bool eBoardCity::unregisterBuilding(eBuilding* const b) {
    if(!mBoard.editorMode()) {
        const int did = b->districtId();
        if(did != -1) {
            const auto bRect = b->tileRect();
            mCityPlan.addScheduledBuilding(did, bRect);
        }
    }
    const auto type = b->type();
    if(type == eBuildingType::road) {
        mRoadState++;
    }
    mAllBuildingsState++;
    eVectorHelpers::remove(mAllBuildings, b);
    eVectorHelpers::remove(mBuildingsWithResource, b);
    eVectorHelpers::remove(mHouses, static_cast<eHouseBase*>(b));
    eVectorHelpers::remove(m3x3AestheticBuildings, b);
    eVectorHelpers::remove(mTimedBuildings, b);
    eVectorHelpers::remove(mCommemorativeBuildings, b);
    eVectorHelpers::remove(mTreesAndVines, b);
    eVectorHelpers::remove(mSanctBuildings, b);
    const bool r = eVectorHelpers::remove(mAnimalBuildings, b);
    if(r) mAnimalBuildingsSurroundingUpdate = true;
    if(type == eBuildingType::hippodromePiece) {
        updateHippodromes();
    }
    return true;
}

void eBoardCity::registerEmplBuilding(eEmployingBuilding* const b) {
    mEmployingBuildings.push_back(b);
    const auto type = b->type();
    const bool sd = isShutDown(type);
    if(sd) b->setShutDown(true);
    eSector s;
    const bool r = eSectorHelpers::sBuildingSector(type, s);
    if(r) {
        mSectorBuildings[s].push_back(b);
    }
    if(!sd) {
        const int me = b->maxEmployees();
        mEmplData.incTotalJobVacancies(me);
        if(r) mEmplDistributor.incMaxEmployees(s, me);
    }
    distributeEmployees();
}

bool eBoardCity::unregisterEmplBuilding(eEmployingBuilding* const b) {
    const bool rr = eVectorHelpers::remove(mEmployingBuildings, b);
    const auto type = b->type();
    eSector s;
    const bool r = eSectorHelpers::sBuildingSector(type, s);
    if(r) {
        eVectorHelpers::remove(mSectorBuildings[s], b);
    }
    if(rr) {
        const bool sd = isShutDown(type);
        if(!sd) {
            const int me = b->maxEmployees();
            mEmplData.incTotalJobVacancies(-me);
            if(r) mEmplDistributor.incMaxEmployees(s, -me);
        }
    }
    distributeEmployees();
    return rr;
}

void eBoardCity::registerTradePost(eTradePost* const b) {
    mTradePosts.push_back(b);
}

bool eBoardCity::unregisterTradePost(eTradePost* const b) {
    return eVectorHelpers::remove(mTradePosts, b);
}

bool eBoardCity::hasTradePost(const eWorldCity& city) {
    for(const auto t : mTradePosts) {
        const bool r = &t->city() == &city;
        if(r) return true;
    }
    return false;
}

void eBoardCity::registerStadium(eStadium* const s) {
    mStadium = s;
}

void eBoardCity::unregisterStadium() {
    mStadium = nullptr;
}

void eBoardCity::registerMuseum(eMuseum* const s) {
    mMuseum = s;
}

void eBoardCity::unregisterMuseum() {
    mMuseum = nullptr;
}

void eBoardCity::registerStorBuilding(eStorageBuilding* const b) {
    mStorBuildings.push_back(b);
}

bool eBoardCity::unregisterStorBuilding(eStorageBuilding* const b) {
    return eVectorHelpers::remove(mStorBuildings, b);
}

void eBoardCity::registerSanctuary(eSanctuary* const b) {
    mSanctuariesState++;
    mSanctuaries.push_back(b);
}

bool eBoardCity::unregisterSanctuary(eSanctuary* const b) {
    mSanctuariesState++;
    return eVectorHelpers::remove(mSanctuaries, b);
}

void eBoardCity::registerMonument(eMonument* const b) {
    mMonumentsState++;
    mMonuments.push_back(b);
}

bool eBoardCity::unregisterMonument(eMonument* const b) {
    mMonumentsState++;
    return eVectorHelpers::remove(mMonuments, b);
}

void eBoardCity::monumentFinished() {
    mMonumentsState++;
}

void eBoardCity::registerHeroHall(eHerosHall* const b) {
    mHeroHalls.push_back(b);
}

bool eBoardCity::unregisterHeroHall(eHerosHall* const b) {
    return eVectorHelpers::remove(mHeroHalls, b);
}

bool eBoardCity::unregisterCommonHouse(eSmallHouse* const ch) {
    const auto p = plagueForHouse(ch);
    if(p) {
        p->removeHouse(ch);
        const int c = p->houseCount();
        if(c <= 0) healPlague(p);
    }
    return true;
}

void eBoardCity::registerPalace(ePalace* const p) {
    mPalace = p;
}

void eBoardCity::unregisterPalace() {
    mPalace = nullptr;
    soldierBannersUpdate();
}

void eBoardCity::registerBanner(eBanner* const b) {
    const int id = b->id();
    const auto type = b->type();
    const auto bb = banner(type, id);
    const auto t = bb ? bb->tile() : nullptr;
    if(t) t->removeBanner(bb);
    mBanners.push_back(b);
}

void eBoardCity::unregisterBanner(eBanner* const b) {
    eVectorHelpers::remove(mBanners, b);
}

void eBoardCity::registerMonster(eMonster* const m) {
    mMonsters.push_back(m);
}

void eBoardCity::unregisterMonster(eMonster* const m) {
    eVectorHelpers::remove(mMonsters, m);
}

void eBoardCity::registerAttackingGod(eCharacter* const c) {
    mAttackingGods.push_back(c);
}

bool eBoardCity::unregisterAttackingGod(eCharacter* const c) {
    return eVectorHelpers::remove(mAttackingGods, c);
}
