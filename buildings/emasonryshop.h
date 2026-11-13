#ifndef EMASONRYSHOP_H
#define EMASONRYSHOP_H

#include "eresourcecollectbuilding.h"

// What the masonry shop and the black marble workshop share: everything
// but the stone they cut and their pictures.
class eMasonryShopBase : public eResourceCollectBuilding {
public:
    using eStones = eTextureCollection eBuildingTextures::*;
    eMasonryShopBase(eGameBoard& board,
                     const eBaseTex baseTex,
                     const eStones stones,
                     const eBuildingType type,
                     const eHasResourceObjectType hasResource,
                     const eResourceType resType,
                     const eTileActionType collectedAction,
                     const eCityId cid);

    std::vector<eOverlay> getOverlays(const eTileSize size) const;

    void timeChanged(const int by);
private:
    const eStones mStones;
    const eTileActionType mCollectedAction;
    bool mCollectActionSet = false;
};

class eMasonryShop : public eMasonryShopBase {
public:
    eMasonryShop(eGameBoard& board, const eCityId cid);
};

#endif // EMASONRYSHOP_H
