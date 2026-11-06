#include "erockthrowerbase.h"

#include "enumbers.h"
#include "efigurestats.h"

eRockThrowerBase::eRockThrowerBase(eGameBoard& board,
                                   const eCharTexs charTexs,
                                   const eCharacterType type) :
    eRangeSoldier(board, charTexs, type, eNumbers::sRabbleRange) {
    setAttack(eNumbers::sRabbleAttack);
    setHP(eNumbers::sRabbleHP);
    // original-game strength for this soldier type, if it has one
    eFigureStats::sApply(this);
}
