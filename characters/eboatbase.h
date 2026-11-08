#ifndef EBOATBASE_H
#define EBOATBASE_H

#include "echaracter.h"

#include "textures/echaractertextures.h"

class eBoatBase : public eCharacter {
public:
    using eCharTexs = eTradeBoatTextures eCharacterTextures::*;
    eBoatBase(eGameBoard& board, const eCharTexs charTexs,
              const eCharacterType type);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const;

    void setCharTexs(const eCharTexs& texs);
protected:
    // which texture set to draw; triremes switch to frigates
    virtual eCharTexs charTexs() const { return mCharTexs; }
private:
    eCharTexs mCharTexs;
};

#endif // EBOATBASE_H
