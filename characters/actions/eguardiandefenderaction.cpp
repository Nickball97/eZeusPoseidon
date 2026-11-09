#include "eguardiandefenderaction.h"

#include "emovearoundaction.h"

eGuardianDefenderAction::eGuardianDefenderAction(eCharacter* const c) :
    eFightingAction(c, eCharActionType::guardianDefenderAction) {}

bool eGuardianDefenderAction::decide() {
    const auto c = character();
    const auto t = c->tile();
    if(!t) return true;
    c->setActionType(eCharacterActionType::walk);
    const auto a = e::make_shared<eMoveAroundAction>(
                       c, t->x(), t->y(),
                       eWalkableObject::sCreateDefault());
    a->setMaxDistance(20);
    a->setTime(5000);
    setCurrentAction(a);
    setOverwrittableAction(true);
    return true;
}

void eGuardianDefenderAction::increment(const int by) {
    const auto r = lookForEnemy(by);
    if(r == eLookForEnemyState::dead) return;
    eComplexAction::increment(by);
}

void eGuardianDefenderAction::goHome() {
    decide();
}
