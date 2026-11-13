#ifndef EATLANTEANFIREMISSILE_H
#define EATLANTEANFIREMISSILE_H

#include "emissile.h"

// Atlantean Fire launched by Atlantean Towers with orichalc (Poseidon
// manual, Towers); drawn with the original PoseidonImps fireball.
class eAtlanteanFireMissile : public eMissile {
public:
    eAtlanteanFireMissile(eGameBoard& board,
                          const std::vector<ePathPoint>& path = {});

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
};

#endif // EATLANTEANFIREMISSILE_H
