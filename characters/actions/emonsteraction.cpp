#include "emonsteraction.h"

#include "engine/egameboard.h"

#include "engine/boardData/eheatmaptask.h"
#include "buildings/eheatgetters.h"

#include "emovetoaction.h"
#include "eiteratesquare.h"

#include "egodaction.h"

#include "enumbers.h"
#include "fileIO/efileformat.h"
#include "erand.h"

#include <algorithm>

namespace {
// Model\monster_behavior_<difficulty>.txt, read by zeus.exe 0x0052F460 into
// 0x00FE42F0 (one row per monster, in the files' order) and used through
// 0x00532290. Per row: rest days at the lair (low, high) for aggressiveness
// 1, 2 and 3; rampage months; an unused number; the distance from the lair
// at which a monster stops chasing its attacker. A month has 16 days.
const int gMonsterBehavior[5][16][9] = {
    {   // monster_behavior_Veryeasy.txt
        {16, 22, 11, 16, 6, 12, 1, 1, 4}, // Hydra
        {16, 24, 12, 16, 9, 12, 1, 1, 4}, // Kraken
        {16, 20, 9, 12, 7, 10, 1, 1, 3}, // Cerberus
        {15, 23, 8, 10, 6, 10, 1, 1, 3}, // Cyclops
        {14, 21, 8, 12, 6, 8, 1, 1, 3}, // Minotaur
        {15, 20, 6, 10, 4, 8, 1, 1, 3}, // Medusa
        {12, 20, 7, 11, 5, 9, 1, 1, 4}, // Calydonian Boar
        {12, 18, 8, 11, 5, 8, 1, 1, 4}, // Dragon
        {18, 24, 11, 14, 7, 8, 1, 1, 4}, // Talos
        {13, 20, 8, 12, 4, 7, 1, 1, 3}, // Hector
        {18, 24, 11, 14, 8, 12, 1, 1, 2}, // Scylla
        {15, 22, 10, 14, 7, 10, 1, 1, 3}, // Maenads
        {18, 24, 12, 15, 8, 12, 1, 1, 3}, // Sphinx
        {15, 21, 10, 13, 6, 10, 1, 1, 4}, // Chimera
        {14, 19, 9, 12, 5, 9, 1, 1, 4}, // Harpies
        {17, 23, 11, 14, 7, 10, 1, 1, 3}, // Echidna
    },
    {   // monster_behavior_Easy.txt
        {16, 22, 11, 16, 6, 12, 1, 2, 6}, // Hydra
        {16, 24, 12, 16, 9, 12, 1, 2, 6}, // Kraken
        {16, 20, 9, 12, 7, 10, 1, 2, 4}, // Cerberus
        {15, 23, 8, 10, 6, 10, 1, 2, 4}, // Cyclops
        {14, 21, 8, 12, 6, 8, 1, 2, 5}, // Minotaur
        {15, 20, 6, 10, 4, 8, 1, 2, 4}, // Medusa
        {12, 20, 7, 11, 5, 9, 1, 2, 6}, // Calydonian Boar
        {12, 18, 8, 11, 5, 8, 1, 2, 5}, // Dragon
        {18, 24, 11, 14, 7, 8, 1, 2, 6}, // Talos
        {13, 20, 8, 12, 4, 7, 1, 2, 4}, // Hector
        {18, 24, 11, 14, 8, 12, 1, 2, 3}, // Scylla
        {15, 22, 10, 14, 7, 10, 1, 2, 5}, // Maenads
        {18, 24, 12, 15, 8, 12, 1, 2, 5}, // Sphinx
        {15, 21, 10, 13, 6, 10, 1, 2, 6}, // Chimera
        {14, 19, 9, 12, 5, 9, 1, 2, 6}, // Harpies
        {17, 23, 11, 14, 7, 10, 1, 2, 4}, // Echidna
    },
    {   // monster_behavior_Normal.txt
        {16, 22, 11, 16, 6, 12, 1, 3, 8}, // Hydra
        {16, 24, 12, 16, 9, 12, 1, 3, 8}, // Kraken
        {16, 20, 10, 15, 7, 10, 1, 3, 5}, // Cerberus
        {15, 23, 8, 12, 6, 10, 1, 3, 5}, // Cyclops
        {14, 21, 9, 13, 6, 8, 1, 3, 5}, // Minotaur
        {15, 20, 8, 11, 6, 8, 1, 3, 4}, // Medusa
        {12, 20, 9, 14, 5, 9, 1, 3, 8}, // Calydonian Boar
        {12, 18, 8, 11, 5, 8, 1, 3, 7}, // Dragon
        {18, 24, 11, 14, 7, 8, 1, 3, 8}, // Talos
        {13, 20, 9, 12, 6, 7, 1, 3, 5}, // Hector
        {18, 24, 11, 14, 8, 12, 1, 3, 4}, // Scylla
        {15, 22, 10, 14, 7, 10, 1, 3, 7}, // Maenads
        {18, 24, 12, 15, 8, 12, 1, 3, 6}, // Sphinx
        {15, 21, 10, 13, 6, 10, 1, 3, 8}, // Chimera
        {14, 19, 9, 12, 5, 9, 1, 3, 7}, // Harpies
        {17, 23, 11, 14, 7, 10, 1, 3, 5}, // Echidna
    },
    {   // monster_behavior_Hard.txt
        {16, 22, 11, 16, 6, 12, 1, 4, 10}, // Hydra
        {16, 24, 12, 16, 9, 12, 1, 4, 10}, // Kraken
        {16, 20, 9, 12, 7, 10, 1, 4, 6}, // Cerberus
        {15, 23, 8, 10, 6, 10, 1, 4, 6}, // Cyclops
        {14, 21, 8, 12, 6, 8, 1, 4, 6}, // Minotaur
        {15, 20, 6, 10, 4, 8, 1, 4, 4}, // Medusa
        {12, 20, 7, 11, 5, 9, 1, 4, 10}, // Calydonian Boar
        {12, 18, 8, 11, 5, 8, 1, 4, 8}, // Dragon
        {18, 24, 11, 14, 7, 8, 1, 4, 10}, // Talos
        {13, 20, 8, 12, 4, 7, 1, 4, 6}, // Hector
        {18, 24, 11, 14, 8, 12, 1, 4, 4}, // Scylla
        {15, 22, 10, 14, 7, 10, 1, 4, 9}, // Maenads
        {18, 24, 12, 15, 8, 12, 1, 4, 7}, // Sphinx
        {15, 21, 10, 13, 6, 10, 1, 4, 10}, // Chimera
        {14, 19, 9, 12, 5, 9, 1, 4, 9}, // Harpies
        {17, 23, 11, 14, 7, 10, 1, 4, 6}, // Echidna
    },
    {   // monster_behavior_VeryHard.txt
        {16, 22, 11, 16, 6, 12, 2, 5, 12}, // Hydra
        {16, 24, 12, 16, 9, 12, 2, 5, 12}, // Kraken
        {16, 20, 9, 12, 7, 10, 2, 5, 8}, // Cerberus
        {15, 23, 8, 10, 6, 10, 2, 5, 8}, // Cyclops
        {14, 21, 8, 12, 6, 8, 2, 5, 8}, // Minotaur
        {15, 20, 6, 10, 4, 8, 2, 5, 5}, // Medusa
        {12, 20, 7, 11, 5, 9, 2, 5, 12}, // Calydonian Boar
        {12, 18, 8, 11, 5, 8, 2, 5, 10}, // Dragon
        {18, 24, 11, 14, 7, 8, 2, 5, 12}, // Talos
        {13, 20, 8, 12, 4, 7, 2, 5, 8}, // Hector
        {18, 24, 11, 14, 8, 12, 2, 5, 6}, // Scylla
        {15, 22, 10, 14, 7, 10, 2, 5, 10}, // Maenads
        {18, 24, 12, 15, 8, 12, 2, 5, 9}, // Sphinx
        {15, 21, 10, 13, 6, 10, 2, 5, 12}, // Chimera
        {14, 19, 9, 12, 5, 9, 2, 5, 11}, // Harpies
        {17, 23, 11, 14, 7, 10, 2, 5, 7}, // Echidna
    },
};

int monsterBehaviorRow(const eMonsterType type) {
    switch(type) {
    case eMonsterType::hydra: return 0;
    case eMonsterType::kraken: return 1;
    case eMonsterType::cerberus: return 2;
    case eMonsterType::cyclops: return 3;
    case eMonsterType::minotaur: return 4;
    case eMonsterType::medusa: return 5;
    case eMonsterType::calydonianBoar: return 6;
    case eMonsterType::dragon: return 7;
    case eMonsterType::talos: return 8;
    case eMonsterType::hector: return 9;
    case eMonsterType::scylla: return 10;
    case eMonsterType::maenads: return 11;
    case eMonsterType::sphinx: return 12;
    case eMonsterType::chimera: return 13;
    case eMonsterType::harpies: return 14;
    case eMonsterType::echidna: return 15;
    default: return -1;
    }
}

const int* monsterBehavior(eGameBoard& board, const eMonsterType type) {
    const int row = monsterBehaviorRow(type);
    if(row < 0) return nullptr;
    const auto diff = board.difficulty(board.personPlayer());
    const int d = std::clamp(static_cast<int>(diff), 0, 4);
    return gMonsterBehavior[d][row];
}

// one day of the original's 16-day month, in eZeus ticks
int originalDayTicks() {
    return 365*eNumbers::sDayLength/(12*16);
}
}


