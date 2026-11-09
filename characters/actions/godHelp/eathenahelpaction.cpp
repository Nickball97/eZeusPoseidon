#include "eathenahelpaction.h"

eAthenaHelpAction::eAthenaHelpAction(eCharacter* const c) :
    eProvideResourceHelpAction(c, eCharActionType::athenaHelpAction,
                               eResourceType::oliveOil, 16) {
    // Athena brings olives as well as olive oil (Citadel of Poseidon
    // walkthroughs: Trojan War ep. 3, Perseus ep. 3, Jason ep. 3)
    setNextResource(eResourceType::olives, 16);
}

bool eAthenaHelpAction::sHelpNeeded(const eCityId cid,
                                    const eGameBoard& board) {
    return eProvideResourceHelpAction::sHelpNeeded(
                cid, board, eResourceType::oliveOil, 8);
}

