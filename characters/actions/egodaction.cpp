#include "egodaction.h"

#include "engine/egameboard.h"
#include "audio/esounds.h"
#include "etilehelper.h"

#include "ewaitaction.h"
#include "characters/actions/emovetoaction.h"
#include "ekillcharacterfinishfail.h"
#include "edionysusfollowaction.h"
#include "buildings/epalacetile.h"
#include "buildings/epalace.h"
#include "buildings/sanctuaries/esanctbuilding.h"
#include "buildings/emonument.h"
#include "engine/eboardcity.h"
#include "enumbers.h"

eGodAction::eGodAction(eCharacter* const c,
                       const eCharActionType type) :
    eGodMonsterAction(c, type),
    mType(eGod::sCharacterToGodType(c->type())) {}

bool eGodAction::lookForBlessCurse(
        const int dtime,
        int& time, const int freq,
        const int range,
        const double bless) {
    using eLFBG = eLookForBlessGodAct;
    const auto act = std::make_shared<eLFBG>(
                         board(), bless);
    eCharacterActionType at;
    eGodSound s;
    if(bless > 0) {
        at = eCharacterActionType::bless;
        s = eGodSound::santcify;
    } else {
        at = eCharacterActionType::curse;
        s = eGodSound::curse;
    }
    const auto c = character();
    const auto chart = c->type();
    return lookForRangeAction(dtime, time, freq, range,
                              at, act, chart, s);
}

bool eGodAction::lookForTargetedBlessCurse(
        const int dtime, int& time,
        const int freq, const int range,
        const double bless) {
    using eLFBG = eLookForTargetedBlessGodAct;
    const auto act = std::make_shared<eLFBG>(
                         board(), bless, type());
    eCharacterActionType at;
    eGodSound s;
    if(bless > 0) {
        at = eCharacterActionType::bless;
        s = eGodSound::santcify;
    } else {
        at = eCharacterActionType::curse;
        s = eGodSound::curse;
    }
    const auto c = character();
    const auto chart = c->type();
    return lookForRangeAction(dtime, time, freq, range,
                              at, act, chart, s);
}

bool eGodAction::lookForSoldierAttack(
        const int dtime, int& time,
        const int freq, const int range) {
    const auto c = character();
    const auto team = c->teamId();

    using eLFSAGA = eLookForSoldierAttackGodAct;
    const auto act = std::make_shared<eLFSAGA>(board(), team);

    const auto at = eCharacterActionType::fight2;
    const auto s = eGodSound::attack;
    const auto chart = c->type();
    return lookForRangeAction(dtime, time, freq, range,
                              at, act, chart, s, 4);
}

bool eGodAction::lookForRangeAction(
        const int dtime,
        int& time, const int freq,
        const int range,
        const eCharacterActionType at,
        const stdsptr<eGodAct>& act,
        const eCharacterType chart,
        const eGodSound missileSound,
        const int nMissiles) {
    const auto c = character();
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
        time = 0;
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
            if(!tt.target()) continue;

            using eGA_LFRAF = eGA_lookForRangeActionFinish;
            const auto finishAttackA =
                    std::make_shared<eGA_LFRAF>(
                        board(), this);

            pauseAction();
            if(nMissiles == 1) {
                spawnGodMissile(at, chart, tt, missileSound,
                                act, finishAttackA);
            } else {
                spawnGodMultipleMissiles(at, chart, tt, missileSound,
                                         act, finishAttackA, nMissiles);
            }
            return true;
        }
        time += freq/2;
    }
    return false;
}

void eGodAction::spawnGodMissile(
        const eCharacterActionType at,
        const eCharacterType chart,
        const eMissileTarget& target,
        const eGodSound sound,
        const stdsptr<eGodAct>& act,
        const stdsptr<eCharActFunc>& finishAttackA) {
    const int time = at == eCharacterActionType::bless ?
                         eGod::sGodBlessTime(type()) :
                         eGod::sGodAttackTime(type());
    const auto c = character();
    using eGA_SGMPS = eGA_spawnGodMissilePlaySound;
    const auto playSound = std::make_shared<eGA_SGMPS>(
                               board(), sound, c);
    spawnMissile(at, chart, time, target,
                 playSound, act, finishAttackA);
}

