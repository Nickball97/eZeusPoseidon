#include "ebuildingregistry.h"

#include "buildings/allbuildings.h"
#include "engine/egameboard.h"

namespace {

template <class T>
stdsptr<eBuilding> make(eGameBoard& board, const eCityId cid) {
    return e::make_shared<T>(board, cid);
}

stdsptr<eBuilding> makeOliveTree(eGameBoard& board, const eCityId cid) {
    const auto type = eResourceBuildingType::oliveTree;
    return e::make_shared<eResourceBuilding>(board, type, cid);
}

stdsptr<eBuilding> makeVine(eGameBoard& board, const eCityId cid) {
    const auto type = eResourceBuildingType::vine;
    return e::make_shared<eResourceBuilding>(board, type, cid);
}

stdsptr<eBuilding> makeOrangeTree(eGameBoard& board, const eCityId cid) {
    const auto type = eResourceBuildingType::orangeTree;
    return e::make_shared<eResourceBuilding>(board, type, cid);
}

stdsptr<eBuilding> makeGrowersLodge(eGameBoard& board, const eCityId cid) {
    const auto type = eGrowerType::grapesAndOlives;
    return e::make_shared<eGrowersLodge>(board, type, cid);
}

stdsptr<eBuilding> makeOrangeTendersLodge(eGameBoard& board,
                                          const eCityId cid) {
    const auto type = eGrowerType::oranges;
    return e::make_shared<eGrowersLodge>(board, type, cid);
}

using eBT = eBuildingType;

// type, width, height, how it is made, plan routes
const std::vector<eBuildingKind> gKinds = {
    {eBT::road, 1, 1, &make<eRoad>, false},
    {eBT::commonHouse, 2, 2, &make<eSmallHouse>, false},
    {eBT::gymnasium, 3, 3, &make<eGymnasium>, true},
    {eBT::podium, 2, 2, &make<ePodium>, true},
    {eBT::fountain, 2, 2, &make<eFountain>, true},
    {eBT::watchPost, 2, 2, &make<eWatchpost>, true},
    {eBT::maintenanceOffice, 2, 2, &make<eMaintenanceOffice>, true},
    {eBT::college, 3, 3, &make<eCollege>, false},
    {eBT::dramaSchool, 3, 3, &make<eDramaSchool>, false},
    {eBT::theater, 5, 5, &make<eTheater>, true},
    {eBT::hospital, 4, 4, &make<eHospital>, true},
    {eBT::bibliotheke, 2, 2, &make<eBibliotheke>, false},
    {eBT::observatory, 5, 5, &make<eObservatory>, false},
    {eBT::university, 3, 3, &make<eUniversity>, false},
    {eBT::laboratory, 4, 4, &make<eLaboratory>, false},
    {eBT::inventorsWorkshop, 3, 3, &make<eInventorsWorkshop>, false},
    {eBT::museum, 6, 6, &make<eMuseum>, false},
    {eBT::eliteHousing, 4, 4, &make<eEliteHousing>, false},
    {eBT::taxOffice, 2, 2, &make<eTaxOffice>, true},
    {eBT::mint, 2, 2, &make<eMint>, false},
    {eBT::foundry, 2, 2, &make<eFoundry>, false},
    {eBT::refinery, 2, 2, &make<eRefinery>, false},
    {eBT::blackMarbleWorkshop, 2, 2, &make<eBlackMarbleWorkshop>, false},
    {eBT::timberMill, 2, 2, &make<eTimberMill>, false},
    {eBT::masonryShop, 2, 2, &make<eMasonryShop>, false},
    {eBT::huntingLodge, 2, 2, &make<eHuntingLodge>, false},
    {eBT::corral, 4, 4, &make<eCorral>, false},
    {eBT::dairy, 2, 2, &make<eDairy>, false},
    {eBT::cardingShed, 2, 2, &make<eCardingShed>, false},
    {eBT::wheatFarm, 3, 3, &make<eWheatFarm>, false},
    {eBT::onionsFarm, 3, 3, &make<eOnionFarm>, false},
    {eBT::carrotsFarm, 3, 3, &make<eCarrotFarm>, false},
    {eBT::granary, 4, 4, &make<eGranary>, false},
    {eBT::warehouse, 3, 3, &make<eWarehouse>, false},
    {eBT::wall, 1, 1, &make<eWall>, false},
    {eBT::tower, 2, 2, &make<eTower>, false},
    {eBT::armory, 2, 2, &make<eArmory>, false},
    {eBT::olivePress, 2, 2, &make<eOlivePress>, false},
    {eBT::winery, 2, 2, &make<eWinery>, false},
    {eBT::sculptureStudio, 2, 2, &make<eSculptureStudio>, false},
    {eBT::artisansGuild, 2, 2, &make<eArtisansGuild>, false},
    {eBT::chariotFactory, 4, 4, &make<eChariotFactory>, false},
    {eBT::park, 1, 1, &make<ePark>, false},
    {eBT::doricColumn, 1, 1, &make<eDoricColumn>, false},
    {eBT::ionicColumn, 1, 1, &make<eIonicColumn>, false},
    {eBT::corinthianColumn, 1, 1, &make<eCorinthianColumn>, false},
    {eBT::avenue, 1, 1, &make<eAvenue>, false},
    {eBT::bench, 1, 1, &make<eBench>, false},
    {eBT::flowerGarden, 2, 2, &make<eFlowerGarden>, false},
    {eBT::gazebo, 2, 2, &make<eGazebo>, false},
    {eBT::hedgeMaze, 3, 3, &make<eHedgeMaze>, false},
    {eBT::fishPond, 4, 4, &make<eFishPond>, false},
    {eBT::birdBath, 1, 1, &make<eBirdBath>, false},
    {eBT::shortObelisk, 1, 1, &make<eShortObelisk>, false},
    {eBT::tallObelisk, 1, 1, &make<eTallObelisk>, false},
    {eBT::shellGarden, 2, 2, &make<eShellGarden>, false},
    {eBT::sundial, 2, 2, &make<eSundial>, false},
    {eBT::dolphinSculpture, 3, 3, &make<eDolphinSculpture>, false},
    {eBT::spring, 3, 3, &make<eSpring>, false},
    {eBT::orrery, 3, 3, &make<eOrrery>, false},
    {eBT::topiary, 3, 3, &make<eTopiary>, false},
    {eBT::baths, 4, 4, &make<eBaths>, false},
    {eBT::stoneCircle, 4, 4, &make<eStoneCircle>, false},
    {eBT::ruins, 1, 1, &make<eRuins>, false},
    {eBT::placeholder, 1, 1, &make<ePlaceholder>, false},
    {eBT::hippodromePiece, 4, 4, &make<eHippodromePiece>, false},
    {eBT::oliveTree, 1, 1, &makeOliveTree, false},
    {eBT::vine, 1, 1, &makeVine, false},
    {eBT::orangeTree, 1, 1, &makeOrangeTree, false},
    {eBT::growersLodge, 2, 2, &makeGrowersLodge, false},
    {eBT::orangeTendersLodge, 2, 2, &makeOrangeTendersLodge, false},
};

}

const eBuildingKind* eBuildingRegistry::sFind(const eBuildingType type) {
    for(const auto& k : gKinds) {
        if(k.fType == type) return &k;
    }
    return nullptr;
}

stdsptr<eBuilding> eBuildingRegistry::sCreate(const eBuildingType type,
                                              eGameBoard& board,
                                              const eCityId cid) {
    const auto kind = sFind(type);
    if(!kind) return nullptr;
    return kind->fCreate(board, cid);
}

const std::vector<eBuildingKind>& eBuildingRegistry::sAll() {
    return gKinds;
}
