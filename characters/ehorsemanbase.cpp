#include "ehorsemanbase.h"

#include "enumbers.h"
#include "efigurestats.h"

eHorsemanBase::eHorsemanBase(eGameBoard& board,
                             const eCharTexs charTexs,
                             const eCharacterType type) :
    eSoldier(board, charTexs, type) {
    setAttack(eNumbers::sHorsemanAttack);
    setSpeed(eNumbers::sHorsemanSpeed);
    setHP(eNumbers::sHorsemanHP);
    // original-game strength for this soldier type, if it has one
    eFigureStats::sApply(this);
}
