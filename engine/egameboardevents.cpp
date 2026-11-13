// Part of eGameBoard: game events, requests, gifts, quests and the games.
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

std::vector<eInvasionHandler*> eGameBoard::invasionHandlers(
        const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return {};
    return c->invasionHandlers();
}

void eGameBoard::addInvasionHandler(const eCityId cid,
                                    eInvasionHandler* const i) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->addInvasionHandler(i);
    updateMusic();
}

void eGameBoard::removeInvasionHandler(const eCityId cid,
                                       eInvasionHandler* const i) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->removeInvasionHandler(i);
    updateMusic();
}

bool eGameBoard::hasActiveInvasions(const eCityId cid) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->hasActiveInvasions();
}

void eGameBoard::planGiftFrom(const stdsptr<eWorldCity>& c,
                              const eResourceType type,
                              const int count,
                              const int delay) {
    const auto e = e::make_shared<eGiftFromEvent>(
                       currentCityId(),
                       eGameEventBranch::root, *this);
    e->initialize(true, type, count, c);
    const auto date = mDate + delay;
    e->initializeDate(date);
    addRootGameEvent(e);
}

void eGameBoard::request(const stdsptr<eWorldCity>& c,
                         const eResourceType type,
                         const eCityId cid) {
    const auto e = e::make_shared<eMakeRequestEvent>(
                       cid, eGameEventBranch::root, *this);
    e->initialize(true, type, c);
    const auto date = mDate + 90;
    e->initializeDate(date);
    addRootGameEvent(e);

    const auto pid = cityIdToPlayerId(cid);
    const auto& cts = mWorld.cities();
    for(const auto& ct : cts) {
        if(ct->isCurrentCity()) continue;
        ct->incAttitude(-10, pid);
    }
    c->incAttitude(-10, pid);
}

void eGameBoard::requestAid(const stdsptr<eWorldCity>& c,
                            const eCityId cid) {
    const auto e = e::make_shared<eRequestAidEvent>(
        cid, eGameEventBranch::root, *this);
    e->setCity(c);
    const auto date = mDate + 30;
    e->initializeDate(date);
    addRootGameEvent(e);
}

void eGameBoard::tributeFrom(const ePlayerId pid,
                             const stdsptr<eWorldCity>& c,
                             const bool postpone) {
    const auto type = c->tributeType();
    const int count = c->tributeCount();

    eEventData ed(pid);
    ed.fType = eMessageEventType::requestTributeGranted;
    ed.fCity = c;
    if(type == eResourceType::drachmas) {
        ed.fA0 = [this, c, count, pid]() { // accept
            const auto p = boardPlayerWithId(pid);
            if(p) p->incDrachmas(count, eFinanceTarget::tributeReceived);
            return count;
        };
    } else {
        const auto cids = playerCitiesOnBoard(pid);
        for(const auto cid : cids) {
            ed.fCSpaceCount[cid] = spaceForResource(cid, type);
            ed.fCityNames[cid] = cityName(cid);
            ed.fCCA0[cid] = [this, cid, c, type, count, pid]() { // accept
                const int a = addResource(cid, type, count);
                if(a == count) return;
                eEventData ed(pid);
                ed.fType = eMessageEventType::resourceGranted;
                ed.fCity = c;
                ed.fResourceType = type;
                ed.fResourceCount = a;
                event(eEvent::tributeAccepted, ed);
            };
        }
    }
    ed.fResourceType = type;
    ed.fResourceCount = count;
    if(postpone) {
        ed.fA1 = [this, c, type, count, pid]() { // postpone
            eEventData ed(pid);
            ed.fType = eMessageEventType::resourceGranted;
            ed.fCity = c;
            ed.fResourceType = type;
            ed.fResourceCount = count;
            event(eEvent::tributePostponed, ed);

            const auto e = e::make_shared<ePayTributeEvent>(
                               currentCityId(),
                               eGameEventBranch::root, *this);
            e->initialize(c);
            const auto date = mDate + 31;
            e->initializeDate(date);
            addRootGameEvent(e);
        };
    }
    ed.fA2 = [this, c, type, count, pid]() { // decline
        eEventData ed(pid);
        ed.fType = eMessageEventType::resourceGranted;
        ed.fCity = c;
        ed.fResourceType = type;
        ed.fResourceCount = count;
        event(eEvent::tributeDeclined, ed);
    };
    event(eEvent::tributePaid, ed);
}

bool eGameBoard::giftTo(const stdsptr<eWorldCity>& c,
                        const eResourceType type,
                        const int count,
                        const eCityId cid) {
    const auto cc = boardCityWithId(cid);
    const int has = cc->resourceCount(type);
    if(has < count) return false;
    cc->takeResource(type, count);
    const auto e = e::make_shared<eGiftToEvent>(
                       cid, eGameEventBranch::root, *this);
    e->initialize(c, type, count);
    const auto date = mDate + 90;
    e->initializeDate(date);
    addRootGameEvent(e);
    return true;
}

