#ifndef EENEMYWARSHIP_H
#define EENEMYWARSHIP_H

#include "eboatbase.h"
#include "efightingcharacter.h"

class eTile;
class eEnemyWarshipAction;

// An invader's warship ("Enemy Warship", figure 64 of the original game).
// Sea invasions in the original's campaigns can bring a number of them
// with the transports. They fight the player's triremes, shoot at the
// player's soldiers in range and at trade and fishing boats, never at
// buildings, and sail away after about four months (zeus.exe 0x004C2980,
// see mod-docs/original-exe-findings.md). Stats per nation come from the
// Model "<Nation> Warship" rows.
//
// Looks (Citadel of Poseidon, enemy appearance): a Greek warship is a
// trireme; every other nation's is the sailed ship eZeus already uses for
// the enemy transport.
class eEnemyWarship : public eBoatBase,
                      public eFightingCharacter {
public:
    eEnemyWarship(eGameBoard& board);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;

    eEnemyWarshipAction* warshipAction() const;

    // index: its place among the invasion's warships (they set off one
    // after another); disembark: the invasion's landing point, one of
    // the places it patrols
    static void sSpawn(eGameBoard& board,
                       const eCityId invadingCid,
                       eTile* const tile,
                       eTile* const disembark,
                       const int index);
protected:
    eCharTexs charTexs() const override;
private:
    mutable int mGreek = -1; // -1 not checked yet
};

#endif // EENEMYWARSHIP_H
