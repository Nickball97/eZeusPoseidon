#include "epriestaction.h"

#include <algorithm>

#include "emovetoaction.h"
#include "efollowaction.h"
#include "characters/epriest.h"
#include "buildings/estoragebuilding.h"
#include "buildings/sanctuaries/esanctuary.h"
#include "buildings/sanctuaries/etemplealtarbuilding.h"
#include "engine/egameboard.h"
#include "enumbers.h"

namespace {
// one of the original's figure ticks in eZeus ticks: the original moves
// its walkers 816 ticks a month (see original-exe-findings.md, timing)
int origTicks(const int n) {
    return static_cast<int>(n*365.*eNumbers::sDayLength/(12*816));
}

int tileDist(eTile* const a, eTile* const b) {
    if(!a || !b) return 10000;
    return std::max(std::abs(a->x() - b->x()), std::abs(a->y() - b->y()));
}

bool isCattle(const eCharacterType t) {
    return t == eCharacterType::cattle1 ||
           t == eCharacterType::cattle2 ||
           t == eCharacterType::cattle3;
}
}

ePriestAction::ePriestAction(eCharacter* const c) :
    eComplexAction(c, eCharActionType::priestAction) {}

void ePriestAction::setSanctuary(eSanctuary* const s) {
    mSanctuary = s;
}

bool ePriestAction::validAnimal(eCharacter* const a) const {
    if(!a || a->dead() || !a->tile()) return false;
    if(a->onCityId() != character()->onCityId()) return false;
    if(!a->visible() || a->busy()) return false;
    // not one a rancher (or another priest) is already leading
    const auto aa = a->action();
    if(aa && aa->type() == eCharActionType::followAction) return false;
    return true;
}

eCharacter* ePriestAction::findAnimal() const {
    const auto c = character();
    const auto& chars = board().characters();
    const auto taken = [&](eCharacter* const a) {
        for(const auto ch : chars) {
            if(ch == c || ch->type() != eCharacterType::priest) continue;
            const auto pa = dynamic_cast<ePriestAction*>(ch->action());
            if(pa && pa->animal() == a) return true;
        }
        return false;
    };
    // first a sheep, goat or bull, then any cattle (exe 0x004E9D90)
    for(int pass = 0; pass < 2; pass++) {
        for(const auto ch : chars) {
            const auto t = ch->type();
            const bool ok = pass == 0 ?
                        t == eCharacterType::sheep ||
                        t == eCharacterType::goat ||
                        t == eCharacterType::bull :
                        isCattle(t);
            if(!ok) continue;
            if(!validAnimal(ch)) continue;
            if(taken(ch)) continue;
            return ch;
        }
    }
    return nullptr;
}

eStorageBuilding* ePriestAction::findStore() const {
    // the first store with at least 4 loads of food (exe 0x004EA060)
    const auto cid = character()->onCityId();
    const auto bc = board().boardCityWithId(cid);
    if(!bc) return nullptr;
    for(const auto s : bc->storBuildings()) {
        if(s->count(eResourceType::food) >= 4) return s;
    }
    return nullptr;
}

void ePriestAction::walkTo(eTile* const t) {
    const auto c = character();
    if(!t) {
        mMoveFailed = true;
        wait(origTicks(1));
        return;
    }
    const auto a = e::make_shared<eMoveToAction>(c);
    const stdptr<ePriestAction> tptr(this);
    a->setFindFailAction([tptr]() {
        if(tptr) tptr->mMoveFailed = true;
    });
    setCurrentAction(a);
    a->start(t);
}

void ePriestAction::search() {
    mTries = 0;
    mAnimal = nullptr;
    mStore = nullptr;
    if(const auto a = findAnimal()) {
        mAnimal = a;
        mStage = ePriestStage::toAnimal;
        goToAnimal();
        return;
    }
    if(const auto s = findStore()) {
        mStore = s;
        mStage = ePriestStage::toStore;
        goToStore();
        return;
    }
    // nothing to offer: he goes away
    vanish();
}

void ePriestAction::searchLater() {
    mAnimal = nullptr;
    mStore = nullptr;
    mStage = ePriestStage::search;
    character()->setActionType(eCharacterActionType::stand);
    wait(origTicks(11));
}

void ePriestAction::goToAnimal() {
    character()->setActionType(eCharacterActionType::walk);
    walkTo(mAnimal ? mAnimal->tile() : nullptr);
}

void ePriestAction::reachedAnimal() {
    const auto c = character();
    const auto p = static_cast<ePriest*>(c);
    const auto t = mAnimal->type();
    if(t == eCharacterType::sheep || t == eCharacterType::goat) {
        // he lifts it onto his shoulders; the animal is gone
        p->setLoad(t == eCharacterType::goat ? ePriestLoad::goat :
                                               ePriestLoad::sheep);
        mAnimal->kill();
        mAnimal = nullptr;
        mStage = ePriestStage::pickUp;
        c->setActionType(eCharacterActionType::collect);
        wait(p->pickUpTime());
        return;
    }
    p->setLoad(ePriestLoad::cattle);
    const auto fa = e::make_shared<eFollowAction>(c, mAnimal.get());
    mAnimal->setAction(fa);
    if(t == eCharacterType::bull) {
        // the bull first stands facing him (exe 0x004CAA60, state 0x15)
        mStage = ePriestStage::bull;
        c->setActionType(eCharacterActionType::stand);
        wait(origTicks(31));
        return;
    }
    goBack();
}

