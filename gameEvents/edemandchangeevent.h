#ifndef EDEMANDCHANGEEVENT_H
#define EDEMANDCHANGEEVENT_H

#include "esupplydemandchangeevent.h"

class eDemandChangeEvent : public eSupplyDemandChangeEvent {
public:
    eDemandChangeEvent(const eCityId cid,
                       const eGameEventBranch branch,
                       eGameBoard& board);

    void trigger() override;
    bool oraclePrediction(eEventData& ed, eMessageType& msg) override;
    std::string longName() const override;
};

#endif // EDEMANDCHANGEEVENT_H
