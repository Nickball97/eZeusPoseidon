#ifndef EENEMYBOAT_H
#define EENEMYBOAT_H

#include "eboatbase.h"

class eEnemyBoat : public eBoatBase {
public:
    eEnemyBoat(eGameBoard& board);

    // Poseidon manual (Atlanteans in the World): "Be careful if the
    // Oceanids attack you. They swim under water to get to your city, and
    // your frigates are powerless to stop them." An Oceanid sea invasion
    // travels like any other but its transport is hidden under water.
    bool hiddenUnderwater() const override;
    // What shows of them while hidden: an Oceanid figure drawn dark and
    // see-through, like a shadow under the surface (as Nicolás remembers
    // the original; no source describes it)
    std::shared_ptr<eTexture> shadowTexture(const eTileSize size) const;
private:
    mutable int mOceanid = -1; // -1 not checked yet
};

#endif // EENEMYBOAT_H
