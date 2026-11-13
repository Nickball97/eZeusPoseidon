// Part of eBoardCity: stored goods and exports.
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


void eBoardCity::updateResources() {
    for(auto& r : mResources) {
        int& count = r.second;
        count = 0;
        const auto type = r.first;
        for(const auto s : mStorBuildings) {
            count += s->count(type);
        }
    }
}

int eBoardCity::resourceCount(const eResourceType type) const {
    if(type == eResourceType::drachmas) {
        const auto p = owningPlayer();
        if(p) return p->drachmas();
        return 0;
    }
    int result = 0;
    for(auto& r : mResources) {
        if(!static_cast<bool>(r.first & type)) continue;
        result += r.second;
    }
    return result;
}

int eBoardCity::takeResource(const eResourceType type, const int count) {
    if(type == eResourceType::drachmas) {
        const auto p = owningPlayer();
        if(p) p->incDrachmas(-count, eFinanceTarget::giftsAndAidGiven);
        return count;
    }
    int r = 0;
    using eValidator = std::function<bool(eStorageBuilding*)>;
    const auto takeFunc = [&](const eValidator& v) {
        if(r >= count) return;
        for(const auto s : mStorBuildings) {
            if(!v(s)) continue;
            if(r >= count) return;
            r += s->take(type, count - r);
        }
    };
    takeFunc([&](eStorageBuilding* const s) {
        return s->empties(type);
    });
    takeFunc([&](eStorageBuilding* const s) {
        return !s->accepts(type);
    });
    takeFunc([&](eStorageBuilding* const s) {
        return !s->get(type);
    });
    takeFunc([&](eStorageBuilding* const s) {
        (void)s;
        return true;
    });
    return r;
}

int eBoardCity::addResource(const eResourceType type, const int count) {
    if(type == eResourceType::drachmas) {
        const auto p = owningPlayer();
        if(p) p->incDrachmas(count, eFinanceTarget::giftsReceived);
        return count;
    }
    int rem = count;
    using eValidator = std::function<bool(eStorageBuilding*)>;
    const auto addFunc = [&](const eValidator& v) {
        if(rem <= 0) return;
        for(const auto s : mStorBuildings) {
            if(!v(s)) continue;
            const int c = s->add(type, rem);
            rem -= c;
            if(rem <= 0) break;
        }
    };
    addFunc([&](eStorageBuilding* const s) {
        return s->get(type);
    });
    addFunc([&](eStorageBuilding* const s) {
        (void)s;
        return true;
    });
    return count - rem;
}

int eBoardCity::spaceForResource(const eResourceType type) const {
    if(type == eResourceType::drachmas) {
        return __INT_MAX__/2;
    }
    int r = 0;
    for(const auto s : mStorBuildings) {
        r += s->spaceLeft(type);
    }
    return r;
}

int eBoardCity::maxSingleSpaceForResource(
        const eResourceType type,
        eStorageBuilding** b) const {
    *b = nullptr;
    int r = 0;
    for(const auto s : mStorBuildings) {
        const int ss = s->spaceLeft(type);
        if(ss > r) {
            *b = s;
            r = ss;
        }
    }
    return r;
}

int eBoardCity::waitingCount(const eResourceType type) const {
    const bool single = eResourceTypeHelpers::isSingleType(type);
    if(single) {
        const auto it = mWaiting.find(type);
        if(it == mWaiting.end()) return 0;
        return it->second;
    }
    int result = 0;
    for(auto& r : mWaiting) {
        if(!static_cast<bool>(r.first & type)) continue;
        result += r.second;
    }
    return result;
}

void eBoardCity::incWaitingCount(const eResourceType type, const int by) {
    mWaiting[type] += by;
}

int eBoardCity::maxMonumentSpaceForResource(
        eMonument** b) const {
    *b = nullptr;
    int r = 0;
    for(const auto s : mMonuments) {
        int ss = s->spaceLeft(eResourceType::wood);
        ss += s->spaceLeft(eResourceType::marble);
        ss += s->spaceLeft(eResourceType::sculpture);
        ss += s->spaceLeft(eResourceType::blackMarble);
        ss += s->spaceLeft(eResourceType::orichalc);
        if(ss > r) {
            *b = s;
            r = ss;
        }
    }
    return r;
}

bool eBoardCity::supportsResource(const eResourceType rt) const {
    const auto s = mSupportedResources & rt;
    return static_cast<bool>(s);
}

void eBoardCity::addExported(const eCityId cid,
                             const eResourceType type,
                             const int count) {
    auto& map = mExported[cid];
    const auto it = map.find(type);
    if(it == map.end()) {
        map[type] = count;
    } else {
        map[type] += count;
    }
}

void eBoardCity::removeExported(const eCityId cid,
                                const eResourceType type,
                                const int count) {
    auto& map = mExported[cid];
    const auto it = map.find(type);
    if(it != map.end()) {
        map[type] -= count;
    }
}

int eBoardCity::exported(const eCityId cid, const eResourceType type) {
    auto& map = mExported[cid];
    const auto it = map.find(type);
    if(it != map.end()) {
        return map[type];
    } else {
        return 0;
    }
}

std::map<eResourceType, int> eBoardCity::exported(const eCityId cid) const {
    const auto it = mExported.find(cid);
    if(it == mExported.end()) return {};
    return it->second;
}
