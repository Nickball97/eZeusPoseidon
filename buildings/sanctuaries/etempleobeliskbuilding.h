#ifndef ETEMPLEOBELISKBUILDING_H
#define ETEMPLEOBELISKBUILDING_H

#include "esanctbuilding.h"

// The Citadel of Poseidon's north-west corner (Model\Sanctuary_POSEIDON_
// GRAND.txt types 117/122, checked against the original game): eight flat
// pyramid floor tiles around a two-level stone block topped by an obelisk.
// Drawn with the Atlantean pyramid images eZeus already has.
class eTempleObeliskBuilding : public eSanctBuilding {
public:
    // id 0: pyramid floor tile, id 1: block with obelisk
    eTempleObeliskBuilding(const int id, eGameBoard& board,
                           const eCityId cid);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    int id() const { return mId; }
private:
    const int mId;
};

#endif // ETEMPLEOBELISKBUILDING_H
