#ifndef EPRIESTACTION_H
#define EPRIESTACTION_H

#include "ecomplexaction.h"

class eSanctuary;
class eBuilding;
class eStorageBuilding;

// A sanctuary priest's errand, after the original game (zeus.exe
// 0x004CA350 and the functions it calls; times are the original's figure
// ticks, about 13 eZeus ticks each):
// - a tick after he leaves he looks, in the order the animals came to
//   the city, for a sheep, goat or bull no other priest is after, then
//   for any cow or calf; failing that for the first granary, storehouse,
//   pier or trading post holding at least 4 loads of food; with none of
//   these he goes away
// - a sheep or goat: he walks up to it, lifts it onto his shoulders and
//   carries it back; a cow walks behind him; a bull first stands facing
//   him for 31 ticks
// - food: he takes 4 loads, from the food types in order, and pushes
//   them back in a cart
// - if the animal is gone, or the store no longer has the food, he looks
//   again after 11 ticks
// - at the altar he waits for his turn (eSanctuary::tryToSacrifice):
//   sheep and goats are offered as the sheep sacrifice, food as goods,
//   cattle as the bull sacrifice
enum class ePriestStage {
    start, search, toAnimal, pickUp, bull, toStore, back, atAltar
};

class ePriestAction : public eComplexAction {
public:
    ePriestAction(eCharacter* const c);

    void setSanctuary(eSanctuary* const s);
    eCharacter* animal() const { return mAnimal.get(); }

    bool decide() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    void search();
    void searchLater();
    eCharacter* findAnimal() const;
    eStorageBuilding* findStore() const;
    bool validAnimal(eCharacter* const a) const;
    void goToAnimal();
    void reachedAnimal();
    void goToStore();
    bool takeFood();
    void goBack();
    void vanish();
    void walkTo(eTile* const t);

    ePriestStage mStage = ePriestStage::start;
    stdptr<eSanctuary> mSanctuary;
    stdptr<eCharacter> mAnimal;
    stdptr<eBuilding> mStore;
    int mTries = 0;
    bool mMoveFailed = false; // not saved
};

#endif // EPRIESTACTION_H
