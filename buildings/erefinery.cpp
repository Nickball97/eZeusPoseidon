#include "erefinery.h"

#include "characters/eorichalcminer.h"
#include "textures/egametextures.h"

eRefinery::eRefinery(eGameBoard& board, const eCityId cid) :
    eResourceCollectBuilding(board,
                             &eBuildingTextures::fRefinery,
                             -5.47, -5.50,
                             &eBuildingTextures::fRefineryOverlay,
                             2, 1.0, -2.0,
                             [this]() { return e::make_shared<eOrichalcMiner>(getBoard()); },
                             eBuildingType::refinery,
                             eHasResourceObject::sCreate(eHasResourceObjectType::orichalc),
                             2, 2, 16, eResourceType::orichalc, cid) {
    eGameTextures::loadRefinery();
    // holds at most 5 loads before it stops (original exe 0x0048A880)
    setMaxResource(5);
    // 3 workers, 25 units each, 100 units = 1 load (Citadel of Poseidon)
    setMaxCollectors(3);
    setRawPerLoad(4);
    setRawCountCollect(16);
    setWaitTime(1000);
}
