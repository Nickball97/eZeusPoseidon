// Part of eBoardCity: soldiers, companies, defence and military aid.
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


void eBoardCity::updateCityDefense() {
    eTile* defendTile = nullptr;
    if(!mInvasionHandlers.empty()) {
        const auto& inv = mInvasionHandlers[0];
        defendTile = inv->currentTile();
    } else {
        const auto i = mBoard.invasionToDefend(mId);
        if(i) defendTile = i->landInvasionTile();
    }
    if(defendTile) {
        const int tx = defendTile->x();
        const int ty = defendTile->y();
        std::vector<eSoldierBanner*> bs;
        for(const auto& b : mSoldierBanners) {
            if(b->isAbroad()) continue;
            b->backFromHome();
            bs.push_back(b.get());
        }
        eSoldierBanner::sPlace(bs, tx, ty, mBoard, 3, 3);
        mDefending = true;
    } else {
        for(const auto& b : mSoldierBanners) {
            if(b->isAbroad()) continue;
            b->moveToDefault();
            b->goHome();
        }
        mDefending = false;
    }
}

void eBoardCity::setManTowers(const bool m) {
    mManTowers = m;
}

void eBoardCity::killCommonFolks(int toKill) {
    auto bs = mTimedBuildings;
    eRand::randomShuffle(bs);
    for(const auto b : bs) {
        const auto bt = b->type();
        if(bt == eBuildingType::commonHouse) {
            const auto sh = static_cast<eSmallHouse*>(b);
            const int pop = sh->people();
            const int shk = std::min(toKill, pop);
            toKill -= shk;
            sh->kill(shk);
            if(toKill <= 0) break;
        }
    }
}

void eBoardCity::walkerKilled() {
    killCommonFolks(5);
}

void eBoardCity::rockThrowerKilled() {
    killCommonFolks(2*eNumbers::sRabbleDivisor);
}

void eBoardCity::hopliteKilled() {
    auto bs = mTimedBuildings;
    eRand::randomShuffle(bs);
    for(const auto b : bs) {
        const auto bt = b->type();
        if(bt == eBuildingType::eliteHousing) {
            const auto eh = static_cast<eEliteHousing*>(b);
            const int pop = eh->people();
            const int shk = std::min(4, pop);
            eh->kill(shk);
            eh->removeArmor();
            return;
        }
    }
}

void eBoardCity::horsemanKilled() {
    auto bs = mTimedBuildings;
    eRand::randomShuffle(bs);
    for(const auto b : bs) {
        const auto bt = b->type();
        if(bt == eBuildingType::eliteHousing) {
            const auto eh = static_cast<eEliteHousing*>(b);
            const int pop = eh->people();
            const int shk = std::min(4, pop);
            eh->kill(shk);
            eh->removeArmor();
            eh->removeHorse();
            return;
        }
    }
}

int eBoardCity::missingArmorFromEliteHouses() const {
    int result = 0;
    for(const auto b : mTimedBuildings) {
        const auto bt = b->type();
        if(bt == eBuildingType::eliteHousing) {
            const auto eh = static_cast<eEliteHousing*>(b);
            const int a = eh->arms();
            result += 4 - a;
        }
    }
    return result;
}

int eBoardCity::maxPalaceBannerCount() const {
    const auto pid = owningPlayerId();
    const auto ppid = mBoard.personPlayer();
    int nSpaces = sPalaceTiles;
    if(pid == ppid) return nSpaces;
    for(const auto& b : mPalaceSoldierBanners) {
        const bool a = b->isAbroad();
        if(a) nSpaces++;
    }
    return nSpaces;
}

void eBoardCity::soldierBannersUpdate() {
    updateMaxSoldiers();
    distributeSoldiers();
    consolidateSoldiers();
}

