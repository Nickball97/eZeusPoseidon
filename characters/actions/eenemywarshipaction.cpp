#include "eenemywarshipaction.h"

#include <map>

#include "emovetoaction.h"
#include "characters/efightingcharacter.h"
#include "characters/eenemywarship.h"
#include "engine/egameboard.h"
#include "enumbers.h"
#include "erand.h"

namespace {
// one of the original's figure ticks in eZeus ticks: the original moves
// its walkers 816 ticks a month (see original-exe-findings.md, timing)
int origTicks(const int n) {
    return static_cast<int>(n*365.*eNumbers::sDayLength/(12*816));
}

bool playerSoldier(const eCharacterType t) {
    return t == eCharacterType::rockThrower ||
           t == eCharacterType::hoplite ||
           t == eCharacterType::horseman ||
           t == eCharacterType::archerPoseidon ||
           t == eCharacterType::hoplitePoseidon ||
           t == eCharacterType::chariotPoseidon;
}

int tileDist(eTile* const a, eTile* const b) {
    return std::max(std::abs(a->x() - b->x()), std::abs(a->y() - b->y()));
}
}

eEnemyWarshipAction::eEnemyWarshipAction(eCharacter* const c) :
    eFightingAction(c, eCharActionType::enemyWarshipAction) {}

void eEnemyWarshipAction::setup(const int index, eTile* const disembark) {
    mStage = eEnemyWarshipStage::wait;
    mWait = origTicks(25*index + 10);
    mLeaveMonth = monthIndex() + 4;
    if(disembark) {
        mDisembarkX = disembark->x();
        mDisembarkY = disembark->y();
    }
}

int eEnemyWarshipAction::monthIndex() const {
    const auto& date = board().date();
    return 12*date.year() + static_cast<int>(date.month());
}

eTile* eEnemyWarshipAction::waypoint(const int i) const {
    auto& brd = board();
    const auto cid = character()->onCityId();
    switch(i) {
    case 0: return brd.riverEntryPoint(cid);
    case 1: return brd.tile(mDisembarkX, mDisembarkY);
    default: return brd.riverExitPoint(cid);
    }
}

eCharacter* eEnemyWarshipAction::findTarget(const int maxDist) const {
    const auto c = character();
    const auto ct = c->tile();
    if(!ct) return nullptr;
    auto& brd = board();
    const auto tid = c->teamId();
    const auto cid = c->cityId();
    const auto onCid = c->onCityId();
    int range = 0;
    if(const auto f = dynamic_cast<eFightingCharacter*>(c)) {
        range = f->range();
    }
    const auto& chars = brd.characters();
    // warships already after each target
    std::map<eCharacter*, int> chasing;
    for(const auto ch : chars) {
        if(ch == c || ch->type() != eCharacterType::enemyWarship) continue;
        if(ch->dead()) continue;
        const auto w = static_cast<eEnemyWarship*>(ch);
        const auto wa = w->warshipAction();
        if(!wa) continue;
        if(const auto t = wa->huntTarget()) chasing[t]++;
    }
    eCharacter* best = nullptr;
    int bestValue = -range;
    for(const auto ch : chars) {
        if(ch == c || ch->dead()) continue;
        if(ch->onCityId() != onCid) continue;
        if(ch->hiddenUnderwater()) continue;
        const auto cht = ch->tile();
        if(!cht) continue;
        const int dist = tileDist(ct, cht);
        if(maxDist >= 0 && dist > maxDist) continue;
        const auto type = ch->type();
        int score;
        if(playerSoldier(type)) {
            if(!eTeamIdHelpers::isEnemy(ch->teamId(), tid)) continue;
            if(dist > range) continue;
            score = 100;
        } else if(type == eCharacterType::tradeBoat ||
                  type == eCharacterType::fishingBoat) {
            if(ch->cityId() == cid) continue;
            score = 10;
        } else if(type == eCharacterType::trireme) {
            if(!eTeamIdHelpers::isEnemy(ch->teamId(), tid)) continue;
            bool onTransport = false;
            if(const auto f = dynamic_cast<eFightingCharacter*>(ch)) {
                const auto fa = f->fightingAction();
                const auto at = fa ? fa->attackTargetCharacter() : nullptr;
                onTransport = at && at->type() == eCharacterType::enemyBoat;
            }
            score = (onTransport ? 1000 : 500) - 10*chasing[ch];
        } else {
            continue;
        }
        const int value = score - dist;
        if(value > bestValue) {
            bestValue = value;
            best = ch;
        }
    }
    return best;
}

