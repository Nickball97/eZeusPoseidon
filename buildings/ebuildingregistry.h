#ifndef EBUILDINGREGISTRY_H
#define EBUILDINGREGISTRY_H

#include <vector>

#include "ebuilding.h"

// The table of building kinds.
//
// One line per kind of building that is made the plain way: from the map
// and the city it belongs to, on a fixed footprint. The save reader, the
// save writer and the computer player's builder all work from this table,
// so a new plain building needs one line here and nothing in those three.
// (Buildings that need more to exist - an orientation, a god, a partner
// building - keep their own cases there.)
//
// To add a building: write its class, give it a value in eBuildingType,
// add its line to the table in ebuildingregistry.cpp. See
// mod-docs/adding-a-building.md for the rest (name, cost, build menu).
struct eBuildingKind {
    eBuildingType fType;
    // footprint in tiles
    int fW;
    int fH;
    // makes one for a city
    stdsptr<eBuilding> (*fCreate)(eGameBoard& board, const eCityId cid);
    // its walkers keep to the route a computer city's plan gives them
    bool fPlanRoutes;
};

namespace eBuildingRegistry {
    // nullptr for a kind that is not in the table
    const eBuildingKind* sFind(const eBuildingType type);
    // nullptr for a kind that is not in the table
    stdsptr<eBuilding> sCreate(const eBuildingType type,
                               eGameBoard& board, const eCityId cid);
    const std::vector<eBuildingKind>& sAll();
}

#endif // EBUILDINGREGISTRY_H
