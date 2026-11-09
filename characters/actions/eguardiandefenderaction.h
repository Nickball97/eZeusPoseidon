#ifndef EGUARDIANDEFENDERACTION_H
#define EGUARDIANDEFENDERACTION_H

#include "efightingaction.h"

// A god's guardian on land, sent when his sanctuary is finished: Ares'
// Dragon "kills invader, wolf and predator boar" and Hades' Cerberus
// "defends city" (Citadel of Poseidon, sanctuary.html). It roams around
// the sanctuary and fights any enemy that comes near, like the Kraken
// does in the water.
class eGuardianDefenderAction : public eFightingAction {
public:
    eGuardianDefenderAction(eCharacter* const c);

    bool decide() override;
    void increment(const int by) override;

    void goHome() override;
    void goAbroad() override {}
};

#endif // EGUARDIANDEFENDERACTION_H