void eEnemyWarshipAction::sailTo(eTile* const t) {
    const auto c = character();
    if(!t) {
        wait(origTicks(25));
        return;
    }
    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::terrain);
    const stdptr<eEnemyWarshipAction> tptr(this);
    a->setFindFailAction([tptr]() {
        // unreachable: rest a while before the next try
        if(tptr) tptr->mRestNext = true;
    });
    c->setActionType(eCharacterActionType::walk);
    setCurrentAction(a);
    a->start(t, eWalkableObject::sCreateDeepWater());
}

void eEnemyWarshipAction::startPatrol() {
    mStage = eEnemyWarshipStage::patrol;
    mTarget = nullptr;
    mTimer = 0;
    mPatrolTime = origTicks(75 + eRand::rand() % 30);
    // from the entry to the landing point, from there to the exit,
    // otherwise to the entry (exe 0x004D4500)
    const auto ct = character()->tile();
    const auto entry = waypoint(0);
    const auto land = waypoint(1);
    if(ct && entry && tileDist(ct, entry) <= 2) {
        mWaypoint = land ? 1 : 2;
    } else if(ct && land && tileDist(ct, land) <= 2) {
        mWaypoint = 2;
    } else {
        mWaypoint = 0;
    }
    sailTo(waypoint(mWaypoint));
}

void eEnemyWarshipAction::startHunt() {
    const auto c = character();
    const auto ct = c->tile();
    if(!ct) return;
    int range = 0;
    if(const auto f = dynamic_cast<eFightingCharacter*>(c)) {
        range = f->range();
    }
    // keep a target still within range, otherwise choose again
    eCharacter* t = mTarget.get();
    const bool keep = t && !t->dead() && t->tile() &&
                      tileDist(ct, t->tile()) < range;
    if(!keep) {
        mTarget = nullptr;
        t = findTarget(-1);
    }
    if(!t) {
        startPatrol();
        return;
    }
    mStage = eEnemyWarshipStage::hunt;
    mTimer = 0;
    if(!keep) {
        mTarget = t;
        const auto type = t->type();
        if((playerSoldier(type) || type == eCharacterType::trireme) &&
           eRand::rand() % 2) {
            mLeaveMonth = monthIndex() + 5;
        }
    }
    if(playerSoldier(t->type())) {
        // soldiers on land: it stays where it is and shoots
        if(!currentAction()) wait(origTicks(24));
        return;
    }
    const auto tt = t->tile();
    if(tileDist(ct, tt) < 3) return;
    goTo(tt->x(), tt->y(), 2);
}

void eEnemyWarshipAction::retreat() {
    const auto c = character();
    const auto ct = c->tile();
    if(!ct) return;
    auto& brd = board();
    const auto tid = c->teamId();
    // the nearest trireme or fishing boat within 9 tiles
    eCharacter* threat = nullptr;
    int threatDist = 10;
    for(const auto ch : brd.characters()) {
        const auto type = ch->type();
        if(type != eCharacterType::trireme &&
           type != eCharacterType::fishingBoat) continue;
        if(ch->dead()) continue;
        if(!eTeamIdHelpers::isEnemy(ch->teamId(), tid)) continue;
        const auto cht = ch->tile();
        if(!cht) continue;
        const int d = tileDist(ct, cht);
        if(d < threatDist) {
            threatDist = d;
            threat = ch;
        }
    }
    if(!threat) {
        if(!currentAction()) sailTo(waypoint(mWaypoint));
        return;
    }
    const auto tt = threat->tile();
    const int dx = ct->x() - tt->x();
    const int dy = ct->y() - tt->y();
    const int fx = ct->x() + (dx > 0 ? 10 : dx < 0 ? -10 : 0);
    const int fy = ct->y() + (dy > 0 ? 10 : dy < 0 ? -10 : 0);
    eTile* to = nullptr;
    int toDist = 1000;
    for(int i = -4; i <= 4; i++) {
        for(int j = -4; j <= 4; j++) {
            const auto t = brd.tile(fx + i, fy + j);
            if(!t || !t->hasDeepWater()) continue;
            if(t->cityId() != c->onCityId()) continue;
            const int d = std::max(std::abs(i), std::abs(j));
            if(d < toDist) {
                toDist = d;
                to = t;
            }
        }
    }
    if(to) sailTo(to);
}

