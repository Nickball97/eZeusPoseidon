#include "eenemywarship.h"

#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "engine/eworldcity.h"
#include "actions/eenemywarshipaction.h"

eEnemyWarship::eEnemyWarship(eGameBoard& board) :
    eBoatBase(board, &eCharacterTextures::fEnemyBoat,
              eCharacterType::enemyWarship),
    eFightingCharacter(this) {
    eGameTextures::loadEnemyBoat();
    eGameTextures::loadTrireme();
}

eBoatBase::eCharTexs eEnemyWarship::charTexs() const {
    if(mGreek < 0) {
        const auto c = getBoard().world().cityWithId(cityId());
        mGreek = c && c->nationality() == eNationality::greek ? 1 : 0;
    }
    return mGreek == 1 ? &eCharacterTextures::fTrireme :
                         &eCharacterTextures::fEnemyBoat;
}

std::shared_ptr<eTexture>
eEnemyWarship::getTexture(const eTileSize size) const {
    const auto a = actionType();
    if(a == eCharacterActionType::fight ||
       a == eCharacterActionType::fight2) {
        // the original shows a fighting warship with its standing frame
        // (exe 0x004C2980); there are no fighting frames
        const int id = static_cast<int>(size);
        const auto& colls = eGameTextures::characters()[id];
        const auto& texs = colls.*(charTexs());
        const int oid = static_cast<int>(rotatedOrientation());
        if(oid < 0 || oid >= texs.fStand.size()) return nullptr;
        return texs.fStand.getTexture(oid);
    }
    return eBoatBase::getTexture(size);
}

eEnemyWarshipAction* eEnemyWarship::warshipAction() const {
    return dynamic_cast<eEnemyWarshipAction*>(action());
}

void eEnemyWarship::sSpawn(eGameBoard& board,
                           const eCityId invadingCid,
                           eTile* const tile,
                           eTile* const disembark,
                           const int index) {
    if(!tile) return;
    const auto w = e::make_shared<eEnemyWarship>(board);
    w->setCityId(invadingCid);
    w->setOnCityId(tile->cityId());
    w->changeTile(tile);
    w->setActionType(eCharacterActionType::stand);
    const auto a = e::make_shared<eEnemyWarshipAction>(w.get());
    a->setup(index, disembark);
    w->setAction(a);
}
