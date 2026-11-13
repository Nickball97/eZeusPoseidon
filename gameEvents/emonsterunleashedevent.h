#ifndef EMONSTERUNLEASHEDEVENT_H
#define EMONSTERUNLEASHEDEVENT_H

#include "emonsterinvasioneventbase.h"

class eMonsterUnleashedEvent : public eMonsterInvasionEventBase {
public:
    eMonsterUnleashedEvent(const eCityId cid,
                           const eGameEventBranch branch,
                           eGameBoard& board);

    void trigger() override;
    bool oraclePrediction(eEventData& ed, eMessageType& msg) override;
    std::string longName() const override;
};

#endif // EMONSTERUNLEASHEDEVENT_H
