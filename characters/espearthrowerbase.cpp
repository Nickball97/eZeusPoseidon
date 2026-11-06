#include "espearthrowerbase.h"

#include "enumbers.h"
#include "efigurestats.h"

eSpearthrowerBase::eSpearthrowerBase(eGameBoard& board,
                                     const eCharTexs charTexs,
                                     const eCharacterType type) :
    eSoldier(board, charTexs, type) {
    setRange(eNumbers::sSpearthrowerRange);
    setAttack(eNumbers::sSpearthrowerAttack);
    setHP(eNumbers::sSpearthrowerHP);
    // original-game strength for this soldier type, if it has one
    eFigureStats::sApply(this);
}
