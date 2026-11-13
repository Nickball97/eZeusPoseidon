#ifndef EARTEMISHUNTACTION_H
#define EARTEMISHUNTACTION_H

#include "characters/actions/egodaction.h"

// Nicolás's mod rule: once her sanctuary is finished, Artemis hunts down
// every wolf in her city with her arrows, then goes home (the original
// only sends her after a wolf that attacks, exe 0x0056AF80)

enum class eArtemisHuntStage {
    none, hunt, back
};

class eLookForWolfGodAct : public eGodAct {
public:
    eLookForWolfGodAct(eGameBoard& board, const eCityId cid);
    eLookForWolfGodAct(eGameBoard& board);

    eMissileTarget find(eTile* const t) override;
    void act() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    eCityId mCityId = eCityId::neutralFriendly;
    stdptr<eCharacter> mTarget;
};

class eArtemisHuntAction : public eGodAction {
public:
    eArtemisHuntAction(eCharacter* const c);

    void increment(const int by) override;
    bool decide() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    // a living wolf in the city, the nearest to tile t if given
    static eCharacter* sWolf(const eGameBoard& board, const eCityId cid,
                             eTile* const t = nullptr);
private:
    void huntNext();

    eArtemisHuntStage mStage{eArtemisHuntStage::none};
    int mLookTime = 0;
};

#endif // EARTEMISHUNTACTION_H
