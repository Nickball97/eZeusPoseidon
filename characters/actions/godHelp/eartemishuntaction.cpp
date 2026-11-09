#include "eartemishuntaction.h"

#include "characters/actions/emovetoaction.h"
#include "engine/egameboard.h"

#include <cstdlib>

eLookForWolfGodAct::eLookForWolfGodAct(eGameBoard& board,
                                       const eCityId cid) :
    eGodAct(board, eGodActType::lookForWolf),
    mCityId(cid) {}

eLookForWolfGodAct::eLookForWolfGodAct(eGameBoard& board) :
    eLookForWolfGodAct(board, eCityId::neutralFriendly) {}

eMissileTarget eLookForWolfGodAct::find(eTile* const t) {
    const auto null = static_cast<eTile*>(nullptr);
    for(const auto& cc : t->characters()) {
        if(cc->dead()) continue;
        if(cc->type() != eCharacterType::wolf) continue;
        if(cc->onCityId() != mCityId) continue;
        mTarget = cc;
        return cc.get();
    }
    return null;
}

void eLookForWolfGodAct::act() {
    if(mTarget && !mTarget->dead()) {
        mTarget->killWithCorpse();
    }
}

void eLookForWolfGodAct::read(eReadStream& src) {
    src >> mCityId;
    src.readCharacter(&board(), [this](eCharacter* const c) {
        mTarget = c;
    });
}

void eLookForWolfGodAct::write(eWriteStream& dst) const {
    dst << mCityId;
    dst.writeCharacter(mTarget);
}

eArtemisHuntAction::eArtemisHuntAction(eCharacter* const c) :
    eGodAction(c, eCharActionType::artemisHuntAction) {}

void eArtemisHuntAction::increment(const int by) {
    if(mStage == eArtemisHuntStage::hunt) {
        // shoots any wolf within 8 tiles while she walks
        const auto c = character();
        const auto act = std::make_shared<eLookForWolfGodAct>(
                             board(), cityId());
        lookForRangeAction(by, mLookTime, 1000, 8,
                           eCharacterActionType::fight2, act,
                           c->type(), eGodSound::attack);
    }
    eGodAction::increment(by);
}

bool eArtemisHuntAction::decide() {
    switch(mStage) {
    case eArtemisHuntStage::none:
        mStage = eArtemisHuntStage::hunt;
        huntNext();
        break;
    case eArtemisHuntStage::hunt:
        huntNext();
        break;
    case eArtemisHuntStage::back: {
        const auto c = character();
        c->kill();
    } break;
    }
    return true;
}

eCharacter* eArtemisHuntAction::sWolf(const eGameBoard& board,
                                      const eCityId cid,
                                      eTile* const t) {
    eCharacter* best = nullptr;
    int bestDist = __INT_MAX__;
    for(const auto c : board.characters()) {
        if(!c || c->dead()) continue;
        if(c->type() != eCharacterType::wolf) continue;
        if(c->onCityId() != cid) continue;
        const auto ct = c->tile();
        if(!ct) continue;
        if(!t) return c;
        const int d = std::abs(ct->x() - t->x()) +
                      std::abs(ct->y() - t->y());
        if(d < bestDist) {
            bestDist = d;
            best = c;
        }
    }
    return best;
}

void eArtemisHuntAction::huntNext() {
    const auto c = character();
    const auto w = sWolf(board(), cityId(), c->tile());
    if(!w) {
        mStage = eArtemisHuntStage::back;
        goBackToSanctuary();
        return;
    }
    const stdptr<eCharacter> wptr(w);
    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::terrain);
    // a wolf she can't walk to is still within reach of her arrows
    a->setFindFailAction([wptr]() {
        if(wptr && !wptr->dead()) wptr->killWithCorpse();
    });
    setCurrentAction(a);
    c->setActionType(eCharacterActionType::walk);
    a->start(w->tile());
}

void eArtemisHuntAction::read(eReadStream& src) {
    eGodAction::read(src);
    src >> mStage;
    src >> mLookTime;
}

void eArtemisHuntAction::write(eWriteStream& dst) const {
    eGodAction::write(dst);
    dst << mStage;
    dst << mLookTime;
}
