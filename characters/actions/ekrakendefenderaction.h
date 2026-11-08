#ifndef EKRAKENDEFENDERACTION_H
#define EKRAKENDEFENDERACTION_H

#include "efightingaction.h"

// Poseidon's Kraken, sent when his sanctuary is finished: it patrols the
// city's waters and kills any enemies that enter them (Zeus manual),
// including Oceanids swimming under water (Poseidon manual).
class eKrakenDefenderAction : public eFightingAction {
public:
    eKrakenDefenderAction(eCharacter* const c);

    bool decide() override;
    void increment(const int by) override;

    void goHome() override;
    void goAbroad() override {}
};

#endif // EKRAKENDEFENDERACTION_H
