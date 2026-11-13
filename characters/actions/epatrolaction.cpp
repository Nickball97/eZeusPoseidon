#include "epatrolaction.h"

#include "../echaracter.h"
#include "engine/etile.h"
#include "epatrolmoveaction.h"
#include "buildings/epatrolbuildingbase.h"
#include "engine/egameboard.h"

ePatrolAction::ePatrolAction(eCharacter* const c,
                             ePatrolBuildingBase* const b,
                             const std::vector<eOrientation>& path,
                             const stdsptr<eDirectionTimes>& dirTimes,
                             const eCharActionType at) :
    eActionWithComeback(c, b ? b->centerTile() : nullptr, at),
    mPath(path), mDirTimes(dirTimes), mBuilding(b) {
    setFinishOnComeback(true);
    setDiagonalOnly(true);
}

ePatrolAction::ePatrolAction(eCharacter* const c,
                             const eCharActionType at) :
    ePatrolAction(c, nullptr, {}, nullptr, at) {}

bool ePatrolAction::decide() {
    const bool r = eActionWithComeback::decide();
    if(r) return r;
    if(mDone) {
        goBackDecision();
    } else {
        patrol();
    }
    return true;
}

void ePatrolAction::read(eReadStream& src) {
    eActionWithComeback::read(src);
    int n;
    src >> n;
    for(int i = 0; i < n; i++) {
        auto& o = mPath.emplace_back();
        src >> o;
    }
    src.readBuilding(&board(), [this](eBuilding* const b) {
        mBuilding = static_cast<ePatrolBuildingBase*>(b);
        // A walker and its building share one memory of the directions
        // taken, so the next walker goes where the last one did not. A
        // save holds a copy for each; join them up again.
        if(!mBuilding) return;
        const auto& times = mBuilding->dirTimes();
        if(!times) return;
        mDirTimes = times;
        const auto ca = currentAction();
        const auto move = dynamic_cast<ePatrolMoveAction*>(ca);
        if(move) move->setDirectionTimes(times);
    });
    src >> mDone;
    mDirTimes = src.readDirectionTimes(board());
}

void ePatrolAction::write(eWriteStream& dst) const {
    eActionWithComeback::write(dst);
    dst << mPath.size();
    for(const auto o : mPath) {
        dst << o;
    }
    dst.writeBuilding(mBuilding);
    dst << mDone;
    dst.writeDirectionTimes(mDirTimes.get());
}

void ePatrolAction::patrol() {
    const auto c = character();
    c->setActionType(eCharacterActionType::walk);
    const auto failFunc = std::make_shared<ePA_patrolFail>(
                              board(), this);
    const auto finishFunc = std::make_shared<ePA_patrolFinish>(
                                board(), this);
    const int dist = mBuilding->maxDistance();
    if(mPath.empty()) {
        const auto walkable = eWalkableObject::sCreateRoadblock();
        const auto a = e::make_shared<ePatrolMoveAction>(
            c, true, walkable, mDirTimes);
        a->setFailAction(failFunc);
        a->setFinishAction(finishFunc);
        a->setMaxWalkDistance(dist);
        setCurrentAction(a);
    } else {
        auto walkable = eWalkableObject::sCreateRoadAvenue();
        walkable = eWalkableObject::sCreateRect(mBuilding, walkable);
        const auto a = e::make_shared<eMovePathAction>(
            c, mPath, walkable);
        a->setFailAction(failFunc);
        a->setFinishAction(finishFunc);
        setCurrentAction(a);
    }
}

void ePatrolAction::goBackDecision(const stdsptr<eWalkableObject>& w) {
    // walkers from fountains and infirmaries return to the road tile
    // directly north of the building, the one touching only its corner
    // (Citadel of Poseidon, walker distances)
    if(mBuilding) {
        const auto bt = mBuilding->type();
        if(bt == eBuildingType::fountain || bt == eBuildingType::hospital) {
            const auto tr = mBuilding->tileRect();
            const auto t = board().tile(tr.x - 1, tr.y - 1);
            if(t && t->hasRoad()) {
                setStartTile(t);
                goBack(w);
                return;
            }
        }
    }
    goBack(mBuilding, w);
}