void eGameBoard::giftToReceived(const stdsptr<eWorldCity>& c,
                                const eResourceType type,
                                const int count,
                                const ePlayerId pid) {
    const bool a = c->acceptsGift(type, count);
    eEventData ed(pid);
    ed.fType = eMessageEventType::resourceGranted;
    ed.fCity = c;
    ed.fResourceType = type;
    ed.fResourceCount = count;
    if(a) {
        const int mult = count/eGiftHelpers::giftCount(type);
        const bool b = c->buys(type);
        const bool s = c->sells(type);
        if(type == eResourceType::drachmas) {
            event(eEvent::giftReceivedDrachmas, ed);
            c->incAttitude(3*mult, pid);
        } else if(b) {
            event(eEvent::giftReceivedNeeded, ed);
            c->incAttitude(3*mult, pid);
        } else if(s) {
            event(eEvent::giftReceivedSells, ed);
            c->incAttitude(1.5*mult, pid);
        } else {
            event(eEvent::giftReceivedNotNeeded, ed);
            c->incAttitude(1.5*mult, pid);
        }
        c->gifted(type, count);
    } else {
        event(eEvent::giftReceivedRefuse, ed);
    }
}

void eGameBoard::addFulfilledQuest(const ePlayerId pid, const eGodQuest q) {
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addFulfilledQuest(q);
}

void eGameBoard::addSlayedMonster(const eCityId cid, const eMonsterType m) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->monsterSlayed(m);
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addSlayedMonster(m);
}

std::vector<eGodQuest> eGameBoard::fulfilledQuests(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->fulfilledQuests();
}

std::vector<eMonsterType> eGameBoard::slayedMonsters(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->slayedMonsters();
}

bool eGameBoard::wasHeroSummoned(const eCityId cid, const eHeroType hero) const {
    const auto c = boardCityWithId(cid);
    if(!c) return false;
    return c->wasHeroSummoned(hero);
}

void eGameBoard::heroSummoned(const eCityId cid, const eHeroType hero) {
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->heroSummoned(hero);
}

void eGameBoard::addMonsterEvent(const eMonsterType type, eMonsterInvasionEventBase * const e) {
    const auto cid = e->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->addMonsterEvent(type, e);
}

void eGameBoard::removeMonsterEvent(eMonsterInvasionEventBase * const e) {
    const auto cid = e->cityId();
    const auto c = boardCityWithId(cid);
    if(!c) return;
    c->removeMonsterEvent(e);
}

eGameBoard::eQuests eGameBoard::godQuests(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->godQuests();
}

