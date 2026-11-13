#include "ekrakendefenderaction.h"

#include "emovearoundaction.h"

eKrakenDefenderAction::eKrakenDefenderAction(eCharacter* const c) :
    eFightingAction(c, eCharActionType::krakenDefenderAction) {}

bool eKrakenDefenderAction::decide() {
    const auto c = character();
    const auto t = c->tile();
    if(!t) return true;
    c->setActionType(eCharacterActionType::walk);
    const auto a = e::make_shared<eMoveAroundAction>(
                       c, t->x(), t->y(),
                       eWalkableObject::sCreateDeepWater());
    a->setMaxDistance(15);
    a->setTime(5000);
    setCurrentAction(a);
    setOverwrittableAction(true);
    return true;
}

void eKrakenDefenderAction::increment(const int by) {
    const auto r = lookForEnemy(by);
    if(r == eLookForEnemyState::dead) return;
    eComplexAction::increment(by);
}

void eKrakenDefenderAction::goHome() {
    decide();
}
