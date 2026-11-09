#include "ewinery.h"

#include "textures/egametextures.h"
#include "enumbers.h"

eWinery::eWinery(eGameBoard& board, const eCityId cid) :
    eProcessingBuilding(board,
                        &eBuildingTextures::fWinery,
                        -2.2, -3.25,
                        &eBuildingTextures::fWineryOverlay,
                        eBuildingType::winery, 2, 2, 12,
                        eResourceType::grapes,
                        eResourceType::wine, 1,
                        eNumbers::sWineryProcessingPeriod,
                        cid) {
    eGameTextures::loadWinery();
    // accepts at most 3 loads of raw material (original exe 0x0048A880)
    setMaxRaw(3);
}
