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
};

#endif // EAVENUE_H
