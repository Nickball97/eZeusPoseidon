// Part of eGameBoard: soldiers, companies, triremes and military aid.
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

void eGameBoard::enlistForces(const eEnlistedForces& forces) {
    for(const auto& b : forces.fSoldiers) {
        b->goAbroad();
    }
    const auto cids = citiesOnBoard();
    for(const auto h : forces.fHeroes) {
        const auto hh = heroHall(h.first, h.second);
        if(!hh) continue;
        hh->sendHeroOnQuest();
    }
    for(const auto& a : forces.fAllies) {
        a->setAbroad(true);
    }
}

void eGameBoard::clearBannerSelection() {
    for(const auto s : mSelectedBanners) {
        s->setSelected(false);
    }
    mSelectedBanners.clear();
}

void eGameBoard::deselectBanner(eSoldierBanner* const c) {
    eVectorHelpers::remove(mSelectedBanners, c);
    c->setSelected(false);
}

void eGameBoard::selectBanner(eSoldierBanner* const c) {
    mSelectedBanners.push_back(c);
    c->setSelected(true);
}

void eGameBoard::clearTriremeSelection() {
    for(const auto s : mSelectedTriremes) {
        s->setSelected(false);
    }
    mSelectedTriremes.clear();
}

void eGameBoard::deselectTrireme(eTrireme * const c) {
    eVectorHelpers::remove(mSelectedTriremes, c);
    c->setSelected(false);
}

void eGameBoard::selectTrireme(eTrireme * const c) {
    if(!c->selectable()) return;
    mSelectedTriremes.push_back(c);
    c->setSelected(true);
}

void eGameBoard::bannersGoHome() {
    const auto triremes = mSelectedTriremes;
    for(const auto t : triremes) {
        t->goHome();
    }
    std::map<eCityId, std::vector<stdsptr<eSoldierBanner>>> armyReturn;
    const auto banners = mSelectedBanners;
    for(const auto b : banners) {
        if(b->militaryAid() && b->isAbroad()) {
            const auto cid = b->cityId();
            armyReturn[cid].push_back(b->ref<eSoldierBanner>());
        }
        b->goHome();
    }
    for(const auto& r : armyReturn) {
        const auto c = boardCityWithId(r.first);
        if(!c) continue;
        if(r.second.empty()) return;
        const auto fromCid = r.second[0]->onCityId();
        const auto fromC = mWorld.cityWithId(fromCid);
        const auto e = e::make_shared<eArmyReturnEvent>(
            r.first, eGameEventBranch::root, *this);
        const auto boardDate = date();
        const int period = eNumbers::sReinforcementsTravelTime;
        const auto date = boardDate + period;
        e->initializeDate(date, period, 1);
        eEnlistedForces forces;
        forces.fSoldiers = r.second;
        e->initialize(forces, fromC);
        c->addRootGameEvent(e);
    }
}

void eGameBoard::bannersBackFromHome() {
    for(const auto b : mSelectedBanners) {
        if(b->fleeing()) {
            // Company's morale too low to respond!
            showTip(b->cityId(), eLanguage::zeusText(19, 34));
            continue;
        }
        b->backFromHome();
    }
}

void eGameBoard::killCommonFolks(const eCityId cid, int toKill) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->killCommonFolks(toKill);
}

void eGameBoard::walkerKilled(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->walkerKilled();
}

void eGameBoard::rockThrowerKilled(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->rockThrowerKilled();
}

void eGameBoard::hopliteKilled(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->hopliteKilled();
}

void eGameBoard::horsemanKilled(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->horsemanKilled();
}

eEnlistedForces eGameBoard::getEnlistableForces(const ePlayerId pid) const {
    eEnlistedForces result;

    const auto cids = playerCitiesOnBoard(pid);
    for(const auto cid : cids) {
        const auto c = boardCityWithId(cid);
        const auto e = c->getEnlistableForces();
        result.add(e);
    }

    const auto& cts = mWorld.cities();
    for(const auto& c : cts) {
        if(!c->active()) continue;
        const auto cpid = c->playerId();
        if(c->isOnBoard() && cpid == pid) continue;
        const auto type = c->type();
        const auto rel = c->relationship();
        const bool e = type == eCityType::colony ||
                       (type == eCityType::foreignCity &&
                        (rel == eForeignCityRelationship::ally ||
                         rel == eForeignCityRelationship::vassal));
        // Zeus attitude advice.txt: only an ally whose opinion of you is
        // very high (the top tier, 80+) sends troops; vassals and colonies
        // keep eZeus's rule
        const bool ally = type == eCityType::foreignCity &&
                          rel == eForeignCityRelationship::ally;
        const double att = c->attitude(pid);
        const bool willing = ally ? att >= 80 : att > 50;
        if(e && willing) {
            result.fAllies.push_back(c);
        }
    }

    return result;
}

