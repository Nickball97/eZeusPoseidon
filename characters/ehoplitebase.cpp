#include "ehoplitebase.h"

#include "enumbers.h"
#include "efigurestats.h"

eHopliteBase::eHopliteBase(eGameBoard& board,
                           const eCharTexs charTexs,
                           const eCharacterType type) :
    eSoldier(board, charTexs, type) {
    setAttack(eNumbers::sHopliteAttack);
    setHP(eNumbers::sHopliteHP);
    // original-game strength for this soldier type, if it has one
    eFigureStats::sApply(this);
}
