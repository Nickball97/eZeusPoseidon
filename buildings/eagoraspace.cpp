#include "eagoraspace.h"

#include "eagorabase.h"

#include "textures/egametextures.h"

eAgoraSpace::eAgoraSpace(const stdsptr<eAgoraBase>& agora,
                         eGameBoard& board, const eCityId cid) :
    eBuilding(board, eBuildingType::agoraSpace, 2, 2, cid),
    mAgora(agora) {}

stdsptr<eTexture> eAgoraSpace::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings();
    const auto& coll = texs[sizeId].fAgora;
    return coll.getTexture(seed() % coll.size());
}

bool eAgoraSpace::isOnFire() {
    if(mAgora) return mAgora->isOnFire();
    return eBuilding::isOnFire();
}

void eAgoraSpace::setOnFire(const bool f) {
    if(mAgora) mAgora->setOnFire(f);
    else eBuilding::setOnFire(f);
}

int eAgoraSpace::provide(const eProvide p, const int n) {
    if(p == eProvide::maintanance && mAgora) return mAgora->provide(p, n);
    return eBuilding::provide(p, n);
}