void eEnemyWarshipAction::startLeaving() {
    mStage = eEnemyWarshipStage::leave;
    mTarget = nullptr;
    const auto c = character();
    const auto cid = c->onCityId();
    auto& brd = board();
    auto t = brd.riverExitPoint(cid);
    if(!t) t = brd.riverEntryPoint(cid);
    if(!t) {
        c->kill();
        return;
    }
    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::terrain);
    const stdptr<eCharacter> cptr(c);
    a->setFindFailAction([cptr]() {
        if(cptr) cptr->kill();
    });
    c->setActionType(eCharacterActionType::walk);
    setCurrentAction(a);
    a->start(t, eWalkableObject::sCreateDeepWater());
}

void eEnemyWarshipAction::goHome() {
    startLeaving();
}

bool eEnemyWarshipAction::decide() {
    const auto c = character();
    if(mRestNext && mStage != eEnemyWarshipStage::leave) {
        mRestNext = false;
        c->setActionType(eCharacterActionType::stand);
        wait(origTicks(25));
        return true;
    }
    switch(mStage) {
    case eEnemyWarshipStage::wait:
        c->setActionType(eCharacterActionType::stand);
        wait(mWait > 0 ? mWait : 1);
        break;
    case eEnemyWarshipStage::patrol: {
        // on to the next stop
        for(int i = 1; i <= 3; i++) {
            const int w = (mWaypoint + i) % 3;
            if(waypoint(w)) {
                mWaypoint = w;
                break;
            }
        }
        sailTo(waypoint(mWaypoint));
    } break;
    case eEnemyWarshipStage::hunt:
    case eEnemyWarshipStage::retreat:
        c->setActionType(eCharacterActionType::stand);
        wait(origTicks(24));
        break;
    case eEnemyWarshipStage::leave:
        // through the exit: gone
        c->kill();
        break;
    }
    return true;
}

void eEnemyWarshipAction::increment(const int by) {
    const auto c = character();
    if(mFullHP <= 0.) mFullHP = c->hp();
    const auto r = lookForEnemy(by);
    if(r == eLookForEnemyState::dead) return;
    if(mStage != eEnemyWarshipStage::leave &&
       monthIndex() >= mLeaveMonth) {
        startLeaving();
    }
    if(r == eLookForEnemyState::attacking) return;

    if(mStage == eEnemyWarshipStage::wait) {
        mWait -= by;
        if(mWait <= 0) {
            startPatrol();
            return;
        }
        eComplexAction::increment(by);
        return;
    }

    if(mStage != eEnemyWarshipStage::leave) {
        // its missiles: the best target within range, while doing
        // anything else (exe 0x004D47D0); rams boats right next to it
        mShoot += by;
        if(mShoot >= 200) {
            mShoot = 0;
            int range = 0;
            if(const auto f = dynamic_cast<eFightingCharacter*>(c)) {
                range = f->range();
            }
            const auto t = findTarget(range);
            const auto ct = c->tile();
            if(t && ct && t->tile()) {
                const int d = tileDist(ct, t->tile());
                startAttack(t, d > 1);
                return;
            }
        }
        if(mStage != eEnemyWarshipStage::retreat && mFullHP > 0. &&
           c->hp() < 0.16*mFullHP) {
            mStage = eEnemyWarshipStage::retreat;
            mTarget = nullptr;
            mTimer = origTicks(75);
        }
    }

    mTimer += by;
    switch(mStage) {
    case eEnemyWarshipStage::patrol:
        if(mTimer >= mPatrolTime) startHunt();
        break;
    case eEnemyWarshipStage::hunt:
        if(mTimer >= origTicks(24)) startHunt();
        break;
    case eEnemyWarshipStage::retreat:
        if(mTimer >= origTicks(75)) {
            mTimer = 0;
            retreat();
        }
        break;
    default:
        break;
    }
    eComplexAction::increment(by);
}

void eEnemyWarshipAction::read(eReadStream& src) {
    eFightingAction::read(src);
    src >> mStage;
    src >> mWait;
    src >> mTimer;
    src >> mPatrolTime;
    src >> mShoot;
    src >> mWaypoint;
    src >> mLeaveMonth;
    src >> mDisembarkX;
    src >> mDisembarkY;
    src >> mFullHP;
    src.readCharacter(&board(), [this](eCharacter* const c) {
        mTarget = c;
    });
}

void eEnemyWarshipAction::write(eWriteStream& dst) const {
    eFightingAction::write(dst);
    dst << mStage;
    dst << mWait;
    dst << mTimer;
    dst << mPatrolTime;
    dst << mShoot;
    dst << mWaypoint;
    dst << mLeaveMonth;
    dst << mDisembarkX;
    dst << mDisembarkY;
    dst << mFullHP;
    dst.writeCharacter(mTarget);
}