eMonsterAction::eMonsterAction(eCharacter* const c) :
    eGodMonsterAction(c, eCharActionType::monsterAction),
    mType(eMonster::sCharacterToMonsterType(c->type())) {}

void eMonsterAction::increment(const int by) {
    const auto c = character();
    const auto at = c->actionType();
    if(at == eCharacterActionType::walk) {
        lookForAttack(by, mLookForAttack,
                      attackPeriod(),
                      eNumbers::sMonsterAttackRange);
    }

    eGodMonsterAction::increment(by);
}

bool eMonsterAction::decide() {
    const auto c = character();
    switch(mStage) {
    case eMonsterAttackStage::none:
        mStage = eMonsterAttackStage::wait;
        if(!c->tile()) {
            randomPlaceOnBoard();
            if(!c->tile()) c->kill();
        } else {
            mHomeTile = c->tile();
        }
        break;
    case eMonsterAttackStage::wait: {
        mStage = eMonsterAttackStage::goTo;
        goToTarget();
        auto& board = this->board();
        board.updateMusic();
    } break;
    case eMonsterAttackStage::goTo:
        mStage = eMonsterAttackStage::patrol;
        monsterPatrol();
        break;
    case eMonsterAttackStage::patrol:
        mStage = eMonsterAttackStage::goBack;
        goBack();
        break;
    case eMonsterAttackStage::goBack: {
        mStage = eMonsterAttackStage::wait;
        if(mType == eMonsterType::scylla ||
           mType == eMonsterType::kraken) {
            moveAround(nullptr, invadePeriod(),
                       eWalkableObject::sCreateDeepWater());
        } else {
            moveAround(nullptr, invadePeriod());
        }
        auto& board = this->board();
        board.updateMusic();
    } break;
    }
    return true;
}

