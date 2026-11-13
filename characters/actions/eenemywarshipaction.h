#ifndef EENEMYWARSHIPACTION_H
#define EENEMYWARSHIPACTION_H

#include "efightingaction.h"

// What an invader's warship does, after the original game (zeus.exe
// 0x004C2980 and the functions it calls; times are the original's
// figure ticks, about 13 eZeus ticks each):
// - wait: the warships set off one after another, the n-th after
//   25n + 10 ticks
// - patrol: it sails between the river entry, the invasion's landing
//   point and the river exit for 75 to 104 ticks, then hunts
// - hunt: it picks a target anywhere in the city: a trireme scores 500
//   (1000 if the trireme is fighting a transport), less 10 for each
//   warship already after it; a trade or fishing boat 10; a rabble,
//   hoplite or horseman 100, but only within its missile range; the
//   highest score less the distance wins. It sails towards boats until
//   within 3 tiles and picks again every 24 ticks. With no target it
//   patrols again.
// - below 16% of its hit points it keeps away from the player's
//   triremes and fishing boats, moving 10 tiles away every 75 ticks
// - it shoots at the best target within its missile range, whatever it
//   is doing, and never attacks buildings
// - it sails away through the river exit in the fourth month after it
//   arrived; each time it takes on a trireme or a soldier there is an
//   even chance this moves to the fifth month from now
enum class eEnemyWarshipStage {
    wait, patrol, hunt, retreat, leave
};

class eEnemyWarshipAction : public eFightingAction {
public:
    eEnemyWarshipAction(eCharacter* const c);

    void setup(const int index, eTile* const disembark);

    bool decide() override;
    void increment(const int by) override;

    void goHome() override;
    void goAbroad() override {}

    eCharacter* huntTarget() const { return mTarget.get(); }

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
protected:
    bool ownTargeting() const override { return true; }
private:
    int monthIndex() const;
    // the original's target choice (exe 0x004E1830); maxDist < 0 for
    // anywhere in the city
    eCharacter* findTarget(const int maxDist) const;
    void startPatrol();
    void startHunt();
    void startLeaving();
    void retreat();
    void sailTo(eTile* const t);
    eTile* waypoint(const int i) const;

    eEnemyWarshipStage mStage = eEnemyWarshipStage::wait;
    int mWait = 0;
    int mTimer = 0;
    int mPatrolTime = 0;
    int mShoot = 0;
    int mWaypoint = 0;
    int mLeaveMonth = 0;
    int mDisembarkX = -1;
    int mDisembarkY = -1;
    double mFullHP = 0;
    bool mRestNext = false; // the last route failed
    stdptr<eCharacter> mTarget;
};

#endif // EENEMYWARSHIPACTION_H
