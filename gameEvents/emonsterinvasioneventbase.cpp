#include "emonsterinvasioneventbase.h"

#include "engine/egameboard.h"
#include "engine/eeventdata.h"
#include "characters/actions/emonsteraction.h"
#include "eiteratesquare.h"
#include "evectorhelpers.h"
#include "emessages.h"
#include "fileIO/efileformat.h"
#include "buildings/eheatgetters.h"

eMonsterInvasionEventBase::eMonsterInvasionEventBase(
        const eCityId cid,
        const eGameEventType type,
        const eGameEventBranch branch,
        eGameBoard& board) :
    eGameEvent(cid, type, branch, board),
    ePointEventValue(eBannerTypeS::monsterPoint,
                    cid, board) {
    const auto e4 = eLanguage::text("killed_trigger");
    mKilledTrigger = e::make_shared<eEventTrigger>(cid, e4, board);
    addTrigger(mKilledTrigger);
}

eMonsterInvasionEventBase::~eMonsterInvasionEventBase() {
    const auto board = gameBoard();
    if(board) board->removeMonsterEvent(this);
}

void eMonsterInvasionEventBase::chooseMonster() {
    if(mChooseMonster) return;
    mChooseMonster = true;
    mValid = eMonstersEventValue::chooseMonster(mSpawned);
}

void eMonsterInvasionEventBase::write(eWriteStream& dst) const {
    eGameEvent::write(dst);
    ePointEventValue::write(dst);
    eMonstersEventValue::write(dst);
    dst << mChooseMonster;
    dst << mAggressivness;
    dst << mValid;

    dst << mSpawned.size();
    for(const auto s : mSpawned) {
        dst << s;
    }

    dst << mKilled.size();
    for(const auto k : mKilled) {
        dst << k;
    }

    dst << 0; // old industry-only list (format monsterTarget)
    dst << mMonsterTargets.size();
    for(const auto& m : mMonsterTargets) {
        dst << m.first;
        dst << m.second;
    }
}

void eMonsterInvasionEventBase::read(eReadStream& src) {
    eGameEvent::read(src);
    ePointEventValue::read(src);
    eMonstersEventValue::read(src);
    src >> mChooseMonster;
    src >> mAggressivness;
    src >> mValid;

    int ns;
    src >> ns;
    for(int i = 0; i < ns; i++) {
        src >> mSpawned.emplace_back();
    }

    int nk;
    src >> nk;
    for(int i = 0; i < nk; i++) {
        src >> mKilled.emplace_back();
    }

    if(src.formatVersion() >= eFileFormat::monsterTarget) {
        int ni;
        src >> ni;
        for(int i = 0; i < ni; i++) {
            eMonsterType m;
            src >> m;
            mMonsterTargets.push_back({m, eMonsterTarget::industry});
        }
    }
    if(src.formatVersion() >= eFileFormat::monsterTargets) {
        int nt;
        src >> nt;
        for(int i = 0; i < nt; i++) {
            eMonsterType m;
            eMonsterTarget t;
            src >> m;
            src >> t;
            mMonsterTargets.push_back({m, t});
        }
    }
}

bool eMonsterInvasionEventBase::finished() const {
    for(const auto s : mSpawned) {
        const bool c = eVectorHelpers::contains(mKilled, s);
        if(!c) return false;
    }
    return eGameEvent::finished();
}

void eMonsterInvasionEventBase::killed(const eMonsterType monster) {
    const auto board = gameBoard();
    if(!board) return;
    mKilled.push_back(monster);
    const auto date = board->date();
    const auto& msgs = eMessages::instance;
    const auto monsterMsgs = msgs.monsterMessages(monster);
    const auto rFull = monsterMsgs->fSlainReason;
    mKilledTrigger->trigger(*this, date, rFull);
}

eMonster* eMonsterInvasionEventBase::triggerBase() {
    const auto board = gameBoard();
    if(!board) return nullptr;
    if(!mChooseMonster) chooseMonster();
    if(!mValid) return nullptr;
    mSpawned.push_back(mMonster);
    board->addMonsterEvent(mMonster, this);
    mChooseMonster = false;
    choosePointId();
    const auto target = targetOf(mMonster);
    chooseTargetPoint(target);
    const auto cid = cityId();
    const auto monster = eMonster::sCreateMonster(mMonster, *board);
    board->registerMonster(cid, monster.get());
    monster->setOnCityId(cid);
    monster->setCityId(eCityId::neutralAggresive);

    const auto a = e::make_shared<eMonsterAction>(monster.get());
    a->setAggressivness(mAggressivness);
    a->setTarget(target);
    monster->setAction(a);
    const int ptId = pointId();
    const auto tile = board->monsterTile(cid, ptId);
    if(tile) {
        const int tx = tile->x();
        const int ty = tile->y();
        const auto placeMonster = [board, monster, tx, ty, cid](
                                  const int dx, const int dy) {
            const int ttx = tx + dx;
            const int tty = ty + dy;
            const auto tile = board->tile(ttx, tty);
            if(!tile) return false;
            const auto tcid = tile->cityId();
            if(cid != tcid) return false;
            if(const auto ub = tile->underBuilding()) {
                const auto type = ub->type();
                const bool w = eBuilding::sWalkableBuilding(type);
                if(!w) {
                    const bool a = eBuilding::sAttackable(type);
                    if(a) {
                        ub->collapse();
                    } else {
                        return false;
                    }
                }
            }
            monster->changeTile(tile);
            return true;
        };
        int k = 0;
        while(!monster->tile()) {
            eIterateSquare::iterateSquare(k++, placeMonster);
        }
    }
    a->increment(1);

    return monster.get();
}

eMonsterTarget eMonsterInvasionEventBase::targetOf(const eMonsterType m) const {
    for(const auto& t : mMonsterTargets) {
        if(t.first == m) return t.second;
    }
    return eMonsterTarget::random;
}

void eMonsterInvasionEventBase::chooseTargetPoint(const eMonsterTarget target) {
    if(target == eMonsterTarget::best ||
       target == eMonsterTarget::random) return;
    const auto board = gameBoard();
    if(!board) return;
    const auto cid = cityId();
    std::vector<eTile*> res;
    board->iterateOverAllTiles([&](eTile* const t) {
        if(t->cityId() != cid) return;
        if(target == eMonsterTarget::industry) {
            const auto terr = t->terrain();
            if(terr == eTerrain::marble || terr == eTerrain::blackMarble ||
               terr == eTerrain::copper || terr == eTerrain::silver ||
               terr == eTerrain::orichalc) {
                res.push_back(t);
                return;
            }
        } else if(target == eMonsterTarget::sea) {
            if(t->hasWater() && !t->isShoreTile()) {
                res.push_back(t);
                return;
            }
        }
        const auto bt = t->underBuildingType();
        if(eHeatGetters::isMonsterTarget(target, bt)) res.push_back(t);
    });
    if(res.empty()) return;
    int bestId = -1;
    int bestDist = __INT_MAX__;
    for(int i = minPointId(); i <= maxPointId(); i++) {
        const auto pt = board->monsterTile(cid, i);
        if(!pt) continue;
        for(const auto t : res) {
            const int dx = t->x() - pt->x();
            const int dy = t->y() - pt->y();
            const int d = dx*dx + dy*dy;
            if(d < bestDist) {
                bestDist = d;
                bestId = i;
            }
        }
    }
    if(bestId > 0) {
        printf("Monster aimed at target %i: marker %i\n",
               static_cast<int>(target), bestId);
        setPointId(bestId);
    }
}
