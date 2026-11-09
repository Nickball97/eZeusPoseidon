#ifndef EDISGRUNTLED_H
#define EDISGRUNTLED_H

#include "efightingpatroler.h"

class eDisgruntled : public eFightingPatroler {
public:
    eDisgruntled(eGameBoard& board);
protected:
    eDisgruntled(eGameBoard& board, const eCharacterType type);
};

// Outlaws (Zeus manual, "Unrest, Disgruntled Citizens and Outlaws").
// The original game data gives all of them the same criminal figure type,
// 10 hit points and no attack; they reuse the disgruntled citizen art.
class eMugger : public eDisgruntled {
public:
    eMugger(eGameBoard& board) :
        eDisgruntled(board, eCharacterType::mugger) {}
};

class eThief : public eDisgruntled {
public:
    eThief(eGameBoard& board) :
        eDisgruntled(board, eCharacterType::thief) {}
};

class eVandal : public eDisgruntled {
public:
    eVandal(eGameBoard& board) :
        eDisgruntled(board, eCharacterType::vandal) {}
};

#endif // EDISGRUNTLED_H
