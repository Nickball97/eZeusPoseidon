#ifndef EAVENUE_H
#define EAVENUE_H

#include "ebuilding.h"

class eRoad;

class eAvenue : public eBuilding {
public:
    // type is avenue or boulevard: the original game's Boulevard (type 126)
    // is an Avenue with trees on both sides of the road; eZeus keeps both
    // as tree tiles beside a road
    eAvenue(eGameBoard& board, const eCityId cid,
            const eBuildingType type = eBuildingType::avenue);

    static bool sAvenueType(const eBuildingType t);

    int provide(const eProvide p, const int n);

    std::shared_ptr<eTexture>
        getTexture(const eTileSize size) const;

    eTile* road() const;

    // The SPR_BOUL_ROAD picture (0-48) zeus.exe gives a road or Avenue
    // tile from the roads and Avenues around it, in the default view
    // (table 0x005E92B8). Pass the building's seed for the variant.
    static int sRoadImage(eTile* const t, const eWorldDirection dir,
                          const int seed);
private:
    // the picture zeus.exe picks for this tree tile (0x004193A0):
    // 0-43 = tree picture, 100 + n = SPR_BOUL_ROAD picture n
    int pickArt(const eWorldDirection dir) const;

    mutable int mArtRevision = -1;
    mutable int mArtDir = -1;
    mutable int mArt = 0;
};

#endif // EAVENUE_H
