#include "emint.h"

#include "characters/esilverminer.h"
#include "textures/egametextures.h"

eMint::eMint(eGameBoard& board, const eCityId cid) :
    eResourceCollectBuilding(board,
                             &eBuildingTextures::fMint,
                             -3.73, -3.73,
                             &eBuildingTextures::fMintOverlay,
                             3, 0.5, -1.5,
                             [this]() { return e::make_shared<eSilverMiner>(getBoard()); },
                             eBuildingType::mint,
                             eHasResourceObject::sCreate(eHasResourceObjectType::silver),
                             2, 2, 15, eResourceType::silver, cid) {
    eGameTextures::loadMint();
    // holds at most 3 loads before it stops (original exe 0x0048A880)
    setMaxResource(3);
    // Mints work like foundries: 3 miners, 25 units each, 100 units = 1
    // load; 200 silver makes 100 drachmas, up to 600-700 a year
    // (Citadel of Poseidon)
    setMaxCollectors(3);
    setRawPerLoad(4);
    setRawCountCollect(16);
    setWaitTime(1000);
}
