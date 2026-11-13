// Part of eGameBoard: undoing the last thing built.
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

// ---- Undo (zeus.exe 0x004A6E90 records, 0x004A6FD0 lights the button,
// 0x004A7A50 counts down and checks; see original-exe-findings.md) ----

void eGameBoard::beginUndoAction(const ePlayerId pid) {
    mUndoPending = eUndoAction();
    mUndoPending.fPid = pid;
    mUndoRecording = true;
}

void eGameBoard::endUndoAction() {
    if(!mUndoRecording) return;
    mUndoRecording = false;
    if(mUndoPending.fBlocked) {
        // clearing buildings: the original can't be undone either way,
        // and the action before it can't be undone any more
        mUndoValid = false;
        mUndo = eUndoAction();
        return;
    }
    if(mUndoPending.empty()) return; // nothing built: keep the last one
    mUndo = mUndoPending;
    // 500 original ticks (0x004A6FD0); a month is 816 of them
    mUndo.fTicksLeft = (500*365*eNumbers::sDayLength)/(12*816);
    mUndoValid = true;
}

void eGameBoard::blockUndo() {
    if(mUndoRecording) mUndoPending.fBlocked = true;
}

void eGameBoard::recordUndoTerrain(eTile* const t, const eTerrain was,
                                   const int cost) {
    if(!mUndoRecording) return;
    mUndoPending.fTerrain.push_back({t, was});
    mUndoPending.fCost += cost;
}

bool eGameBoard::canUndo(const ePlayerId pid) const {
    return mUndoValid && mUndo.fPid == pid;
}

void eGameBoard::updateUndo(const int by) {
    if(!mUndoValid) return;
    mUndo.fTicksLeft -= by;
    bool valid = mUndo.fTicksLeft > 0;
    // a building that is gone, has moved in people or has workers ends it
    // (the original checks the recorded buildings' state each tick)
    for(const auto& b : mUndo.fBuildings) {
        if(!valid) break;
        if(!b) {
            valid = false;
        } else if(const auto h = dynamic_cast<eHouseBase*>(b.get())) {
            if(h->people() > 0) valid = false;
        } else if(const auto e = dynamic_cast<eEmployingBuilding*>(b.get())) {
            if(e->employed() > 0) valid = false;
        }
    }
    if(!valid) {
        mUndoValid = false;
        mUndo = eUndoAction();
    }
}

void eGameBoard::undo(const ePlayerId pid) {
    if(!canUndo(pid)) return;
    for(const auto& b : mUndo.fBuildings) {
        if(b) b->erase();
    }
    std::set<eCityId> cities;
    for(const auto& t : mUndo.fTerrain) {
        t.first->setTerrain(t.second);
        cities.insert(t.first->cityId());
    }
    for(const auto cid : cities) {
        if(const auto c = boardCityWithId(cid)) c->incForestsState();
    }
    incDrachmas(pid, mUndo.fCost, eFinanceTarget::construction);
    mUndoValid = false;
    mUndo = eUndoAction();
}
