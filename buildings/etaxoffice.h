#ifndef ETAXOFFICE_H
#define ETAXOFFICE_H

#include "epatrolbuilding.h"

class eTaxOffice : public ePatrolBuilding {
public:
    eTaxOffice(eGameBoard& board, const eCityId cid);

    // Hades' blessing: the next collector's houses pay 4 times their tax
    // once (original exe), not faster collectors
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }
};

#endif // ETAXOFFICE_H
