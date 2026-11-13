#include "ehuntaction.h"

#include "engine/epathfinder.h"

#include "emovetoaction.h"
#include "ewaitaction.h"

#include "characters/eboar.h"
#include "characters/edeer.h"

#include "buildings/ehuntinglodge.h"
#include "characters/efigurestats.h"
#include <algorithm>
#include "engine/egameboard.h"
#include "engine/eorientation.h"
#include "vec2.h"
#include "erand.h"

#include <cmath>

eHuntAction::eHuntAction(eHuntingLodge* const b, eCharacter* const c) :
    eActionWithComeback(c, eCharActionType::huntAction),
    mLodge(b), mHunter(static_cast<eHunter*>(c)) {}

eHuntAction::eHuntAction(eCharacter* const c) :
    eHuntAction(nullptr, c) {}

bool tryToCollect(eTile* const tile) {
    const auto cs = tile->characters();
    for(const auto& c : cs) {
        const auto t = c->type();
        if(t == eCharacterType::boar) {
            const auto b = static_cast<eBoar*>(c.get());
            if(b->dead()) {
                b->kill();
                return true;
            }
        } else if(t == eCharacterType::deer) {
            const auto d = static_cast<eDeer*>(c.get());
            if(d->dead()) {
                d->kill();
                return true;
            }
        }
    }
    return false;
}

void eHuntAction::increment(const int by) {
    if(mHunter && !mHunter->dead()) {
        if(mHunter->collected() == 0 &&
           mHunter->actionType() == eCharacterActionType::walk) {
            mPreyScan += by;
            if(mPreyScan > 200) {
                mPreyScan = 0;
                provokeBoars();
            }
        } else if(mHunter->collected() > 0) {
            mJudged.clear();
        }
    }
    if(mHunter) {
        const auto t = mHunter->tile();
        if(t && tryToCollect(t)) {
            mHunter->incCollected(1);
            setCurrentAction(nullptr);
            return;
        }
    }
    eActionWithComeback::increment(by);
}

bool hasAnimal(eTileBase* const tile, eCharacterType& type) {
    return tile->hasCharacter([&type](const eCharacterBase& c) {
        const bool b = c.type() == eCharacterType::boar && !c.fighting();
        if(b) {
            type = eCharacterType::boar;
            return true;
        }
        const bool d = c.type() == eCharacterType::deer && !c.fighting();
        if(d) {
            type = eCharacterType::deer;
            return true;
        }
        return false;
    });
}

bool eHuntAction::decide() {
    const bool r = eActionWithComeback::decide();
    if(r) return r;

    const auto t = mHunter->tile();
    if(!t) return false;

    if(tryToCollect(t)) mHunter->incCollected(1);
    const int coll = mHunter->collected();

    const SDL_Point p{t->x(), t->y()};
    const auto rect = mLodge->tileRect();
    const bool inLodge = SDL_PointInRect(&p, &rect);

    if(coll > 0) {
        if(inLodge) {
            mLodge->addProduced(eResourceType::meat, mLodge->useBlessing(coll));
            mHunter->incCollected(-coll);
            setState(eCharacterActionState::finished);
        } else {
            goBackDecision();
        }
    } else {
        if(inLodge) {
            const int res = mLodge->resource();
            if(res >= mLodge->maxResource() ||
               !mLodge->enabled()) {
                waitDecision();
            } else {
                if(mNoResource) {
                    mNoResource = false;
                    waitDecision();
                } else {
                    findResourceDecision();
                }
            }
        } else if(mNoResource) {
            mNoResource = false;
            goBackDecision();
        } else {
            findResourceDecision();
        }
    }
    return true;
}

void eHuntAction::read(eReadStream& src) {
    eActionWithComeback::read(src);
    src.readBuilding(&board(), [this](eBuilding* const b) {
        mLodge = static_cast<eHuntingLodge*>(b);
    });
    src.readCharacter(&board(), [this](eCharacter* const c) {
        mHunter = static_cast<eHunter*>(c);
    });
    src >> mNoResource;
}

void eHuntAction::write(eWriteStream& dst) const {
    eActionWithComeback::write(dst);
    dst.writeBuilding(mLodge);
    dst.writeCharacter(mHunter);
    dst << mNoResource;
}

void eHuntAction::findResourceDecision() {
    const auto c = character();

    const stdptr<eHuntAction> tptr(this);

    auto aType = std::make_shared<eCharacterType>();

    const auto hha = [aType](eTileBase* const tile) {
        eCharacterType type;
        const bool r = hasAnimal(tile, type);
        if(r) *aType = type;
        return r;
    };

    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::huntingGrounds |
                         eStateRelevance::buildings |
                         eStateRelevance::terrain);
    a->setFoundAction([tptr, this, c, aType]() {
        if(tptr) {
            if(*aType == eCharacterType::deer) {
                mHunter->setDeerHunter(true);
            }
            mLodge->setNoTarget(false);
        }
        c->setActionType(eCharacterActionType::walk);
    });
    const auto findFailFunc = [tptr, this]() {
        if(tptr) {
            mNoResource = true;
            mLodge->setNoTarget(true);
        }
    };
    a->setFindFailAction(findFailFunc);
    a->setMaxFindDistance(60);
    a->start(hha);
    setCurrentAction(a);
}

void eHuntAction::goBackDecision() {
    mHunter->setActionType(eCharacterActionType::carry);
    const auto rect = mLodge->tileRect();
    eActionWithComeback::goBack(rect, eWalkableObject::sCreateDefault());
}

void eHuntAction::waitDecision() {
    const auto w = e::make_shared<eWaitAction>(mHunter);
    w->setTime(5000);
    setCurrentAction(w);
}

// The manual: "In anger, some will attack your hunters and kill them."
// Per Nicolas, 1 boar in 5 charges a hunter who comes within 3 tiles
// (the hunter's missile range in Figure_model), with attack 10 (the
// file's 1 would do nothing against a hunter's armor of 1). Calm boars
// just get caught; either way the hunter fights it with his usual
// attack animation.
void eHuntAction::provokeBoars() {
    const auto t = mHunter->tile();
    if(!t) return;
    auto& brd = board();
    const int range = 3;
    const int tx = t->x();
    const int ty = t->y();
    for(int i = -range; i <= range; i++) {
        for(int j = -range; j <= range; j++) {
            const auto tt = brd.tile(tx + i, ty + j);
            if(!tt) continue;
            for(const auto& cc : tt->characters()) {
                if(cc->type() != eCharacterType::boar) continue;
                if(cc->dead() || cc->fighting()) continue;
                const auto boar = cc.get();
                // each boar is judged once per hunter's approach
                bool judged = false;
                for(const auto& j : mJudged) {
                    if(j.get() != boar) continue;
                    judged = true;
                    break;
                }
                if(judged) continue;
                mJudged.emplace_back(boar);
                if(boar->attack() != 0) continue; // already angry
                if(eRand::rand() % 5 != 0) continue;
                boar->setAttack(10);
                boar->pauseAction();
                const auto a = e::make_shared<eMoveToAction>(boar);
                // back to grazing afterwards (as after a fight)
                const auto resume = std::make_shared<eChar_fightFinish>(
                                        brd, boar);
                a->setFinishAction(resume);
                a->setFailAction(resume);
                boar->setActionType(eCharacterActionType::walk);
                boar->setAction(a);
                a->start(t);
            }
        }
    }
}
