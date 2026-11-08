#include "eatlanteanfiremissile.h"

#include <cmath>

#include "textures/egametextures.h"
#include "engine/egameboard.h"

eAtlanteanFireMissile::eAtlanteanFireMissile(
        eGameBoard& board, const std::vector<ePathPoint>& path) :
    eMissile(board, eMissileType::atlanteanFire, path) {
    eGameTextures::loadAtlanteanFire();
}

std::shared_ptr<eTexture>
eAtlanteanFireMissile::getTexture(const eTileSize size) const {
    auto& board = this->board();
    const auto dir = board.direction();
    const int id = static_cast<int>(size);
    const auto& texs = eGameTextures::destrution()[id].fAtlanteanFire;
    if(texs.size() < 8) return nullptr;
    // same angle handling as arrows (32 directions), reduced to the
    // fireball's 8
    double da;
    if(dir == eWorldDirection::N) {
        da = 225;
    } else if(dir == eWorldDirection::E) {
        da = 315;
    } else if(dir == eWorldDirection::S) {
        da = 45;
    } else {
        da = 135;
    }
    const int iid = std::round((angle() - da)/11.25);
    const int i32 = iid < 0 ? ((32 + iid % 32) % 32) : (iid % 32);
    const int dir8 = ((i32 + 2)/4) % 8;
    const auto& coll = texs[dir8];
    const int n = coll.size();
    if(n == 0) return nullptr;
    return coll.getTexture(textureTime() % n);
}
