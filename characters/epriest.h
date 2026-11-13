#ifndef EPRIEST_H
#define EPRIEST_H

#include "echaracter.h"

#include "engine/eresourcetype.h"

// what a sanctuary priest brings to the altar
enum class ePriestLoad {
    none,
    sheep,
    goat,
    food, // pushed in a cart
    cattle // walks behind him
};

// A sanctuary priest (figure 65 in the original game) who fetches a
// sheep, goat, bull or cow, or food from a store, for a sacrifice.
// See ePriestAction.
class ePriest : public eCharacter {
public:
    ePriest(eGameBoard& board);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    eOverlay getSecondaryTexture(const eTileSize size) const override;

    ePriestLoad load() const { return mLoad; }
    void setLoad(const ePriestLoad l) { mLoad = l; }

    eResourceType foodType() const { return mFoodType; }
    void setFoodType(const eResourceType t) { mFoodType = t; }

    // how long lifting the current animal onto his shoulders takes
    int pickUpTime() const;

    // a cow or bull he leads goes too (exe 0x004CB350, state 0x14)
    void beingKilled() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    ePriestLoad mLoad = ePriestLoad::none;
    eResourceType mFoodType = eResourceType::none;
};

#endif // EPRIEST_H