void eBoardCity::updateMaxSoldiers() {
    mMaxRabble = 0;
    mMaxHoplites = 0;
    mMaxHorsemen = 0;
    if(!mPalace) return;
    for(const auto b : mTimedBuildings) {
        const auto bt = b->type();
        if(bt == eBuildingType::commonHouse) {
            const auto ch = static_cast<eSmallHouse*>(b);
            const int l = ch->level();
            if(l < 2) continue;
            int lvlMax = 0;
            if(l == 2) lvlMax = 5;
            else if(l == 3) lvlMax = 6;
            else if(l == 4) lvlMax = 10;
            else if(l == 5) lvlMax = 12;
            else if(l == 6) lvlMax = 15;
            const int pop = ch->people();
            // hovels and homesteads give 20% of their people as soldiers,
            // bigger houses 25% (housing tables, Model files)
            const int popMax = l <= 3 ? (2*pop + 5)/10 : pop/4;
            mMaxRabble += std::min(lvlMax, popMax);
        } else if(bt == eBuildingType::eliteHousing) {
            const auto eh = static_cast<eEliteHousing*>(b);
            int hop;
            int hor;
            eh->soldiers(hop, hor);
            mMaxHoplites += hop;
            mMaxHorsemen += hor;
        }
    }
    mMaxRabble /= eNumbers::sRabbleDivisor;

    const int spb = eNumbers::sSoldiersPerBanner;
    const int nSpaces = maxPalaceBannerCount();
    mMaxHorsemen = std::min(spb*nSpaces, mMaxHorsemen);
    mMaxHorsemen = std::max(0, mMaxHorsemen);
    const int nHorsemenB = std::ceil(mMaxHorsemen/double(spb));
    mMaxHoplites = std::min(spb*nSpaces - spb*nHorsemenB, mMaxHoplites);
    mMaxHoplites = std::max(0, mMaxHoplites);
    const int nHoplitesB = std::ceil(mMaxHoplites/double(spb));
    mMaxRabble = std::min(spb*nSpaces - spb*nHorsemenB - spb*nHoplitesB, mMaxRabble);
    mMaxRabble = std::max(0, mMaxRabble);
}

void eBoardCity::distributeSoldiers() {
    int cRabble = 0;
    int cHoplites = 0;
    int cHorsemen = 0;
    for(const auto& b : mPalaceSoldierBanners) {
        const auto bt = b->type();
        const int c = b->count();
        if(bt == eBannerType::rockThrower) {
            cRabble += c;
        } else if(bt == eBannerType::hoplite) {
            cHoplites += c;
        } else if(bt == eBannerType::horseman) {
            cHorsemen += c;
        }
    }
    const int remRabble = mMaxRabble - cRabble;
    const int remHoplites = mMaxHoplites - cHoplites;
    const int remHorsemen = mMaxHorsemen - cHorsemen;

    for(int i = 0; i < -remRabble; i++) {
        removeSoldier(eCharacterType::rockThrower);
    }
    for(int i = 0; i < -remHoplites; i++) {
        removeSoldier(eCharacterType::hoplite);
    }
    for(int i = 0; i < -remHorsemen; i++) {
        removeSoldier(eCharacterType::horseman);
    }
    for(int i = 0; i < remHorsemen; i++) {
        addSoldier(eCharacterType::horseman);
    }
    for(int i = 0; i < remHoplites; i++) {
        addSoldier(eCharacterType::hoplite);
    }
    for(int i = 0; i < remRabble; i++) {
        addSoldier(eCharacterType::rockThrower);
    }
}

void eBoardCity::consolidateSoldiers() {
    using eSoldierBanners = std::vector<stdsptr<eSoldierBanner>>;
    eSoldierBanners rabble;
    eSoldierBanners hoplites;
    eSoldierBanners horsemen;
    for(const auto& s : mPalaceSoldierBanners) {
        if(s->isAbroad()) continue;
        const auto tile = s->tile();
        if(!tile) s->moveToDefault();
        if(!s->isHome()) continue;
        switch(s->type()) {
        case eBannerType::rockThrower:
            rabble.push_back(s);
            break;
        case eBannerType::hoplite:
            hoplites.push_back(s);
            break;
        case eBannerType::horseman:
            horsemen.push_back(s);
            break;
        default:
            break;
        }
    }
    const auto consolidator = [](const eSoldierBanners& banners) {
        for(int i = 0; i < static_cast<int>(banners.size()); i++) {
            const auto s = banners[i];
            const int sc = s->count();
            int sSpace = eNumbers::sSoldiersPerBanner - sc;
            if(sSpace <= 0) continue;
            for(int j = banners.size() - 1; j > i; j--) {
                const auto ss = banners[j];
                const int ssc = ss->count();
                const int kMax = std::min(sSpace, ssc);
                for(int k = 0; k < kMax; k++) {
                    ss->decCount();
                    s->incCount();
                    sSpace--;
                    if(sSpace <= 0) break;
                }
                if(sSpace <= 0) break;
            }
        }
    };
    consolidator(rabble);
    consolidator(hoplites);
    consolidator(horsemen);
}

