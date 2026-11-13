#ifndef EMONSTERINVASIONEVENTBASE_H
#define EMONSTERINVASIONEVENTBASE_H

#include "egameevent.h"
#include "epointeventvalue.h"
#include "emonsterseventvalue.h"

#include "characters/monsters/emonster.h"
#include "characters/monsters/emonstertarget.h"

class eMonsterInvasionEventBase : public eGameEvent,
                                  public ePointEventValue,
                                  public eMonstersEventValue {
public:
    eMonsterInvasionEventBase(const eCityId cid,
                              const eGameEventType type,
                              const eGameEventBranch branch,
                              eGameBoard& board);
    ~eMonsterInvasionEventBase();

    eMonsterAggressivness aggressivness() const
    { return mAggressivness; }
    void setAggressivness(const eMonsterAggressivness a)
    { mAggressivness = a; }

    void chooseMonster();

    // The adventure gives each monster slot a target. The monster appears
    // at the monster marker nearest what it aims at (a monster aimed at
    // industry appears by marble or ore: the Hydra guards the marble in
    // Zeus and Europa, per Nicolás) and prefers to attack it.
    void addMonsterTarget(const eMonsterType m, const eMonsterTarget t)
    { mMonsterTargets.push_back({m, t}); }
    bool valid() const { return mValid; }

    void write(eWriteStream& dst) const override;
    void read(eReadStream& src) override;

    bool finished() const override;

    void killed(const eMonsterType monster);

    eEventTrigger& killedTrigger() { return *mKilledTrigger; }
protected:
    eMonster* triggerBase();
private:
    stdsptr<eEventTrigger> mKilledTrigger;
    bool mChooseMonster = false;
    eMonsterAggressivness mAggressivness = eMonsterAggressivness::passive;

    bool mValid = false;
    std::vector<eMonsterType> mKilled;
    std::vector<eMonsterType> mSpawned;
    std::vector<std::pair<eMonsterType, eMonsterTarget>> mMonsterTargets;

    eMonsterTarget targetOf(const eMonsterType m) const;
    void chooseTargetPoint(const eMonsterTarget t);
};

#endif // EMONSTERINVASIONEVENTBASE_H