void eMonsterAction::read(eReadStream& src) {
    eGodMonsterAction::read(src);
    mHomeTile = src.readTile(board());
    src >> mAggressivness;
    src >> mStage;
    src >> mLookForAttack;
    if(src.formatVersion() >= eFileFormat::monsterTargets) {
        src >> mTarget;
    }
}

void eMonsterAction::write(eWriteStream& dst) const {
    eGodMonsterAction::write(dst);
    dst.writeTile(mHomeTile);
    dst << mAggressivness;
    dst << mStage;
    dst << mLookForAttack;
    dst << mTarget;
}

eTile* eMonsterAction::closestEmptySpace(const int rdx, const int rdy) const {
    const auto c = character();
    const auto cid = c->onCityId();
    auto& board = c->getBoard();
    eTile* plainTile = nullptr;
    const auto prcsTile = [&](const int i, const int j) {
        const int tx = rdx + i;
        const int ty = rdy + j;
        const auto tt = board.tile(tx, ty);
        if(!tt) return false;
        const auto ttcid = tt->cityId();
        if(ttcid != cid) return false;
        if(!plainTile && tt->walkable()) {
            plainTile = tt;
            return true;
        }
        return false;
    };

    for(int k = 0; k < 1000; k++) {
        eIterateSquare::iterateSquare(k, prcsTile, 1);
    }

    return plainTile;
}

