#include "eoutlawaction.h"

#include "characters/echaracter.h"
#include "buildings/eelitehousing.h"
#include "engine/egameboard.h"
#include "engine/eboardcity.h"
#include "emovetoaction.h"
#include "elanguage.h"
#include "audio/esounds.h"

eOutlawAction::eOutlawAction(eCharacter* const c, eBuilding* const home) :
    eSickDisgruntledAction(c, home, eCharActionType::outlawAction) {}

bool eOutlawAction::decide() {
    const auto c = character();
    const auto ct = c->type();
    const bool deed = ct == eCharacterType::thief ||
                      ct == eCharacterType::vandal;
    if(!deed || mStage >= 2) return eSickDisgruntledAction::decide();
    if(mStage == 1) {
        actOnTarget();
        mStage = 2;
        goBackDecision(eWalkableObject::sCreateDefault());
        return true;
    }
    goToTarget();
    return true;
}

void eOutlawAction::goToTarget() {
    const auto c = character();
    const auto t = c->tile();
    auto& brd = board();
    const auto city = brd.boardCityWithId(c->onCityId());
    eBuilding* target = nullptr;
    if(t && city) target = city->nearestEliteHouse(t->x(), t->y());
    if(!target) {
        mStage = 3;
        eSickDisgruntledAction::decide();
        return;
    }
    mTarget = target;
    mStage = 1;
    c->setActionType(eCharacterActionType::walk);
    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::buildings |
                         eStateRelevance::terrain);
    a->start(target, eWalkableObject::sCreateDefault());
    setCurrentAction(a);
}

void eOutlawAction::actOnTarget() {
    const auto c = character();
    const auto b = mTarget.get();
    mTarget = nullptr;
    if(!b) return;
    const auto t = c->tile();
    if(!t) return;
    // only if the outlaw actually got there
    const auto rect = b->tileRect();
    const int dx = std::max({rect.x - t->x(), 0, t->x() - (rect.x + rect.w - 1)});
    const int dy = std::max({rect.y - t->y(), 0, t->y() - (rect.y + rect.h - 1)});
    if(std::max(dx, dy) > 1) return;
    if(b->type() != eBuildingType::eliteHousing) return;
    auto& brd = board();
    const auto cid = b->cityId();
    if(c->type() == eCharacterType::thief) {
        static_cast<eEliteHousing*>(b)->robbed();
        brd.showTip(cid, eLanguage::zeusText(19, 167)); // robbed an elite home
    } else {
        b->collapse();
        brd.showTip(cid, eLanguage::zeusText(19, 168)); // razed an elite home
    }
}

static bool sMuggable(const eCharacterType t) {
    switch(t) {
    case eCharacterType::settler:
    case eCharacterType::homeless:
    case eCharacterType::healer:
    case eCharacterType::actor:
    case eCharacterType::philosopher:
    case eCharacterType::competitor:
    case eCharacterType::gymnast:
    case eCharacterType::scholar:
    case eCharacterType::astronomer:
    case eCharacterType::inventor:
    case eCharacterType::curator:
    case eCharacterType::fireFighter:
    case eCharacterType::taxCollector:
    case eCharacterType::waterDistributor:
    case eCharacterType::peddler:
        return true;
    default:
        return false;
    }
}

void eOutlawAction::tryMugging() {
    const auto c = character();
    const auto t = c->tile();
    if(!t) return;
    for(const auto& v : t->characters()) {
        if(!v || v.get() == c || v->dead()) continue;
        if(!sMuggable(v->type())) continue;
        // the original mugger (exe 0x004B68B0) always kills his victim
        eSounds::playAttackSound(c);
        v->killWithCorpse();
        mStage = 2;
        goBackDecision(eWalkableObject::sCreateDefault());
        return;
    }
}

void eOutlawAction::increment(const int by) {
    const auto c = character();
    const auto ct = c->type();
    const bool mugger = ct == eCharacterType::mugger ||
                        (mStage == 3 && (ct == eCharacterType::thief ||
                                         ct == eCharacterType::vandal));
    if(mugger && mStage != 2) {
        mMugTime += by;
        if(mMugTime > 1000) {
            mMugTime = 0;
            tryMugging();
        }
    }
    eSickDisgruntledAction::increment(by);
}

void eOutlawAction::read(eReadStream& src) {
    eSickDisgruntledAction::read(src);
    src.readBuilding(&board(), [this](eBuilding* const b) {
        mTarget = b;
    });
    src >> mStage;
    src >> mMugTime;
}

void eOutlawAction::write(eWriteStream& dst) const {
    eSickDisgruntledAction::write(dst);
    dst.writeBuilding(mTarget.get());
    dst << mStage;
    dst << mMugTime;
}
