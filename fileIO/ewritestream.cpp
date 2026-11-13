#include "ewritestream.h"

#include <algorithm>
#include <vector>

#include "engine/egameboard.h"
#include "characters/actions/echaracteraction.h"
#include "characters/actions/walkable/ewalkableobject.h"
#include "characters/actions/walkable/ehasresourceobject.h"
#include "characters/actions/egodaction.h"
#include "characters/actions/walkable/eobsticlehandler.h"
#include "characters/actions/epatrolmoveaction.h"
#include "spawners/ebanner.h"
#include "einvasionhandler.h"

void (*eWriteTarget::sWatch)(size_t) = nullptr;

eWriteStream::eWriteStream(const eWriteTarget& dst) :
    mDst(dst) {}

void eWriteStream::writeFormat(const std::string& format) {
    *this << std::string(format);
    *this << eFileFormat::version;
}

void eWriteStream::writeTile(eTile* const tile) {
    *this << bool(tile);
    if(tile) {
        *this << tile->x();
        *this << tile->y();
    }
}

void eWriteStream::writeBuilding(eBuilding* const b) {
    const int bid = b ? b->ioID() : -1;
    *this << bid;
}

void eWriteStream::writeCharacter(eCharacter* const c) {
    const int bid = c ? c->ioID() : -1;
    *this << bid;
}

void eWriteStream::writeCharacterAction(eCharacterAction* const ca) {
    const int bid = ca ? ca->ioID() : -1;
    *this << bid;
}

void eWriteStream::writeWalkable(eWalkableObject* const w) {
    *this << bool(w);
    if(w) {
        *this << w->type();
        w->write(*this);
    }
}

void eWriteStream::writeHasResource(eHasResourceObject* const hr) {
    *this << bool(hr);
    if(hr) {
        *this << hr->type();
        hr->write(*this);
    }
}

void eWriteStream::writeCharActFunc(eCharacterActionFunction* const caf) {
    const bool hasFinish = caf != nullptr;
    *this << hasFinish;
    if(hasFinish) {
        *this << caf->type();
        caf->write(*this);
    }
}

void eWriteStream::writeGodAct(eGodAct* const ga) {
    const bool hasFinish = ga != nullptr;
    *this << hasFinish;
    if(hasFinish) {
        *this << ga->type();
        ga->write(*this);
    }
}

void eWriteStream::writeObsticleHandler(eObsticleHandler* const w) {
    *this << bool(w);
    if(w) {
        *this << w->type();
        w->write(*this);
    }
}

void eWriteStream::writeDirectionTimes(eDirectionTimes* const d) {
    *this << d->size();
    // The map is ordered by where its tiles sit in memory, which changes
    // from run to run. Written in map order (by x, then y) instead, so the
    // same city always gives the same save file. Loading is unaffected.
    using ePair = std::pair<eTile* const, eDirectionLastUseTime>;
    std::vector<const ePair*> sorted;
    sorted.reserve(d->size());
    for(const auto& dt : *d) {
        sorted.push_back(&dt);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const ePair* const a, const ePair* const b) {
        const auto ta = a->first;
        const auto tb = b->first;
        if(!ta || !tb) return !ta && tb;
        if(ta->x() != tb->x()) return ta->x() < tb->x();
        return ta->y() < tb->y();
    });
    for(const auto dt : sorted) {
        writeTile(dt->first);
        dt->second.write(*this);
    }
}

void eWriteStream::writeCity(eWorldCity* const c) {
    const int cid = c ? c->ioID() : -1;
    *this << cid;
}

void eWriteStream::writeBanner(eBanner* const b) {
    const int bid = b ? b->ioID() : -1;
    *this << bid;
}

void eWriteStream::writeSoldierBanner(eSoldierBanner* const b) {
    const int bid = b ? b->ioID() : -1;
    *this << bid;
}

void eWriteStream::writeGameEvent(eGameEvent* const e) {
    const int eid = e ? e->ioID() : -1;
    *this << eid;
}

void eWriteStream::writeInvasionHandler(eInvasionHandler* const i) {
    const int iid = i ? i->ioID() : -1;
    *this << iid;
}