void ePriestAction::goToStore() {
    character()->setActionType(eCharacterActionType::walk);
    const auto c = character();
    const auto a = e::make_shared<eMoveToAction>(c);
    const stdptr<ePriestAction> tptr(this);
    a->setFindFailAction([tptr]() {
        if(tptr) tptr->mMoveFailed = true;
    });
    setCurrentAction(a);
    a->start(mStore.get());
}

bool ePriestAction::takeFood() {
    // 4 loads, from the food types in order; the cart shows the type he
    // took most of (exe 0x004E9E50)
    const auto s = dynamic_cast<eStorageBuilding*>(mStore.get());
    if(!s) return false;
    if(s->count(eResourceType::food) < 4) return false;
    int rem = 4;
    int most = 0;
    auto mostType = eResourceType::none;
    for(int i = 0; i < 8 && rem > 0; i++) {
        const auto t = static_cast<eResourceType>(1 << i);
        const int n = std::min(rem, s->count(t));
        if(n <= 0) continue;
        const int took = s->take(t, n);
        rem -= took;
        if(took > most) {
            most = took;
            mostType = t;
        }
    }
    const auto p = static_cast<ePriest*>(character());
    p->setLoad(ePriestLoad::food);
    p->setFoodType(mostType);
    mStore = nullptr;
    return true;
}

void ePriestAction::goBack() {
    mStage = ePriestStage::back;
    const auto c = character();
    const auto p = static_cast<ePriest*>(c);
    const auto l = p->load();
    if(l == ePriestLoad::sheep || l == ePriestLoad::goat) {
        c->setActionType(eCharacterActionType::carry);
    } else {
        c->setActionType(eCharacterActionType::walk);
    }
    walkTo(mSanctuary ? mSanctuary->priestTile() : nullptr);
}

void ePriestAction::vanish() {
    const auto c = character();
    // a cow or bull he leads goes too (exe 0x004CB350, state 0x14)
    const auto p = static_cast<ePriest*>(c);
    if(p->load() == ePriestLoad::cattle && mAnimal) mAnimal->kill();
    mAnimal = nullptr;
    setState(eCharacterActionState::finished);
    c->kill();
}

bool ePriestAction::decide() {
    const auto c = character();
    if(!mSanctuary || !mSanctuary->altar()) {
        vanish();
        return true;
    }
    const bool failed = mMoveFailed;
    mMoveFailed = false;
    switch(mStage) {
    case ePriestStage::start:
        c->setActionType(eCharacterActionType::stand);
        mStage = ePriestStage::search;
        wait(origTicks(1));
        break;
    case ePriestStage::search:
        search();
        break;
    case ePriestStage::toAnimal: {
        if(failed || !validAnimal(mAnimal)) {
            searchLater();
        } else if(tileDist(c->tile(), mAnimal->tile()) <= 1) {
            reachedAnimal();
        } else if(++mTries > 50) {
            // a safety limit for an animal he can't catch up with
            searchLater();
        } else {
            goToAnimal();
        }
    } break;
    case ePriestStage::pickUp:
    case ePriestStage::bull:
        goBack();
        break;
    case ePriestStage::toStore:
        if(failed || !takeFood()) {
            searchLater();
        } else {
            goBack();
        }
        break;
    case ePriestStage::back: {
        const auto t = mSanctuary->priestTile();
        if(tileDist(c->tile(), t) <= 1) {
            mSanctuary->priestArrived(c);
            mStage = ePriestStage::atAltar;
            if(c->actionType() != eCharacterActionType::carry) {
                c->setActionType(eCharacterActionType::stand);
            }
            wait(origTicks(1));
        } else if(failed || ++mTries > 5) {
            vanish();
        } else {
            goBack();
        }
    } break;
    case ePriestStage::atAltar: {
        const auto p = static_cast<ePriest*>(c);
        auto s = eSacrifice::sheep;
        if(p->load() == ePriestLoad::food) s = eSacrifice::goods;
        else if(p->load() == ePriestLoad::cattle) s = eSacrifice::bull;
        if(mSanctuary->tryToSacrifice(s)) {
            if(mAnimal) mAnimal->kill();
            mAnimal = nullptr;
            vanish();
        } else {
            wait(origTicks(1));
        }
    } break;
    }
    return true;
}

void ePriestAction::read(eReadStream& src) {
    eComplexAction::read(src);
    src >> mStage;
    auto& brd = board();
    src.readBuilding(&brd, [this](eBuilding* const b) {
        mSanctuary = static_cast<eSanctuary*>(b);
    });
    src.readCharacter(&brd, [this](eCharacter* const c) {
        mAnimal = c;
    });
    src.readBuilding(&brd, [this](eBuilding* const b) {
        mStore = b;
    });
    src >> mTries;
}

void ePriestAction::write(eWriteStream& dst) const {
    eComplexAction::write(dst);
    dst << mStage;
    dst.writeBuilding(mSanctuary.get());
    dst.writeCharacter(mAnimal);
    dst.writeBuilding(mStore.get());
    dst << mTries;
}