void eMonsterAction::randomPlaceOnBoard() {
    const auto c = character();
    const auto cid = c->onCityId();
    auto& board = c->getBoard();
    const auto city = board.boardCityWithId(cid);
    const auto rtile = city->randomTile();
    const int tx = rtile->x();
    const int ty = rtile->y();
    const auto tile = closestEmptySpace(tx, ty);
    if(!tile) return;
    c->changeTile(tile);
    mHomeTile = tile;
//    const auto c = character();
//    const stdptr<eMonsterAction> tptr(this);
//    const stdptr<eCharacter> cptr(c);
//    const auto hmFinish = [tptr, this, cptr, c](eHeatMap& map) {
//        if(!tptr || !cptr) return;
//        eHeatMapDivisor divisor(map);
//        divisor.divide(10);
//        int tx;
//        int ty;
//        const bool r = divisor.maxHeatTile(tx, ty);
//        if(r) {
//            const auto& board = c->getBoard();
//            c->changeTile(board.tile(tx, ty));
//        } else {
//            setCurrentAction(nullptr);
//        }
//    };
//    const auto hg = eHeatGetters::empty;
//    const auto task = new eHeatMapTask(hg, hmFinish);
//    auto& board = c->getBoard();
//    auto& tp = board.threadPool();
//    tp.queueTask(task);
    //    wait();
}

stdsptr<eObsticleHandler> eMonsterAction::obsticleHandler() {
    return std::make_shared<eMonsterObsticleHandler>(board(), this);
}

void eMonsterAction::goToTarget() {
    if(mType == eMonsterType::scylla ||
       mType == eMonsterType::kraken) {
        const auto c = character();

        const auto underBuilding = [](eTileBase* const tile) {
            const auto type = tile->underBuildingType();
            return eBuilding::sAttackable(type);
        };

        const auto a = e::make_shared<eMoveToAction>(c);
        a->setStateRelevance(eStateRelevance::buildings |
                             eStateRelevance::terrain);
        const stdptr<eMonsterAction> tptr(this);
        a->setFoundAction([tptr, this]() {
            if(!tptr) return;
            const auto c = character();
            c->setActionType(eCharacterActionType::walk);
        });
        a->setRemoveLastTurn(true);

        a->start(underBuilding, eWalkableObject::sCreateDeepWater());
        setCurrentAction(a);
    } else {
        const stdptr<eMonsterAction> tptr(this);
        const auto tryAgain = std::make_shared<eGoToTargetTryAgain>(
                                  board(), this);
        eGodMonsterAction::goToTarget(eHeatGetters::monsterTarget(mTarget),
                                      tryAgain,
                                      obsticleHandler(),
                                      eWalkableHelpers::sMonsterTileDistance,
                                      eWalkableObject::sCreateAttacker(),
                                      eWalkableObject::sCreateDefault());
    }
}

