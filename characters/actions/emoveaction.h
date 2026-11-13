#ifndef EMOVEACTION_H
#define EMOVEACTION_H

#include "echaracteraction.h"
#include "engine/eorientation.h"
#include "walkable/eobsticlehandler.h"

class eTileBase;
class eTile;

class eMoveAction : public eCharacterAction {
public:
    eMoveAction(eCharacter* const c,
                const stdsptr<eWalkableObject>& tileWalkable,
                const eCharActionType type);
    eMoveAction(eCharacter* const c, const eCharActionType type);

    bool walkable(eTileBase* const tile) const;

    void setObsticleHandler(const stdsptr<eObsticleHandler>& oh);

    void increment(const int by) override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
protected:
    bool nextTurn();
    void setWait(const bool w) { mWait = w; }
private:
    virtual eCharacterActionState nextTurn(eOrientation& turn) = 0;
    void moveBy(const double inc);
    void moveToTargetTile();

    stdsptr<eWalkableObject> mTileWalkable;

    eOrientation mOrientation = eOrientation::topRight;
    eTile* mTargetTile = nullptr;

    bool mWait = false;

    double mStartX = 0.;
    double mStartY = 0.;
    // set by the first step; given a value so a save made before that
    // does not hold leftover memory
    double mTargetX = 0.;
    double mTargetY = 0.;

    stdsptr<eObsticleHandler> mObstHandler;
};

#endif // EMOVEACTION_H