std::vector<eMonster*> eGameBoard::monsters(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->monsters();
}

eGameBoard::eChars eGameBoard::attackingGods(
        const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->attackingGods();
}

bool eGameBoard::manTowers(const eCityId cid) const {
    const auto city = boardCityWithId(cid);
    if(!city) return false;
    return city->manTowers();
}

void eGameBoard::setManTowers(const eCityId cid, const bool m) {
    const auto city = boardCityWithId(cid);
    if(!city) return;
    city->setManTowers(m);
}

void eGameBoard::requestForces(const eEnlistAction& action,
                               const std::vector<eResourceType>& plunderResources,
                               const std::vector<stdsptr<eWorldCity>>& exclude,
                               const bool onlySoldiers) {
    if(mEnlistRequester) {
        const auto ppid = personPlayer();
        const auto baseCids = playerCitiesOnBoard(ppid);
        std::vector<eCityId> cids;
        for(const auto cid : baseCids) {
            bool e = false;
            for(const auto& ee : exclude) {
                const auto eCid = ee->cityId();
                if(cid == eCid) {
                    e = true;
                    break;
                }
            }
            if(e) continue;
            cids.push_back(cid);
        }
        std::vector<std::string> cnames;
        for(const auto cid : cids) {
            const auto n = cityName(cid);
            cnames.push_back(n);
        }
        auto f = getEnlistableForces(ppid);
        if(onlySoldiers) {
            f.fHeroes.clear();
            f.fAllies.clear();
            f.fAres = false;
        }

        const auto ss = f.fSoldiers;
        for(const auto& c : exclude) {
            const auto cid = c->cityId();
            for(const auto& s : ss) {
                const auto sCid = s->cityId();
                if(cid != sCid) continue;
                eVectorHelpers::remove(f.fSoldiers, s);
            }
        }
        std::vector<eHeroType> heroesAbroad;
        for(const auto h : f.fHeroes) {
            const auto hh = heroHall(h.first, h.second);
            const bool abroad = !hh ? true : hh->heroOnQuest();
            if(abroad) heroesAbroad.push_back(h.second);
        }
        for(const auto& e : exclude) {
            eVectorHelpers::remove(f.fAllies, e);
        }
        mEnlistRequester(f, cids, cnames, heroesAbroad, action, plunderResources);
    }
}

std::vector<stdsptr<eSoldierBanner>> eGameBoard::banners(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->banners();
}

int eGameBoard::countBanners(const eBannerType bt,
                             const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->countBanners(bt);
}

int eGameBoard::countSoldiers(const eBannerType bt,
                              const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->countSoldiers(bt);
}

int eGameBoard::countWorkingTriremes(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return 0;
    return c->countWorkingTriremes();
}

void eGameBoard::musterAllSoldiers(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->musterAllSoldiers();
}

void eGameBoard::sendAllSoldiersHome(const eCityId cid) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->sendAllSoldiersHome();
}

eMilitaryAid* eGameBoard::militaryAid(const eCityId cid,
                                      const stdsptr<eWorldCity>& c) const {
    const auto cc = boardCityWithId(cid);
    if(!cc) return nullptr;
    return cc->militaryAid(c);
}

void eGameBoard::removeMilitaryAid(const eCityId cid,
                                   const stdsptr<eWorldCity>& c) {
    const auto cc = boardCityWithId(cid);
    if(!cc) return;
    cc->removeMilitaryAid(c);
}

void eGameBoard::addMilitaryAid(const eCityId cid,
                                const stdsptr<eMilitaryAid>& a) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->addMilitaryAid(a);
}

void eGameBoard::defeatedBy(const eCityId defeated,
                            const stdsptr<eWorldCity>& by) {
    auto& defs = mDefeatedBy[defeated];
    const bool r = eVectorHelpers::contains(defs, by);
    if(r) {
        const auto ccid = currentCityId();
        if(defeated == ccid) setEpisodeLost();
    } else {
        defs.push_back(by);
    }
}

using eCities = std::vector<stdsptr<eWorldCity>>;
eCities eGameBoard::defeatedBy(const eCityId cid) {
    return mDefeatedBy[cid];
}
