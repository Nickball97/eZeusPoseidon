#ifndef ESUPPLYCHANGEEVENT_H
#define ESUPPLYCHANGEEVENT_H

#include "esupplydemandchangeevent.h"

class eSupplyChangeEvent : public eSupplyDemandChangeEvent {
public:
    eSupplyChangeEvent(const eCityId cid,
                       const eGameEventBranch branch,
                       eGameBoard& board);

    void trigger() override;
    bool oraclePrediction(eEventData& ed, eMessageType& msg) override;
    std::string longName() const override;
};

#endif // ESUPPLYCHANGEEVENT_H
