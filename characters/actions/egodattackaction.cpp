#include "egodattackaction.h"

#include "buildings/ebuilding.h"
#include "engine/egameboard.h"
#include "edionysusfollowaction.h"
#include "characters/monsters/ecalydonianboar.h"
#include "characters/actions/ewaitaction.h"
#include "buildings/epalace.h"
#include "enumbers.h"
#include "buildings/ehousebase.h"
#include "engine/eboardcity.h"
#include "engine/eworldcity.h"
#include "einvasionhandler.h"

#include <algorithm>
#include <cmath>

eGodAttackAction::eGodAttackAction(eCharacter* const c) :
    eGodAction(c, eCharActionType::godAttackAction) {}

eGodAttackAction::~eGodAttackAction() {
    if(mSanctuary) mSanctuary->godComeback();
}

void eGodAttackAction::increment(const int by) {
    const auto c = character();
    const auto type = this->type();
    const auto at = c->actionType();
    if(at == eCharacterActionType::walk) {
        const int cursePeriod = eNumbers::sGodAttackCursePeriod;
        const int curseRange = eNumbers::sGodAttackCurseRange;
        const int lookForGodCheck = 1000;
        int attackPeriod;
        switch(type) {
        case eGodType::ares:
        case eGodType::artemis:
        case eGodType::athena:
        case eGodType::zeus:
            attackPeriod = eNumbers::sGodAttackAggressiveAttackPeriod;
            break;
        case eGodType::aphrodite:
        case eGodType::apollo:
        case eGodType::atlas:
        case eGodType::demeter:
        case eGodType::dionysus:
        case eGodType::hades:
        case eGodType::hephaestus:
        case eGodType::hera:
        case eGodType::hermes:
        case eGodType::poseidon:
        default:
            attackPeriod = eNumbers::sGodAttackAttackPeriod;
            break;
        }
        const int attackRange = eNumbers::sGodAttackAttackRange;

        const int targetedAttackPeriod = eNumbers::sGodAttackTargetedAttackPeriod;
        const int targetedAttackRange = eNumbers::sGodAttackTargetedAttackRange;
        const int targetedCursePeriod = eNumbers::sGodAttackTargetedCursePeriod;
        const int targetedCurseRange = eNumbers::sGodAttackTargetedCurseRange;
        const int godFightRange = eNumbers::sGodAttackGodFightRange;

        // Only Zeus curses any building; the others curse their own
        // targets (targeted curse below). Only some gods smash and kill
        // at random (eGod::sGeneralAttack).
        bool r = false;
        if(type == eGodType::zeus) {
            r = lookForBlessCurse(by, mLookForCurse, cursePeriod, curseRange, -1);
        }
        if(!r && eGod::sGeneralAttack(type)) {
            r = lookForAttack(by, mLookForAttack, attackPeriod, attackRange);
        }
        if(!r) r = lookForTargetedAttack(by, mLookForTargetedAttack,
                                         targetedAttackPeriod, targetedAttackRange);
        if(!r) r = lookForTargetedBlessCurse(by, mLookForTargetedCurse,
                                             targetedCursePeriod, targetedCurseRange, -1);
        if(!r) lookForGodAttack(by, mLookForGod, lookForGodCheck, godFightRange);
        if(!r) {
            if(type == eGodType::apollo) {
                auto& board = this->board();
                using eLFPG = eLookForPlagueGodAct;
                const auto act = std::make_shared<eLFPG>(board);
                const auto at = eCharacterActionType::curse;
                const auto s = eGodSound::curse;
                const auto c = character();
                const auto chart = c->type();
                const int plaguePeriod = eNumbers::sGodAttackApolloPlaguePeriod;
                const int plagueRange = eNumbers::sGodAttackApolloPlagueRange;
                lookForRangeAction(by, mLookForSpecial,
                                   plaguePeriod, plagueRange,
                                   at, act, chart, s);
            } else if(type == eGodType::aphrodite) {
                auto& board = this->board();
                using eLFEG = eLookForEvictGodAct;
                const auto act = std::make_shared<eLFEG>(board);
                const auto at = eCharacterActionType::curse;
                const auto s = eGodSound::curse;
                const auto c = character();
                const auto chart = c->type();
                const int evictPeriod = eNumbers::sGodAttackAphroditeEvictPeriod;
                const int evictRange = eNumbers::sGodAttackAphroditeEvictRange;
                lookForRangeAction(by, mLookForSpecial,
                                   evictPeriod, evictRange,
                                   at, act, chart, s);
            }
        }
    }

    if(type == eGodType::atlas) {
        auto& board = this->board();
        const auto tile = c->tile();
        if(tile) {
            const auto& chars = tile->characters();
            for(const auto& cc : chars) {
                if(cc.get() == c) continue;
                const auto cType = cc->type();
                const bool r = eDionysusFollowAction::sShouldFollow(cType);
                if(!r) continue;
                const auto ccaa = cc->actionType();
                if(ccaa == eCharacterActionType::die) continue;
                const auto wa = e::make_shared<eWaitAction>(cc.get());
                wa->setTime(20000);
                cc->setActionType(eCharacterActionType::stand);
                cc->setAction(wa);
                const auto killA = std::make_shared<eChar_killWithCorpseFinish>(
                                       board, cc.get(), true);
                wa->setFinishAction(killA);
                wa->setFailAction(killA);
            }
        }
    }

    eGodAction::increment(by);
}

