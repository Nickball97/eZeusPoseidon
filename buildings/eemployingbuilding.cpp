#include "eemployingbuilding.h"

#include "engine/egameboard.h"

#include <algorithm>
#include <cmath>

eEmployingBuilding::eEmployingBuilding(
        eGameBoard& board,
        const eBuildingType type,
        const int sw, const int sh,
        const int maxEmployees,
        const eCityId cid) :
    eBuildingWithResource(board, type, sw, sh, cid),
    mMaxEmployees(maxEmployees) {
    board.registerEmplBuilding(this);
}

eEmployingBuilding::~eEmployingBuilding() {
    auto& board = getBoard();
    board.unregisterEmplBuilding(this);
}

void eEmployingBuilding::setEmployed(const int e) {
    mEmployed = e;
    setEnabled(mEmployed > 0);
}

double eEmployingBuilding::vacanciesFilledFraction() const {
    if(mMaxEmployees <= 0) return 1;
    return double(mEmployed)/mMaxEmployees;
}

// How much a god's blessing speeds a building up. HeavenGames' god pages
// (zeus.heavengames.com/gameinfo/gods/): blessed production buildings
// "produce 4 times the normal amount"; Apollo's blessed culture buildings
// "produce twice as many walkers". Other buildings keep eZeus's 1.5.
double eEmployingBuilding::sBlessMultiplier(const eBuildingType type) {
    switch(type) {
    case eBuildingType::wheatFarm:
    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
        return 1.5;
    case eBuildingType::cardingShed:
    case eBuildingType::olivePress:
    case eBuildingType::winery:
    case eBuildingType::mint:
    case eBuildingType::armory:
    case eBuildingType::foundry:
    case eBuildingType::huntingLodge:
    case eBuildingType::fishery:
    case eBuildingType::urchinQuay:
        return 4.;
    case eBuildingType::podium:
    case eBuildingType::gymnasium:
    case eBuildingType::stadium:
    case eBuildingType::theater:
    case eBuildingType::bibliotheke:
    case eBuildingType::laboratory:
    case eBuildingType::observatory:
    case eBuildingType::museum:
        return 2.;
    default:
        return 1.5;
    }
}

double eEmployingBuilding::effectiveness() const {
    if(mShutDown) return 0.;
    const double ef = vacanciesFilledFraction();
    if(eBuilding::blessed()) {
        if(!blessingBoostsSpeed()) return ef;
        return ef*sBlessMultiplier(type());
    } else if(eBuilding::cursed()) {
        // Zeus manual (When Gods Attack): cursed buildings "become
        // inoperable until the curse is lifted"
        return 0.;
    }
    return ef;
}

void eEmployingBuilding::setShutDown(const bool sd) {
    mShutDown = sd;
    if(sd) setEmployed(0);
}

void eEmployingBuilding::read(eReadStream& src) {
    eBuildingWithResource::read(src);
    src >> mShutDown;
    src >> mMaxEmployees;
    src >> mEmployed;
    setEnabled(mEmployed > 0);
}

void eEmployingBuilding::write(eWriteStream& dst) const {
    eBuildingWithResource::write(dst);
    dst << mShutDown;
    dst << mMaxEmployees;
    dst << mEmployed;
}

int eEmployingBuilding::useBlessing(const int normal) {
    if(!blessed()) return normal;
    spendBlessing();
    switch(type()) {
    case eBuildingType::wheatFarm:
    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
        // "the farm produces 12 loads instead of 8" (Citadel of Poseidon)
        return std::max(normal, static_cast<int>(std::round(normal*1.5)));
    default:
        // "produces 4 loads once instead of only 1" (Citadel of Poseidon,
        // every other blessed production building; mint 400 drachmas
        // instead of 100)
        return normal + 3;
    }
}