void eBoardCity::addSoldier(const eCharacterType st) {
    bool found = false;
    for(const auto& b : mPalaceSoldierBanners) {
        if(b->isAbroad()) continue;
        const auto bt = b->type();
        const int c = b->count();
        if(c >= eNumbers::sSoldiersPerBanner) continue;
        if(bt == eBannerType::rockThrower &&
           st == eCharacterType::rockThrower) {
            found = true;
        } else if(bt == eBannerType::hoplite &&
                  st == eCharacterType::hoplite) {
            found = true;
        } else if(bt == eBannerType::horseman &&
                  st == eCharacterType::horseman) {
            found = true;
        }
        if(found) {
            b->incCount();
            break;
        }
    }
    if(found) return;
    const int nSpaces = maxPalaceBannerCount();
    const int nB = mPalaceSoldierBanners.size();
    if(nB >= nSpaces) return;
    eBannerType bt;
    if(st == eCharacterType::rockThrower) {
        bt = eBannerType::rockThrower;
    } else if(st == eCharacterType::hoplite) {
        bt = eBannerType::hoplite;
    } else if(st == eCharacterType::horseman) {
        bt = eBannerType::horseman;
    } else {
        return;
    }
    const auto b = e::make_shared<eSoldierBanner>(bt, mBoard);
    b->setBothCityIds(mId);
    registerSoldierBanner(b);
    b->incCount();
    b->moveToDefault();
}

void eBoardCity::removeSoldier(const eCharacterType st,
                               const bool skipNotHome) {
    for(const auto& b : mPalaceSoldierBanners) {
        if(b->isAbroad()) continue;
        if(skipNotHome && !b->isHome()) continue;
        const auto bt = b->type();
        const int c = b->count();
        if(c <= 0) continue;
        bool found = false;
        if(bt == eBannerType::rockThrower &&
           st == eCharacterType::rockThrower) {
            found = true;
        } else if(bt == eBannerType::hoplite &&
                  st == eCharacterType::hoplite) {
            found = true;
        } else if(bt == eBannerType::horseman &&
                  st == eCharacterType::horseman){
            found = true;
        }
        if(found) {
            b->decCount();
            return;
        }
    }
    if(skipNotHome) removeSoldier(st, false);
}

void eBoardCity::registerSoldierBanner(const stdsptr<eSoldierBanner>& b) {
    if(b->cityId() != mId) return;
    switch(b->type()) {
    case eBannerType::enemy:
    case eBannerType::trireme:
        return;
    case eBannerType::rockThrower:
    case eBannerType::hoplite:
    case eBannerType::horseman:
        mPalaceSoldierBanners.push_back(b);
    default:
        mSoldierBanners.push_back(b);
    }
}

bool eBoardCity::unregisterSoldierBanner(const stdsptr<eSoldierBanner>& b) {
    eVectorHelpers::remove(mPalaceSoldierBanners, b);
    return eVectorHelpers::remove(mSoldierBanners, b);
}

int eBoardCity::countBanners(const eBannerType bt) const {
    int c = 0;
    for(const auto& bn : mSoldierBanners) {
        if(bn->type() != bt) continue;
        c++;
    }
    return c;
}

int eBoardCity::countSoldiers(const eBannerType bt) const {
    int c = 0;
    for(const auto& bn : mSoldierBanners) {
        if(bn->type() != bt) continue;
        c += bn->count();
    }
    return c;
}

int eBoardCity::countWorkingTriremes() const {
    int c = 0;
    for(const auto b : mTimedBuildings) {
        const auto type = b->type();
        if(type != eBuildingType::triremeWharf) continue;
        const auto tw = static_cast<eTriremeWharf*>(b);
        if(tw->hasTrireme()) c++;
    }
    return c;
}

