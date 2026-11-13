#include "etempleobeliskbuilding.h"

#include "textures/egametextures.h"

// Same cost as the floor tile each piece replaces, so the Citadel's
// materials still match HeavenGames.
eTempleObeliskBuilding::eTempleObeliskBuilding(
        const int id, eGameBoard& board, const eCityId cid) :
    eSanctBuilding({{0, 1, 0}}, board, eBuildingType::templeObelisk,
                   1, 1, cid),
    mId(id) {
    eGameTextures::loadPyramid();
}

// Atlantean pyramid sheet (Poseidon_Pyramid.bmp, images 620-664 = entries
// 0-44): 8 white obelisk, 35 white block two levels tall, 38 flat white
// pyramid tile.
std::shared_ptr<eTexture>
eTempleObeliskBuilding::getTexture(const eTileSize size) const {
    if(!finished()) return nullptr;
    const int sizeId = static_cast<int>(size);
    const auto& coll = eGameTextures::buildings()[sizeId].fPyramid;
    if(coll.size() < 45) return nullptr;
    return coll.getTexture(mId == 1 ? 35 : 38);
}

std::vector<eOverlay>
eTempleObeliskBuilding::getOverlays(const eTileSize size) const {
    if(mId != 1 || !finished()) return {};
    const int sizeId = static_cast<int>(size);
    const auto& coll = eGameTextures::buildings()[sizeId].fPyramid;
    if(coll.size() < 45) return {};
    // the obelisk stands on the block, two levels (one tile height) up
    eOverlay o;
    o.fX = -1;
    o.fY = -1;
    o.fTex = coll.getTexture(8);
    o.fAlignTop = true;
    return {o};
}
