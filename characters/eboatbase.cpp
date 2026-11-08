#include "eboatbase.h"

#include "textures/egametextures.h"

eBoatBase::eBoatBase(
        eGameBoard& board, const eCharTexs charTexs,
        const eCharacterType type) :
    eCharacter(board, type),
    mCharTexs(charTexs) {}

std::shared_ptr<eTexture> eBoatBase::getTexture(const eTileSize size) const {
    const int id = static_cast<int>(size);
    const auto& texs = eGameTextures::characters();
    const auto& colls = texs[id];
    const auto& charTexs = colls.*(this->charTexs());
    const eTextureCollection* coll = nullptr;
    const int oid = static_cast<int>(rotatedOrientation());
    bool wrap = true;
    const auto a = actionType();
    switch(a) {
    case eCharacterActionType::stand:
        return charTexs.fStand.getTexture(oid);
    case eCharacterActionType::carry:
    case eCharacterActionType::walk:
        coll = &charTexs.fSwim[oid];
        break;
    case eCharacterActionType::die:
        wrap = false;
        coll = &charTexs.fDie[oid];
        break;
    case eCharacterActionType::fight:
    case eCharacterActionType::fight2:
        if(charTexs.fFight.empty()) return nullptr;
        coll = &charTexs.fFight[oid];
        break;
    default:
        return nullptr;
    }

    return eCharacter::getTexture(coll, wrap, false);
}
