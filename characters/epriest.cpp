#include "epriest.h"

#include <algorithm>

#include "textures/egametextures.h"
#include "actions/epriestaction.h"

ePriest::ePriest(eGameBoard& board) :
    eCharacter(board, eCharacterType::priest) {
    eGameTextures::loadPriest();
    eGameTextures::loadCart();
    setHasSecondaryTexture(true);
}

int ePriest::pickUpTime() const {
    // the original plays the lifting animation once: 18 frames for a
    // goat, 9 for a sheep (exe 0x004CA350, art 0x6694 and 0x6696)
    const int frames = mLoad == ePriestLoad::goat ? 18 : 9;
    return frames*sTextureTimeDivisor;
}

std::shared_ptr<eTexture> ePriest::getTexture(const eTileSize size) const {
    const int id = static_cast<int>(size);
    const auto& texs = eGameTextures::characters()[id].fPriest;
    const int oid = static_cast<int>(rotatedOrientation());
    const eTextureCollection* coll = nullptr;
    bool wrap = true;
    switch(actionType()) {
    case eCharacterActionType::stand:
        return texs.fWalk[oid].getTexture(0);
    case eCharacterActionType::walk:
        coll = &texs.fWalk[oid];
        break;
    case eCharacterActionType::carry:
        if(mLoad == ePriestLoad::goat) {
            coll = &texs.fCarryGoat[oid];
        } else if(mLoad == ePriestLoad::sheep) {
            coll = &texs.fCarrySheep[oid];
        } else {
            coll = &texs.fWalk[oid];
        }
        break;
    case eCharacterActionType::collect:
        wrap = false;
        if(mLoad == ePriestLoad::goat) {
            coll = &texs.fPickGoat;
        } else {
            coll = &texs.fPickSheep;
        }
        break;
    case eCharacterActionType::die:
        wrap = false;
        coll = &texs.fDie;
        break;
    default:
        return nullptr;
    }
    return eCharacter::getTexture(coll, wrap, false);
}

eOverlay ePriest::getSecondaryTexture(const eTileSize size) const {
    if(mLoad != ePriestLoad::food) return eOverlay();
    const auto a = actionType();
    if(a == eCharacterActionType::none ||
       a == eCharacterActionType::die) return eOverlay();
    const int id = static_cast<int>(size);
    const auto& texs = eGameTextures::characters()[id];
    const int oi = static_cast<int>(rotatedOrientation());
    // a full food cart, placed as eCartTransporter places its cart
    const int ci = 1;
    double xx = 0;
    double yy = 0;
    switch(rotatedOrientation()) {
    case eOrientation::topRight: xx = 0.25; yy = -0.60; break;
    case eOrientation::right: xx = 0.75; yy = -0.25; break;
    case eOrientation::bottomRight: xx = 0.85; yy = 0.15; break;
    case eOrientation::bottom: xx = 0.75; yy = 0.75; break;
    case eOrientation::bottomLeft: xx = 0.15; yy = 0.85; break;
    case eOrientation::left: xx = -0.25; yy = 0.75; break;
    case eOrientation::topLeft: xx = -0.75; yy = 0.1; break;
    case eOrientation::top: xx = -0.25; yy = -0.25; break;
    }
    std::shared_ptr<eTexture> tex;
    switch(mFoodType) {
    case eResourceType::urchin:
        tex = texs.fUrchinCart[oi].getTexture(ci); break;
    case eResourceType::fish:
        tex = texs.fFishCart[oi].getTexture(ci); break;
    case eResourceType::meat:
        tex = texs.fMeatCart[oi].getTexture(ci); break;
    case eResourceType::cheese:
        tex = texs.fCheeseCart[oi].getTexture(ci); break;
    case eResourceType::carrots:
        tex = texs.fCarrotsCart[oi].getTexture(ci); break;
    case eResourceType::onions:
        tex = texs.fOnionsCart[oi].getTexture(ci); break;
    case eResourceType::wheat:
        tex = texs.fWheatCart[oi].getTexture(ci); break;
    case eResourceType::oranges:
        tex = texs.fOrangesCart[oi].getTexture(ci); break;
    default:
        tex = texs.fEmptyCart.getTexture(oi); break;
    }
    return {xx, yy, tex};
}

void ePriest::read(eReadStream& src) {
    eCharacter::read(src);
    src >> mLoad;
    src >> mFoodType;
}

void ePriest::write(eWriteStream& dst) const {
    eCharacter::write(dst);
    dst << mLoad;
    dst << mFoodType;
}

void ePriest::beingKilled() {
    if(mLoad != ePriestLoad::cattle) return;
    const auto a = dynamic_cast<ePriestAction*>(action());
    if(!a) return;
    if(const auto an = a->animal()) an->kill();
}