void eMonsterAction::goBack() {
    if(!mHomeTile) return setCurrentAction(nullptr);
    const auto c = character();

    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::buildings |
                         eStateRelevance::terrain);
    a->setTileDistance(eWalkableHelpers::sMonsterTileDistance);
    a->setObsticleHandler(obsticleHandler());
    a->setFindFailAction([](){});

    if(mType == eMonsterType::scylla ||
       mType == eMonsterType::kraken) {
        a->start(mHomeTile, eWalkableObject::sCreateDeepWater());
    } else {
        a->start(mHomeTile, eWalkableObject::sCreateAttacker(),
                 eWalkableObject::sCreateDefault());
    }
    setCurrentAction(a);
    c->setActionType(eCharacterActionType::walk);
}

void eMonsterAction::monsterPatrol() {
    if(mType == eMonsterType::scylla ||
       mType == eMonsterType::kraken) {
        moveAround(nullptr, moveAroundPeriod(), eWalkableObject::sCreateDeepWater());
    } else {
        goToNearestRoad();
    }
}

void eMonsterAction::destroyBuilding(eBuilding* const b) {
    const auto at = eCharacterActionType::fight2;
    const auto c = character();
    const auto chart = c->type();
    const auto finishAttackA = std::make_shared<eMA_destroyBuildingFinish>(
                                   board(), this, b);

    const auto playHitSound = std::make_shared<ePlayMonsterBuildingAttackSoundGodAct>(
                                  board(), b);
    pauseAction();
    spawnMultipleMissiles(at, chart, 500, b->centerTile(),
                          nullptr, playHitSound, finishAttackA, 3);
}

bool eMonsterAction::lookForAttack(const int dtime,
                                   int& time, const int freq,
                                   const int range) {
    const auto c = character();
    const auto act = std::make_shared<eLookForAttackGodAct>(
                         board(), c);

    const auto at = eCharacterActionType::fight2;

    return lookForRangeAction(dtime, time, freq, range,
                              at, act, nullptr);
}

bool eMonsterAction::lookForRangeAction(const int dtime,
                                        int& time, const int freq,
                                        const int range,
                                        const eCharacterActionType at,
                                        const stdsptr<eGodAct>& act,
                                        const stdsptr<eCharActFunc>& missileSound) {
    const auto c = character();
    const auto chart = c->type();
    const auto cat = c->actionType();
    const bool walking = cat == eCharacterActionType::walk;
    if(!walking) return false;
    auto& brd = c->getBoard();
    const auto ct = c->tile();
    if(!ct) return false;
    const int tx = ct->x();
    const int ty = ct->y();

    time += dtime;
    if(time > freq) {
        time -= freq;
        std::vector<eTile*> tiles;
        const int rr = 2*range + 1;
        tiles.reserve(rr*rr);
        for(int i = -range; i <= range; i++) {
            for(int j = -range; j <= range; j++) {
                const int ttx = tx + i;
                const int tty = ty + j;
                const auto t = brd.tile(ttx, tty);
                if(!t) continue;
                tiles.push_back(t);
            }
        }
        eRand::randomShuffle(tiles);
        for(const auto t : tiles) {
            const auto tt = act->find(t);
            if(!tt) continue;

            const auto finishAttackA = std::make_shared<eMA_lookForRangeActionFinishAttack>(
                                           board(), this);

            pauseAction();
            const int attackTime = eMonster::sMonsterAttackTime(mType);
            spawnMissile(at, chart, attackTime, tt,
                         missileSound, act, finishAttackA);
            return true;
        }
    }
    return false;
}

int eMonsterAction::attackPeriod() const {
    switch(mAggressivness) {
    case eMonsterAggressivness::passive:
        return eNumbers::sPassiveMonsterAttackPeriod;
    case eMonsterAggressivness::active:
        return eNumbers::sActiveMonsterAttackPeriod;
    case eMonsterAggressivness::veryActive:
        return eNumbers::sVeryActiveMonsterAttackPeriod;
    case eMonsterAggressivness::aggressive:
        return eNumbers::sAggressiveMonsterAttackPeriod;
    }
    return eNumbers::sPassiveMonsterAttackPeriod;
}

