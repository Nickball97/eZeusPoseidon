#include "eamazon.h"

#include "textures/egametextures.h"
#include "enumbers.h"
#include "efigurestats.h"

eAmazon::eAmazon(eGameBoard& board) :
    eSoldier(board, nullptr, eCharacterType::amazon) {
    setIsArcher(false);
}

void eAmazon::setIsArcher(const bool a) {
    mIsArcher = a;
    if(a) {
        eGameTextures::loadAmazonArcher();
        setCharTexs(&eCharacterTextures::fAmazonArcher);
        setRange(eNumbers::sArcherRange);
        setAttack(eNumbers::sArcherAttack);
        setHP(eNumbers::sArcherHP);
        eFigureStats::sApply(this, eFigureStats::sAmazonArcher);
    } else {
        eGameTextures::loadAmazonSpear();
        setCharTexs(&eCharacterTextures::fAmazonSpear);
        setRange(0);
        setAttack(eNumbers::sHopliteAttack);
        setHP(eNumbers::sHopliteHP);
        eFigureStats::sApply(this, eFigureStats::sAmazonWarrior);
    }
}

void eAmazon::read(eReadStream& src) {
    eSoldier::read(src);
    bool archer;
    src >> archer;
    // keep the loaded hit points (setIsArcher resets them to full)
    const double hp = this->hp();
    setIsArcher(archer);
    setHP(hp);
}

void eAmazon::write(eWriteStream& dst) const {
    eSoldier::write(dst);
    dst << mIsArcher;
}

