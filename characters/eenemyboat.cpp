#include "eenemyboat.h"

#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "engine/eworldcity.h"

eEnemyBoat::eEnemyBoat(eGameBoard& board) :
    eBoatBase(board, &eCharacterTextures::fEnemyBoat,
              eCharacterType::enemyBoat) {
    eGameTextures::loadEnemyBoat();
}

bool eEnemyBoat::hiddenUnderwater() const {
    if(mOceanid < 0) {
        // the transport belongs to the invading city (eInvasionHandler)
        const auto c = getBoard().world().cityWithId(cityId());
        mOceanid = c && c->nationality() == eNationality::oceanid ? 1 : 0;
    }
    return mOceanid == 1;
}