bool eGodAttackAction::lookForAttack(const int dtime,
                                     int& time, const int freq,
                                     const int range) {
    const auto c = character();
    const auto act = std::make_shared<eLookForAttackGodAct>(board(), c);

    const auto at = eCharacterActionType::fight2;
    const auto s = eGodSound::attack;
    const auto chart = c->type();

    return lookForRangeAction(dtime, time, freq, range,
                              at, act, chart, s);
}

bool eGodAttackAction::lookForTargetedAttack(const int dtime,
                                             int& time, const int freq,
                                             const int range) {
    const auto c = character();
    const auto act = std::make_shared<eLookForTargetedAttackGodAct>(
                         board(), type());

    const auto at = eCharacterActionType::fight2;
    const auto s = eGodSound::attack;
    const auto chart = c->type();

    return lookForRangeAction(dtime, time, freq, range,
                              at, act, chart, s);
}

stdsptr<eObsticleHandler> eGodAttackAction::obsticleHandler() {
    return std::make_shared<eGodObsticleHandler>(board(), this);
}

void eGodAttackAction::destroyBuilding(eBuilding* const b) {
    const auto finishAttackA = std::make_shared<eGAA_destroyBuildingFinish>(
                                   board(), this, b);
    const auto playHitSound = std::make_shared<ePlayMonsterBuildingAttackSoundGodAct>(
                                  board(), b);
    pauseAction();
    const auto at = eCharacterActionType::fight2;
    const auto s = eGodSound::attack;
    const auto c = character();
    const auto chart = c->type();
    spawnGodMultipleMissiles(at, chart, b->centerTile(),
                             s, playHitSound, finishAttackA, 3);
}

void eGodAttackAction::goToTarget() {
    const auto gt = type();
    const auto hg = eHeatGetters::godLeaning(gt);
    const stdptr<eGodAction> tptr(this);
    const auto tele = std::make_shared<eTeleportFindFailFunc>(board(), this);
    eGodMonsterAction::goToTarget(hg, tele, obsticleHandler());
}

bool eGodAttackAction::decide() {
    const auto c = character();
    switch(mStage) {
    case eGodAttackStage::none:
        mStage = eGodAttackStage::appear;
        randomPlaceOnBoard();
        if(!c->tile()) {
            c->kill();
        } else {
            appear();
        }
        break;
    case eGodAttackStage::appear:
        mStage = eGodAttackStage::goTo1;
        initialize();
        goToTarget();
        break;
    case eGodAttackStage::goTo1: {
        mStage = eGodAttackStage::patrol1;
        goToNearestRoad();
    }   break;
    case eGodAttackStage::patrol1:
        mStage = eGodAttackStage::goTo2;
        goToTarget();
        break;
    case eGodAttackStage::goTo2: {
        mStage = eGodAttackStage::patrol2;
        goToNearestRoad();
    }   break;
    case eGodAttackStage::patrol2:
        mStage = eGodAttackStage::disappear;
        disappear();
        break;
    case eGodAttackStage::disappear:
        c->kill();
        break;
    }
    return true;
}

void eGodAttackAction::read(eReadStream& src) {
    eGodAction::read(src);
    src >> mStage;
    src >> mLookForCurse;
    src >> mLookForTargetedCurse;
    src >> mLookForAttack;
    src >> mLookForTargetedAttack;
    src >> mLookForGod;
    src >> mLookForSpecial;
    auto& board = this->board();
    src.readBuilding(&board, [this](eBuilding* const b) {
        mSanctuary = static_cast<eSanctuary*>(b);
    });
}

void eGodAttackAction::write(eWriteStream& dst) const {
    eGodAction::write(dst);
    dst << mStage;
    dst << mLookForCurse;
    dst << mLookForTargetedCurse;
    dst << mLookForAttack;
    dst << mLookForTargetedAttack;
    dst << mLookForGod;
    dst << mLookForSpecial;
    dst.writeBuilding(mSanctuary);
}

void eGodAttackAction::setSanctuary(const stdptr<eSanctuary>& s) {
    mSanctuary = s;
}

void eGodAttackAction::initialize() {
    auto& board = this->board();
    const auto cid = cityId();
    const auto c = character();
    const auto tile = c->tile();
    if(!tile) return;
    const auto type = this->type();
    if(type == eGodType::dionysus) {
        eCharacter* f = c;
        eDionysusFollowAction* fa = nullptr;
        for(int i = 0; i < 3; i++) {
            const auto s = e::make_shared<eSatyr>(board);
            s->changeTile(tile);
            const auto a = e::make_shared<eDionysusFollowAction>(
                               f, s.get());
            s->setAction(a);
            f = s.get();
            if(fa) fa->setFollower(s.get());
            fa = a.get();
        }
    } else if(type == eGodType::hades) {
        const auto p = board.palace(cid);
        if(p) p->setBlessed(-1.);
        const auto& chars = board.characters();
        for(const auto c : chars) {
            const auto cType = c->type();
            const bool r = eDionysusFollowAction::sShouldFollow(cType);
            if(!r) continue;
            if(c->dead()) continue;
            c->killWithCorpse();
        }
    }
    applyArrivalEffects();
}

