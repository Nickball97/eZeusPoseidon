#ifndef EOUTLAWACTION_H
#define EOUTLAWACTION_H

#include "esickdisgruntledaction.h"

// Outlaws from the Zeus manual ("Unrest, Disgruntled Citizens and
// Outlaws"):
// - muggers roam the streets attacking people and might kill a few,
// - thieves go to an elite house and steal its goods and food,
// - vandals go to an elite house and raze it.
// Thieves and vandals with no elite house to go to roam like muggers.
// Watchmen and noblemen stop them on the way.
class eOutlawAction : public eSickDisgruntledAction {
public:
    eOutlawAction(eCharacter* const c, eBuilding* const home);

    bool decide() override;
    void increment(const int by) override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    void goToTarget();
    void actOnTarget();
    void tryMugging();

    stdptr<eBuilding> mTarget;
    int mStage = 0; // 0 find target, 1 walking to it, 2 going home, 3 roam
    int mMugTime = 0;
};

#endif // EOUTLAWACTION_H
