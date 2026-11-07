#include "ehuntaction.h"

#include "engine/epathfinder.h"

#include "emovetoaction.h"
#include "ewaitaction.h"

#include "characters/eboar.h"
#include "characters/edeer.h"

#include "buildings/ehuntinglodge.h"
#include "characters/efigurestats.h"
#include "missiles/espearmissile.h"
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
        if(mPrey) {
            if(throwSpear(by)) return;
        } else if(mHunter->collected() == 0 &&
                  mHunter->actionType() == eCharacterActionType::walk) {
            mPreyScan += by;
            if(mPreyScan > 200) {
                mPreyScan = 0;
                if(findPrey()) return;
            }
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
            mLodge->addProduced(eResourceType::meat, coll);
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

namespace {
// Hunters throw spears from up to 3 tiles away (Figure_model: Hunter,
// missile range 3; "Hunter's Spear").
const int sSpearRange = 3;
}

bool eHuntAction::findPrey() {
    const auto t = mHunter->tile();
    if(!t) return false;
    auto& brd = board();
    const int tx = t->x();
    const int ty = t->y();
    for(int i = -sSpearRange; i <= sSpearRange; i++) {
        for(int j = -sSpearRange; j <= sSpearRange; j++) {
            const auto tt = brd.tile(tx + i, ty + j);
            if(!tt) continue;
            for(const auto& cc : tt->characters()) {
                const auto ct = cc->type();
                if(ct != eCharacterType::boar &&
                   ct != eCharacterType::deer) continue;
                if(cc->dead() || cc->fighting()) continue;
                mPrey = cc.get();
                mSpearTime = 0;
                if(ct == eCharacterType::deer) mHunter->setDeerHunter(true);
                // The manual: "In anger, some will attack your hunters and
                // kill them." Per Nicolas, 1 boar in 5 charges, with
                // attack 10 (the file's 1 would do nothing against a
                // hunter's armor of 1).
                if(ct == eCharacterType::boar && cc->attack() == 0 &&
                   eRand::rand() % 5 == 0) {
                    cc->setAttack(10);
                    const auto boar = cc.get();
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
                setCurrentAction(nullptr);
                return true;
            }
        }
    }
    return false;
}

bool eHuntAction::throwSpear(const int by) {
    const auto c = mHunter;
    const auto ht = c->tile();
    const auto pt = mPrey->tile();
    if(!ht || !pt) {
        mPrey = nullptr;
        return false;
    }
    if(mPrey->dead()) {
        // walk to the carcass and pick it up (see tryToCollect)
        mPrey = nullptr;
        const auto a = e::make_shared<eMoveToAction>(c);
        c->setActionType(eCharacterActionType::walk);
        setCurrentAction(a);
        a->start(pt);
        return true;
    }
    const double dx = pt->x() - ht->x();
    const double dy = pt->y() - ht->y();
    const double dist = std::sqrt(dx*dx + dy*dy);
    if(dist > sSpearRange + 0.5) { // got away
        mPrey = nullptr;
        findResourceDecision();
        return true;
    }
    c->setActionType(eCharacterActionType::collect); // throwing
    const vec2d posdif{mPrey->absX() - c->absX(), mPrey->absY() - c->absY()};
    c->setOrientation(sAngleOrientation(posdif.angle()));
    mSpearTime += by;
    const int spearPeriod = 500;
    if(mSpearTime > spearPeriod) {
        mSpearTime -= spearPeriod;
        eMissile::sCreate<eSpearMissile>(board(), ht->x(), ht->y(), 0.5,
                                         pt->x(), pt->y(), 0.5, 0.1*dist);
    }
    mPrey->defend(eFigureStats::sDamage(c, mPrey.get(), true, by));
    return true;
}