void eBoardCity::musterAllSoldiers() {
    for(const auto& s : mSoldierBanners) {
        s->backFromHome();
    }
}

void eBoardCity::sendAllSoldiersHome() {
    for(const auto& s : mSoldierBanners) {
        s->goHome();
    }
}

eEnlistedForces eBoardCity::getEnlistableForces() const {
    eEnlistedForces result;

    for(const auto& s : mSoldierBanners) {
        if(s->count() <= 0) continue;
        switch(s->type()) {
        case eBannerType::hoplite:
        case eBannerType::horseman:
        case eBannerType::amazon:
        case eBannerType::aresWarrior:
            result.fSoldiers.push_back(s);
            break;
        default:
            break;
        }
    }

    for(const auto& h : mHeroHalls) {
        const auto s = h->stage();
        if(s != eHeroSummoningStage::arrived) continue;
        const auto ht = h->heroType();
        result.fHeroes.push_back({mId, ht});
    }

    return result;
}

bool eBoardCity::nearestEnemySoldier(const eTeamId tid,
                                     const int tx, const int ty,
                                     int& nX, int& nY) const {
    bool found = false;
    bool kNoTles = true;
    const auto prcs = [&](const int dx, const int dy) {
        const int x = tx + dx;
        const int y = ty + dy;
        const auto tile = mBoard.tile(x, y);
        if(!tile) return false;
        const auto tcid = tile->cityId();
        if(tcid != mId) return false;
        kNoTles = false;
        const auto& chars = tile->characters();
        for(const auto& c : chars) {
            const auto ctid = c->teamId();
            const bool e = eTeamIdHelpers::isEnemy(tid, ctid);
            if(e) {
                found = true;
                nX = x;
                nY = y;
                return true;
            }
        }
        return false;
    };
    for(int k = 0;; k++) {
        kNoTles = true;
        eIterateSquare::iterateSquare(k, prcs);
        if(kNoTles) return false;
        if(found) return true;
    }
    return false;
}

eMilitaryAid* eBoardCity::militaryAid(const stdsptr<eWorldCity>& c) const {
    for(const auto& m : mMilitaryAid) {
        if(m->fCity == c) return m.get();
    }
    return nullptr;
}

void eBoardCity::removeMilitaryAid(const stdsptr<eWorldCity>& c) {
    const int iMax = mMilitaryAid.size();
    for(int i = 0; i < iMax; i++) {
        const auto& m = mMilitaryAid[i];
        if(m->fCity != c) continue;
        mMilitaryAid.erase(mMilitaryAid.begin() + i);
        break;
    }
}

void eBoardCity::addMilitaryAid(const stdsptr<eMilitaryAid>& a) {
    mMilitaryAid.push_back(a);
}

void eBoardCity::addReinforcements(const eCityId fromCid,
                                   const eEnlistedForces& forces) {
    mReinforcements.emplace_back(eReinforcements{forces, fromCid});
}

void eBoardCity::reinforcementsGoHome(const stdsptr<eSoldierBanner>& b) {
    int i = -1;
    for(auto& r : mReinforcements) {
        i++;
        const bool result = r.remove(b);
        if(!result) continue;
        const bool e = r.checkEmpty();
        if(e) {
            mReinforcements.erase(mReinforcements.begin() + i);
        }
        break;
    }
}

void eBoardCity::sendAllReinforcementsHome() {
    const auto& world = mBoard.world();
    for(const auto& r : mReinforcements) {
        const auto fromCid = r.fromCid();
        const auto c = mBoard.boardCityWithId(fromCid);
        if(!c) continue;
        const auto& forces = r.forces();
        if(forces.fSoldiers.empty()) return;
        const auto fromC = world.cityWithId(fromCid);
        const auto e = e::make_shared<eArmyReturnEvent>(
            fromCid, eGameEventBranch::root, mBoard);
        const auto boardDate = mBoard.date();
        const int period = eNumbers::sReinforcementsTravelTime;
        const auto date = boardDate + period;
        e->initializeDate(date, period, 1);
        e->initialize(forces, fromC);
        c->addRootGameEvent(e);
    }
    mReinforcements.clear();
}