int eMonsterAction::invadePeriod() const {
    // the original's rest at the lair (zeus.exe 0x0052FA80 on entering
    // the lair state): a random number of days from the monster's row,
    // low <= days < high; aggressiveness 0 gives 10000 days, so never again
    if(const auto r = monsterBehavior(board(), mType)) {
        int days = 10000;
        const int a = static_cast<int>(mAggressivness);
        if(a >= 1 && a <= 3) {
            const int low = r[2*(a - 1)];
            const int high = r[2*(a - 1) + 1];
            days = low + (high > low ? eRand::rand() % (high - low) : 0);
        }
        return days*originalDayTicks();
    }
    if(mType == eMonsterType::scylla ||
       mType == eMonsterType::kraken) {
        switch(mAggressivness) {
        case eMonsterAggressivness::passive:
            return eNumbers::sPassiveWaterMonsterInvadePeriod;
        case eMonsterAggressivness::active:
            return eNumbers::sActiveWaterMonsterInvadePeriod;
        case eMonsterAggressivness::veryActive:
            return eNumbers::sVeryActiveWaterMonsterInvadePeriod;
        case eMonsterAggressivness::aggressive:
            return eNumbers::sAggressiveWaterMonsterInvadePeriod;
        }
        return eNumbers::sPassiveWaterMonsterInvadePeriod;
    } else {
        switch(mAggressivness) {
        case eMonsterAggressivness::passive:
            return eNumbers::sPassiveLandMonsterInvadePeriod;
        case eMonsterAggressivness::active:
            return eNumbers::sActiveLandMonsterInvadePeriod;
        case eMonsterAggressivness::veryActive:
            return eNumbers::sVeryActiveLandMonsterInvadePeriod;
        case eMonsterAggressivness::aggressive:
            return eNumbers::sAggressiveLandMonsterInvadePeriod;
        }
        return eNumbers::sPassiveLandMonsterInvadePeriod;
    }
}

int eMonsterAction::moveAroundPeriod() const {
    if(mType == eMonsterType::scylla ||
        mType == eMonsterType::kraken) {
        // the original's rampage lasts the row's months (zeus.exe
        // 0x005304B0 sets 16 days a month, counted down in 0x00531550)
        if(const auto r = monsterBehavior(board(), mType)) {
            return r[6]*16*originalDayTicks();
        }
        switch(mAggressivness) {
        case eMonsterAggressivness::passive:
            return eNumbers::sPassiveWaterMonsterMoveAroundPeriod;
        case eMonsterAggressivness::active:
            return eNumbers::sActiveWaterMonsterMoveAroundPeriod;
        case eMonsterAggressivness::veryActive:
            return eNumbers::sVeryActiveWaterMonsterMoveAroundPeriod;
        case eMonsterAggressivness::aggressive:
            return eNumbers::sAggressiveWaterMonsterMoveAroundPeriod;
        }
        return eNumbers::sPassiveWaterMonsterMoveAroundPeriod;
    } else {
        switch(mAggressivness) {
        case eMonsterAggressivness::passive:
            return eNumbers::sPassiveLandMonsterMoveAroundPeriod;
        case eMonsterAggressivness::active:
            return eNumbers::sActiveLandMonsterMoveAroundPeriod;
        case eMonsterAggressivness::veryActive:
            return eNumbers::sVeryActiveLandMonsterMoveAroundPeriod;
        case eMonsterAggressivness::aggressive:
            return eNumbers::sAggressiveLandMonsterMoveAroundPeriod;
        }
        return eNumbers::sPassiveLandMonsterMoveAroundPeriod;
    }
}