void eGameBoard::addGodQuest(eGodQuestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addGodQuest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

void eGameBoard::removeGodQuest(eGodQuestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->removeGodQuest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

eGameBoard::eRequests eGameBoard::cityRequests(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->cityRequests();
}

void eGameBoard::addCityRequest(eReceiveRequestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addCityRequest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

void eGameBoard::removeCityRequest(eReceiveRequestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->removeCityRequest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

eGameBoard::eTroopsRequests eGameBoard::cityTroopsRequests(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->cityTroopsRequests();
}

void eGameBoard::addCityTroopsRequest(eTroopsRequestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addCityTroopsRequest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

void eGameBoard::removeCityTroopsRequest(eTroopsRequestEvent* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->removeCityTroopsRequest(q);
    if(mRequestUpdateHandler) mRequestUpdateHandler();
}

eGameBoard::eConquests eGameBoard::conquests(const ePlayerId pid) const {
    const auto p = boardPlayerWithId(pid);
    if(!p) return {};
    return p->conquests();
}

void eGameBoard::addConquest(ePlayerConquestEventBase* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->addConquest(q);
}

void eGameBoard::removeConquest(ePlayerConquestEventBase* const q) {
    const auto cid = q->cityId();
    const auto pid = cityIdToPlayerId(cid);
    const auto p = boardPlayerWithId(pid);
    if(!p) return;
    p->removeConquest(q);
}

eInvasionEvent* eGameBoard::invasionToDefend(const eCityId cid) const {
    const auto date = eGameBoard::date();
    for(const auto i : mInvasions) {
        const auto icid = i->cityId();
        if(icid != cid) continue;
        const auto t = i->landInvasionTile();
        if(!t) continue;
        const auto sDate = i->nextDate();
        if(sDate - date < 120) {
            return i;
        }
    }
    return nullptr;
}

void eGameBoard::addInvasion(eInvasionEvent* const i) {
    mInvasions.push_back(i);
}

void eGameBoard::removeInvasion(eInvasionEvent* const i) {
    eVectorHelpers::remove(mInvasions, i);
}

eGameBoard::eArmyEvents eGameBoard::armyEvents() const {
    return mArmyEvents;
}

void eGameBoard::addArmyEvent(eArmyEventBase* const q) {
    mArmyEvents.push_back(q);
}

void eGameBoard::removeArmyEvent(eArmyEventBase* const q) {
    eVectorHelpers::remove(mArmyEvents, q);
}

void eGameBoard::addRootGameEvent(const stdsptr<eGameEvent>& e) {
    const auto cid = e->cityId();
    const auto c = boardCityWithId(cid);
    c->addRootGameEvent(e);
}

void eGameBoard::removeRootGameEvent(const stdsptr<eGameEvent>& e) {
    const auto cid = e->cityId();
    const auto c = boardCityWithId(cid);
    c->removeRootGameEvent(e);
}

void eGameBoard::addGameEvent(eGameEvent* const e) {
    mAllGameEvents.push_back(e);
}

void eGameBoard::removeGameEvent(eGameEvent* const e) {
    eVectorHelpers::remove(mAllGameEvents, e);
}

void eGameBoard::handleGamesBegin(const eGames game) {
    eGameMessages* msgs = nullptr;
    switch(game) {
    case eGames::isthmian:
        msgs = &eMessages::instance.fIsthmianGames;
        break;
    case eGames::nemean:
        msgs = &eMessages::instance.fNemeanGames;
        break;
    case eGames::pythian:
        msgs = &eMessages::instance.fPythianGames;
        break;
    case eGames::olympian:
        msgs = &eMessages::instance.fOlympianGames;
        break;
    }

    const auto pcids = personPlayerCitiesOnBoard();
    for(const auto cid : pcids) {
        const auto c = boardCityWithId(cid);
        if(c->atlantean()) continue;
        eEventData ed(cid);
        if(c->canTakePartInGames(game)) {
            showMessage(ed, msgs->fBegin);
        } else {
            showMessage(ed, msgs->fNoPart);
        }
    }
}

void eGameBoard::handleGamesEnd(const eGames game) {
    eGameMessages* msgs = nullptr;
    switch(game) {
    case eGames::isthmian:
        msgs = &eMessages::instance.fIsthmianGames;
        break;
    case eGames::nemean:
        msgs = &eMessages::instance.fNemeanGames;
        break;
    case eGames::pythian:
        msgs = &eMessages::instance.fPythianGames;
        break;
    case eGames::olympian:
        msgs = &eMessages::instance.fOlympianGames;
        break;
    }

    // zeus.exe (0x00565E90): a roll of 0-99 below the score wins; otherwise
    // a score of 30 or more places second and a lower one loses
    using eCityChance = std::pair<eCityId, int>;
    std::vector<eCityChance> chances;
    for(const auto c : mActiveCitiesOnBoard) {
        if(c->atlantean()) continue;
        const auto id = c->id();
        const int score = c->canTakePartInGames(game) ?
                              c->gamesScore(game, true) : 0;
        chances.push_back(eCityChance{id, score});
    }
    if(chances.empty()) return;
    eRand::randomShuffle(chances);

    eCityId winner = eCityId::neutralFriendly;
    for(const auto& c : chances) {
        const bool won = eRand::rand() % 100 < c.second;
        if(!won) continue;
        const auto cid = c.first;
        const auto city = boardCityWithId(cid);
        if(won) {
            winner = cid;
            eEventData ed(cid);
            showMessage(ed, msgs->fWon);
            city->incWonGames();
            int id = 0;
            switch(game) {
            case eGames::isthmian:
                id = 8;
                break;
            case eGames::nemean:
                id = 3;
                break;
            case eGames::pythian:
                id = 8;
                break;
            case eGames::olympian: {
                const auto pid = cityIdToPlayerId(cid);
                const auto& cs = mWorld.cities();
                for(const auto& c : cs) {
                    c->incAttitude(10., pid);
                }
                id = 8;
            } break;
            }
            allow(c.first, eBuildingType::commemorative, id);
        }
        break;
    }
    eCityId secondCid = eCityId::neutralFriendly;
    for(const auto& c : chances) {
        const auto cid = c.first;
        if(cid == winner) continue;
        const bool second = c.second >= 30;
        eEventData ed(cid);
        if(second && secondCid == eCityId::neutralFriendly) {
            secondCid = cid;
            showMessage(ed, msgs->fSecond);
        } else {
            showMessage(ed, msgs->fLost);
        }
    }
}

bool eGameBoard::handleEpisodeCompleteEvents() {
    bool r = false;
    for(const auto& c : mCitiesOnBoard) {
        r = c->handleEpisodeCompleteEvents() || r;
    }
    return r;
}
