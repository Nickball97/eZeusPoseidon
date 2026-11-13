#ifndef EHUNTACTION_H
#define EHUNTACTION_H

#include "eactionwithcomeback.h"

#include "characters/ehunter.h"

#include <vector>

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
    // 1 boar in 5 near the hunter charges (not saved; a loaded hunter
    // simply judges nearby boars again)
    void provokeBoars();

    eHuntingLodge* mLodge = nullptr;
    eHunter* mHunter = nullptr;

    bool mNoResource = false;

    int mPreyScan = 0;
    // Pointers that know when their boar is gone (not just where it sat in
    // memory): a boar that died could otherwise be mistaken for a new one
    // that happens to be put in the same place in memory.
    std::vector<stdptr<eCharacter>> mJudged;
};

#endif // EHUNTACTION_H