// One-time effects when an angry god arrives. The behavior is from the
// Zeus and Poseidon manuals; the amounts were given by Nicolás from the
// original game (the manuals only say "some").
void eGodAttackAction::applyArrivalEffects() {
    auto& board = this->board();
    const auto cid = cityId();
    const auto pid = board.cityIdToPlayerId(cid);
    const auto type = this->type();

    const auto inCity = [cid](eCharacter* const ch) {
        if(!ch || ch->dead()) return false;
        const auto t = ch->tile();
        return t && t->cityId() == cid;
    };
    // kills every n-th character of the given types in this city
    const auto killShare = [&](const std::vector<eCharacterType>& types,
                               const int everyNth) {
        int i = 0;
        const auto chars = board.characters();
        for(const auto ch : chars) {
            if(!inCity(ch)) continue;
            const auto ct = ch->type();
            if(std::find(types.begin(), types.end(), ct) == types.end()) continue;
            if(i++ % everyNth == 0) ch->killWithCorpse();
        }
    };
    const int year = 365*eNumbers::sDayLength;

    switch(type) {
    case eGodType::hermes: { // steals 33% of the treasury
        const int d = board.drachmas(pid);
        if(d > 0) board.setDrachmas(pid, d - (33*d)/100);
    } break;
    case eGodType::demeter: // kills half the sheep and goats
        killShare({eCharacterType::sheep}, 2);
        killShare({eCharacterType::goat}, 2);
        break;
    case eGodType::hera: { // half the cattle and half the orange trees
        killShare({eCharacterType::cattle1, eCharacterType::cattle2,
                   eCharacterType::cattle3, eCharacterType::bull}, 2);
        // (trees that are part of a sanctuary are left alone)
        const auto trees = board.buildings(cid, [](eBuilding* const b) {
            if(b->type() != eBuildingType::orangeTree) return false;
            const auto rb = dynamic_cast<eResourceBuilding*>(b);
            return !(rb && rb->sanctuary());
        });
        for(int i = 0; i < (int)trees.size(); i += 2) {
            trees[i]->erase();
        }
        // other leaders think one attitude tier (20 points) less of you
        const auto& cts = board.world().cities();
        for(const auto& ct : cts) {
            if(ct->isCurrentCity()) continue;
            ct->incAttitude(-20, pid);
        }
    } break;
    case eGodType::aphrodite: { // 25% of the people leave with her
        const auto bc = board.boardCityWithId(cid);
        if(bc) {
            const auto houses = bc->houses();
            for(const auto h : houses) {
                h->emigrate(int(std::round(0.25*h->people())));
            }
        }
        // all walkers on the street follow her out of the city
        const auto chars = board.characters();
        for(const auto ch : chars) {
            if(!inCity(ch)) continue;
            if(!eDionysusFollowAction::sShouldFollow(ch->type())) continue;
            ch->kill();
        }
    } break;
    case eGodType::ares: // "marches into your city with an army of Sparti"
        spawnArmy(ePlayerSoldierType::aresWarrior);
        break;
    case eGodType::artemis: { // kills all hunters, wolves twice as strong
        // "she and her Amazons launch a full-scale attack on your city"
        spawnArmy(ePlayerSoldierType::amazon);
        killShare({eCharacterType::hunter}, 1);
        const auto chars = board.characters();
        for(const auto ch : chars) {
            if(!inCity(ch)) continue;
            if(ch->type() != eCharacterType::wolf) continue;
            ch->setHP(2*ch->hp());
            ch->setAttack(2*ch->attack());
        }
    } break;
    case eGodType::athena: { // walls and towers unmanned for a year
        const auto towers = board.buildings(cid, eBuildingType::tower);
        for(const auto t : towers) t->setBlessed(-1., year);
    } break;
    default: break;
    }
}

// An angry Ares or Artemis brings an army hostile to the city. The manual
// gives no size; it uses the size of the army these gods lend a city that
// worships them (2 companies, eNumbers::sSoldiersPerAresArtemisBanner each).
void eGodAttackAction::spawnArmy(const ePlayerSoldierType type) {
    auto& board = this->board();
    const auto tile = character()->tile();
    if(!tile) return;
    const int count = 2*eNumbers::sSoldiersPerAresArtemisBanner;
    // the handler registers itself with the city, saves with it and
    // deletes itself when the army is gone
    const auto h = new eInvasionHandler(board, cityId(), nullptr, nullptr);
    h->initializeGodArmy(tile, type, count);
}
