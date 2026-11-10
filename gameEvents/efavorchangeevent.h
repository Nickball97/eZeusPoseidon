#ifndef EFAVORCHANGEEVENT_H
#define EFAVORCHANGEEVENT_H

#include "eeconomicmilitarychangeeventbase.h"

// Original adventure events 17 (favor increase) and 18 (favor decrease):
// a city's opinion of the player goes up or down by the amount
// (zeus.exe 0x0046E840 and 0x0046E970). A negative count lowers it; a
// count of 0 does nothing but set off the linked events (used for the
// original's trigger-only event types 6, 7, 10 and 11, zeus.exe
// 0x00472EC0).
class eFavorChangeEvent : public eEconomicMilitaryChangeEventBase {
public:
    eFavorChangeEvent(const eCityId cid,
                      const eGameEventBranch branch,
                      eGameBoard& board);

    void trigger() override;
    std::string longName() const override;
};

#endif // EFAVORCHANGEEVENT_H
