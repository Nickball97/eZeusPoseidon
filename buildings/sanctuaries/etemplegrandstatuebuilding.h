#ifndef ETEMPLEGRANDSTATUEBUILDING_H
#define ETEMPLEGRANDSTATUEBUILDING_H

#include "esanctbuilding.h"

// The Citadel of Poseidon's centrepiece: "a gorgeous statue of the god in
// his element, surrounded by leaping dolphins and beautiful Nereids"
// (Poseidon manual). 6x6, built in the three stages of the original art
// (Poseidon_GrandStatue1-3).
class eTempleGrandStatueBuilding : public eSanctBuilding {
public:
    eTempleGrandStatueBuilding(eGameBoard& board, const eCityId cid);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
};

#endif // ETEMPLEGRANDSTATUEBUILDING_H
