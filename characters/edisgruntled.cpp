#include "edisgruntled.h"

#include "textures/egametextures.h"

eDisgruntled::eDisgruntled(eGameBoard& board) :
    eDisgruntled(board, eCharacterType::disgruntled) {}

eDisgruntled::eDisgruntled(eGameBoard& board, const eCharacterType type) :
    eFightingPatroler(board, &eCharacterTextures::fDisgruntled, type) {
    eGameTextures::loadDisgruntled();
    setAttack(0.2);
}
