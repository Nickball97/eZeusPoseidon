#include "eregrowforestaction.h"

#include "engine/etile.h"
#include "engine/egameboard.h"

// A cut tree grows back in about 3 months (Citadel of Poseidon).

eRegrowForestAction::eRegrowForestAction(eTile* const tile) :
    ePlannedAction(false, (365*350)/4, ePlannedActionType::regrowForest),
    mTile(tile) {}

eRegrowForestAction::eRegrowForestAction() :
    eRegrowForestAction(nullptr) {}

void eRegrowForestAction::trigger(eGameBoard& board) {
    (void)board;
    if(mTile->terrain() != eTerrain::choppedForest) return;
    mTile->setTerrain(eTerrain::forest);
}

void eRegrowForestAction::read(eReadStream& src, eGameBoard& board) {
    ePlannedAction::read(src, board);
    mTile = src.readTile(board);
}

void eRegrowForestAction::write(eWriteStream& dst) const {
    ePlannedAction::write(dst);
    dst.writeTile(mTile);
}
