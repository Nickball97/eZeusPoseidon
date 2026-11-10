#include "etemplealtarbuilding.h"

#include "textures/egametextures.h"

#include "esanctuary.h"
#include "engine/egameboard.h"

eTempleAltarBuilding::eTempleAltarBuilding(eGameBoard& board,
                                           const eCityId cid) :
    eSanctBuilding({{0, 4, 0}}, board,
                   eBuildingType::templeAltar, 2, 2, cid) {
    setOverlayEnabledFunc([this]() {
        const auto s = monument();
        return s->finished();
    });
}

std::shared_ptr<eTexture>
eTempleAltarBuilding::getTexture(const eTileSize size) const {
    const int p = progress();
    if(p <= 0) return nullptr;
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings()[sizeId];
    return blds.fSanctuaryAltar;
}

std::vector<eOverlay> eTempleAltarBuilding::getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings()[sizeId];
    if(!sacrificing()) return {};
    const eTextureCollection* coll = nullptr;
    if(mSacrifice == eSacrifice::sheep) {
        eGameTextures::loadAltarSheepOverlay();
        coll = &blds.fAltarSheepOverlay;
    } else if(mSacrifice == eSacrifice::goods) {
        eGameTextures::loadAltarGoodsOverlay();
        coll = &blds.fAltarGoodsOverlay;
    } else if(mSacrifice == eSacrifice::bull) {
        eGameTextures::loadAltarBullOverlay();
        coll = &blds.fAltarBullOverlay;
    } else {
        return {};
    }
    std::vector<eOverlay> result;
    const int frame = textureTime();
    const auto& tex = coll->getTexture(frame % coll->size());
    result.emplace_back(eOverlay{-1.7, -3.4, tex, false});
    return result;
}

void eTempleAltarBuilding::read(eReadStream& src) {
    eSanctBuilding::read(src);
    src >> mSacrifice;
    src >> mSacrificeTime;
}

void eTempleAltarBuilding::write(eWriteStream& dst) const {
    eSanctBuilding::write(dst);
    dst << mSacrifice;
    dst << mSacrificeTime;
}

bool eTempleAltarBuilding::sacrificing() const {
    if(mSacrifice == eSacrifice::none) return false;
    const auto s = dynamic_cast<eSanctuary*>(monument());
    return s && s->sacrificeShown();
}
