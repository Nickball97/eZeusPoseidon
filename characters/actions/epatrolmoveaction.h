#ifndef EPATROLMOVEACTION_H
#define EPATROLMOVEACTION_H

#include "emoveaction.h"
#include "engine/emovedirection.h"
#include "ewalkablehelpers.h"

#include <SDL2/SDL_rect.h>

class eMovePathAction;

class eBuilding;

class eDirectionLastUseTime {
public:
    eDirectionLastUseTime() {
        for(int i = 0; i < 8; i++) {
            mTime.push_back(0);
        }
    }

    int& time(const eOrientation o) {
        const int id = static_cast<int>(o);
        return mTime[id];
    }

    void read(eReadStream& src) {
        // Reading a list adds to what is already in it, and this one
        // starts with 8 zeros: the saved times used to land after them,
        // where nothing looks, so every load wiped the walkers' memory of
        // the directions they had taken and made the save file longer.
        mTime.clear();
        src >> mTime;
        // a save made with that mistake holds the real times last
        if(mTime.size() > 8) mTime.erase(mTime.begin(), mTime.end() - 8);
        mTime.resize(8, 0);
    }

    void write(eWriteStream& dst) const {
        dst << mTime;
    }
private:
    std::vector<int> mTime;
};

using eDirectionTimes = std::map<eTile*, eDirectionLastUseTime>;

class ePatrolMoveAction : public eMoveAction {
public:
    ePatrolMoveAction(eCharacter* const c,
                      const bool diagonalOnly = true,
                      const stdsptr<eWalkableObject>& walkable =
                        eWalkableObject::sCreateRoad(),
                      const stdsptr<eDirectionTimes>& os =
                        std::make_shared<eDirectionTimes>());

    // after loading: use the building's shared memory of directions
    void setDirectionTimes(const stdsptr<eDirectionTimes>& os)
    { if(os) mOs = os; }

    void setMaxWalkDistance(const int dist)
    { mMaxWalkDistance = dist; }

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    eCharacterActionState nextTurn(eOrientation& t) override;

    bool mDiagonalOnly;
    stdsptr<eWalkableObject> mWalkable;
    stdsptr<eDirectionTimes> mOs;

    eOrientation mO{eOrientation::topRight};
    int mMaxWalkDistance = 10000;
    int mWalkedDistance = 0;
};

#endif // EPATROLMOVEACTION_H
