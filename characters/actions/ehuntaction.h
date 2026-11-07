#ifndef EHUNTACTION_H
#define EHUNTACTION_H

#include "eactionwithcomeback.h"

#include "characters/ehunter.h"

class eHuntingLodge;

class eHuntAction : public eActionWithComeback {
public:
    eHuntAction(eHuntingLodge* const b,
                eCharacter* const c);
    eHuntAction(eCharacter* const c);

    void increment(const int by) override;
    bool decide() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    void findResourceDecision();
    void goBackDecision();
    void waitDecision();
    // hunters throw spears at prey within range (Figure_model: Hunter,
    // missile range 3); not saved, a loaded hunter looks again
    bool findPrey();
    bool throwSpear(const int by);

    eHuntingLodge* mLodge = nullptr;
    eHunter* mHunter = nullptr;

    bool mNoResource = false;

    stdptr<eCharacter> mPrey;
    int mPreyScan = 0;
    int mSpearTime = 0;
};

#endif // EHUNTACTION_H
