#include "etemplegrandstatuebuilding.h"

#include "textures/egametextures.h"

// HeavenGames (Sanctuaries): the Citadel needs 231 marble, 52 wood,
// 2 sculptures and 16 orichalc. Its other pieces use eZeus's usual piece
// costs (173 tiles, 9 obelisk-corner pieces, 4 temple halves, altar,
// 2 statues: 216 marble,
// 20 wood, 2 sculptures), so the grand statue takes the rest:
// 15 marble, 32 wood and 16 orichalc over its three stages.
eTempleGrandStatueBuilding::eTempleGrandStatueBuilding(
        eGameBoard& board, const eCityId cid) :
    eSanctBuilding({{11, 5, 0, 6}, {11, 5, 0, 5}, {10, 5, 0, 5}}, board,
                   eBuildingType::templeGrandStatue, 6, 6, cid) {
    eGameTextures::loadPoseidonGrandStatue();
}

std::shared_ptr<eTexture>
eTempleGrandStatueBuilding::getTexture(const eTileSize size) const {
    const int p = progress();
    if(p <= 0) return nullptr;
    const int sizeId = static_cast<int>(size);
    const auto& coll = eGameTextures::buildings()[sizeId].fPoseidonGrandStatue;
    const int n = coll.size();
    if(n == 0) return nullptr;
    return coll.getTexture(std::min(p, n) - 1);
}

std::vector<eOverlay>
eTempleGrandStatueBuilding::getOverlays(const eTileSize size) const {
    if(!finished()) return {};
    const int sizeId = static_cast<int>(size);
    const auto& coll = eGameTextures::buildings()[sizeId].fPoseidonGrandStatueWater;
    const int n = coll.size();
    if(n == 0) return {};
    eOverlay o;
    o.fX = 0;
    o.fY = 0;
    o.fTex = coll.getTexture(textureTime() % n);
    o.fAlignTop = true;
    return {o};
}