void eGodAction::goBackToSanctuary() {
    auto& board = eGodAction::board();
    const auto s = board.sanctuary(cityId(), type());
    if(!s) return;
    const auto c = character();
    const auto god = static_cast<eGod*>(c);
    const stdptr<eGod> cptr(god);
    const auto fail = std::make_shared<eKillCharacterFinishFail>(
                          board, god);
    const auto finish = std::make_shared<eKillCharacterFinishFail>(
                            board, god);

    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::buildings |
                         eStateRelevance::terrain);
    a->setFailAction(fail);
    a->setFinishAction(finish);
    a->setFindFailAction([cptr]() {
        if(cptr) cptr->kill();
    });
    setCurrentAction(a);
    c->setActionType(eCharacterActionType::walk);
    a->start(s);
}

void eGodAction::goToTarget() {
    const auto gt = type();
    const auto hg = eHeatGetters::godLeaning(gt);
    using eGTTT = eGoToTargetTeleport;
    const auto tele = std::make_shared<eGTTT>(board(), this);
    eGodMonsterAction::goToTarget(hg, tele);
}

eBuilding* eGodCurseTargetBuilding(eBuilding* const b) {
    if(!b) return nullptr;
    if(const auto pt = dynamic_cast<ePalaceTile*>(b)) return pt->palace();
    if(const auto sb = dynamic_cast<eSanctBuilding*>(b)) {
        return sb->monument();
    }
    return b;
}

void eGodCurseBuildingType(eGameBoard& board, eBuilding* const target,
                           const eGodType god) {
    if(!target) return;
    const auto cid = target->cityId();
    const auto pid = board.cityIdToPlayerId(cid);
    const int diff = static_cast<int>(board.difficulty(pid));
    const int months = eGod::sCurseMonths(god, diff);
    if(months <= 0) return;
    // the original counts months x 16 days down one day at a time
    const int ticks = months*365*eNumbers::sDayLength/12;
    const auto curseType = [&](const eBuildingType bt) {
        const auto bs = board.buildings(cid, bt);
        for(const auto b : bs) {
            b->setBlessed(-1., ticks);
            // Poseidon manual (Atlas): cursed marble workshops lose the
            // marble on the premises; a cursed Artisans' Guild's artisan
            // out working dies.
            if(bt == eBuildingType::masonryShop ||
               bt == eBuildingType::blackMarbleWorkshop) {
                if(const auto rb = dynamic_cast<eResourceBuildingBase*>(b)) {
                    rb->take(rb->resourceType(), rb->resource());
                }
            } else if(const auto g = dynamic_cast<eArtisansGuild*>(b)) {
                g->killArtisan();
            }
        }
    };
    curseType(target->type());

    // the god's extra (zeus.exe 0x004EDFC0), on every curse
    const auto walkersInCity = [&]() {
        std::vector<eCharacter*> r;
        for(const auto ch : board.characters()) {
            if(!ch || ch->dead()) continue;
            const auto t = ch->tile();
            if(!t || t->cityId() != cid) continue;
            if(!eDionysusFollowAction::sShouldFollow(ch->type())) continue;
            r.push_back(ch);
        }
        return r;
    };
    switch(god) {
    case eGodType::hera: // walkers turn into cows, walk home and vanish
        for(const auto ch : walkersInCity()) ch->kill();
        break;
    case eGodType::hades: // the same walkers die
        for(const auto ch : walkersInCity()) ch->killWithCorpse();
        break;
    case eGodType::athena: // walls and towers too (type slots 130, 132)
        curseType(eBuildingType::wall);
        curseType(eBuildingType::tower);
        break;
    case eGodType::apollo: // a plague outbreak (0x004481B0)
        if(const auto c = board.boardCityWithId(cid)) c->godPlagueOutbreak();
        break;
    default:
        break;
    }
}
