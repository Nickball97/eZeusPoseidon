#include "efarmbase.h"

#include "textures/egametextures.h"
#include "enumbers.h"
#include "engine/egameboard.h"

#include <cmath>

#include <algorithm>

eFarmBase::eFarmBase(eGameBoard& board,
                     const eBuildingType type,
                     const int sw, const int sh,
                     const eResourceType resType,
                     const eCityId cid) :
    eResourceBuildingBase(board, type, sw, sh, 10, resType, cid),
    mTextures(eGameTextures::buildings())  {
    eGameTextures::loadPlantation();
    // room for a blessed harvest (4 x 8 loads)
    setMaxResource(4*eNumbers::sFarmHarvestLoads);
}

std::shared_ptr<eTexture> eFarmBase::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return mTextures[sizeId].fPlantation;
}

std::vector<eOverlay> eFarmBase::getOverlays(const eTileSize size) const {
    std::vector<eOverlay> os;
    const int sizeId = static_cast<int>(size);
    const auto& texs = mTextures[sizeId];
    const std::pair<int, int> xy[5] = {{-1, -1},
                                       {0, -1},
                                       {1, -1},
                                       {1, -2},
                                       {1, -3}};
    const double e = effectiveness();
    const int usedFields = std::clamp((int)std::round(1 + e*4), 0, 5);
    for(int i = 0; i < 5; i++) {
        eOverlay& o = os.emplace_back();
        const auto& xxyy = xy[i];
        o.fX = xxyy.first;
        o.fY = xxyy.second;
        o.fAlignTop = true;
        const int texId = i >= usedFields ? 0 : std::clamp(mRipe, 0, 5);
        const auto type = resourceType();
        switch(type) {
        case eResourceType::onions:
            o.fTex = texs.fOnions.getTexture(texId);
            break;
        case eResourceType::carrots:
            o.fTex = texs.fCarrots.getTexture(texId);
            break;
        case eResourceType::wheat:
            o.fTex = texs.fWheat.getTexture(texId);
            break;
        default: break;
        }
    }
    return os;
}

// One harvest a year. Zeus manual (Husbandry): carrots and onions are
// harvested in April, wheat in July; a farm started a year before harvest
// time with all its employees gives a full harvest, partial harvests come
// from missing workers or late planting. HeavenGames
// (gameinfo/industry-production): a farm makes 8 loads a year.
// mNextRipe holds the growth (in ticks of fully staffed work) since the
// last harvest; a full harvest needs one year of it.
void eFarmBase::timeChanged(const int by) {
    const double year = 365.*eNumbers::sDayLength;
    if(enabled()) {
        mNextRipe += by*effectiveness();
    }
    mNextRipe = std::min(mNextRipe, year);
    mRipe = std::clamp(int(5*mNextRipe/year), 0, 5);

    const auto& date = getBoard().date();
    const auto harvestMonth = this->harvestMonth();
    if(date.month() == harvestMonth && date.day() == 1) {
        const int loads = int(std::round(
                              eNumbers::sFarmHarvestLoads*mNextRipe/year));
        if(loads > 0) {
            // a blessed farm brings in 4 times this harvest (sBlessMultiplier)
            addProduced(resourceType(), useBlessing(loads));
            mNextRipe = 0;
            mRipe = 0;
        }
    }
    eResourceBuildingBase::timeChanged(by);
}

void eFarmBase::read(eReadStream& src) {
    eResourceBuildingBase::read(src);

    src >> mNextRipe;
    src >> mRipe;
}

void eFarmBase::write(eWriteStream& dst) const {
    eResourceBuildingBase::write(dst);

    dst << mNextRipe;
    dst << mRipe;
}

double eFarmBase::growth() const {
    const double year = 365.*eNumbers::sDayLength;
    return std::clamp(mNextRipe/year, 0., 1.);
}

eMonth eFarmBase::harvestMonth() const {
    // numbers.txt holds the month as 1 (January) to 12
    const int m = resourceType() == eResourceType::wheat ?
                      eNumbers::sFarmWheatHarvestMonth :
                      eNumbers::sFarmVegetableHarvestMonth;
    return static_cast<eMonth>(m - 1);
}
