#include "earcherbase.h"

#include "enumbers.h"
#include "efigurestats.h"

eArcherBase::eArcherBase(eGameBoard& board,
                         const eCharTexs charTexs,
                         const eCharacterType type) :
    eSoldier(board, charTexs, type) {
    setRange(eNumbers::sArcherRange);
    setAttack(eNumbers::sArcherAttack);
    setHP(eNumbers::sArcherHP);
    // original-game strength for this soldier type, if it has one
    eFigureStats::sApply(this);
}
