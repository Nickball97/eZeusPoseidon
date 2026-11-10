#include "eheatgetters.h"

#include "characters/gods/egodtype.h"
#include "characters/monsters/emonstertarget.h"

eHeat eHeatGetters::appeal(const eBuildingType type) {
    switch(type) {
    case eBuildingType::wheatFarm: return {-3, 3, 1, 1};
    // Original model data: onion farm DES -4 RGE 3, carrot farm -2 / 2.
    case eBuildingType::carrotsFarm: return {-2, 2, 1, 1};
    case eBuildingType::onionsFarm: return {-4, 3, 2, 1};
    case eBuildingType::growersLodge: return {-3, 2, 1, 1};
    case eBuildingType::oliveTree:
    case eBuildingType::vine: return {1, 1, 1, -1};
    case eBuildingType::orangeTree: return {1, 2, 2, -1};
    case eBuildingType::cardingShed: return {-5, 3, 1, 2};
    case eBuildingType::dairy: return {-5, 3, 1, 2};
    case eBuildingType::urchinQuay:
    case eBuildingType::fishery: return {-15, 4, 1, 2};
    case eBuildingType::huntingLodge: return {-6, 4, 2, -2};
    case eBuildingType::corral: return {-12, 5, 2, 1};
    case eBuildingType::orangeTendersLodge: return {2, 2, 1, -1};

    case eBuildingType::timberMill: return {-5, 4, 1, 1};
    case eBuildingType::masonryShop: return {-6, 4, 1, 1};
    case eBuildingType::blackMarbleWorkshop: return {-6, 4, 1, 1};
    case eBuildingType::mint: return {-10, 6, 2, 2};
    case eBuildingType::foundry: return {-10, 6, 1, 1};
    case eBuildingType::refinery: return {-10, 5, 1, 1};
    case eBuildingType::sculptureStudio: return {-2, 2, 1, 1};
    case eBuildingType::olivePress: return {-4, 4, 2, 2};
    case eBuildingType::winery: return {4, 4, 1, -1};
    case eBuildingType::armory: return {-8, 4, 1, 1};
    case eBuildingType::tower: return {-4, 4, 4, 1};

    case eBuildingType::granary: return {-12, 4, 1, 2};
    case eBuildingType::warehouse: return {-2, 4, 4, 1};
    // Trading Post and Dock: DES -2 RGE 4 in the original model data. A
    // dock's storage part is also a tradePost in eZeus, so this covers both.
    case eBuildingType::tradePost: return {-2, 4, 4, 1};
    case eBuildingType::foodVendor:
    case eBuildingType::fleeceVendor:
    case eBuildingType::oilVendor:
    case eBuildingType::armsVendor:
    case eBuildingType::wineVendor: return {0, 0}; // vendors: DES 0 in the Model data
    case eBuildingType::horseTrainer:
    case eBuildingType::chariotVendor: return {0, 0};
    // Agora DES 12, every 2 tiles -2, range 6 (Model data and the
    // Citadel of Poseidon appeal table).
    case eBuildingType::commonAgora:
    case eBuildingType::grandAgora: return {12, 6, 2, -2};
    // Race track: DES -6, +1 per tile, range 4.
    case eBuildingType::hippodromePiece: return {-6, 4, 1, 1};
    // Elite house: value set per level in eHeatMapTask; range 4,
    // -1 every 2 tiles.
    case eBuildingType::eliteHousing: return {0, 4, 2, -1};

    case eBuildingType::horseRanch: return {-10, 4, 1, 1};
    // The pen is its own Model row: DES -6, range 4.
    case eBuildingType::horseRanchEnclosure: return {-6, 4, 1, 1};

    case eBuildingType::fountain: return {4, 4, 2, -2};
    case eBuildingType::hospital: return {-2, 4, 4, 0};
    case eBuildingType::maintenanceOffice: return {0, 0};
    case eBuildingType::watchPost: return {-2, 2, 1, 1};
    case eBuildingType::palace: return {18, 6, 3, -3};
    case eBuildingType::taxOffice: return {-4, 2, 1, 1};

    case eBuildingType::college: return {-5, 2, 1, -3};
    case eBuildingType::podium: return {3, 3, 1, -1};
    case eBuildingType::gymnasium: return {0, 0};
    case eBuildingType::dramaSchool: return {0, 0};
    case eBuildingType::theater: return {6, 3, 1, -1};
    case eBuildingType::stadium: return {0, 0};

    case eBuildingType::bibliotheke: return {3, 3, 1, -1};
    case eBuildingType::university: return {-5, 3, 1, -3};
    case eBuildingType::observatory: return {7, 4, 1, -1};
    case eBuildingType::museum: return {12, 4, 2, -3};

    case eBuildingType::doricColumn:
    case eBuildingType::ionicColumn:
    case eBuildingType::corinthianColumn: return {4, 3, 1, -2};

    case eBuildingType::avenue: return {3, 3, 1, -1};
    // Model BUILD_BOULEVARD: appeal 3, falling by 2 every 2 tiles, range 4
    case eBuildingType::boulevard: return {3, 4, 2, -2};

    case eBuildingType::commemorative: return {30, 6, 2, -5};
    case eBuildingType::godMonument: return {35, 7};

    case eBuildingType::park: return {3, 3, 1, -1};

    case eBuildingType::bench: return {2, 4, 2, -1};
    case eBuildingType::flowerGarden: return {8, 3, 1, -1};
    case eBuildingType::gazebo: return {6, 6, 1, -1};
    case eBuildingType::hedgeMaze: return {12, 4, 1, -1};
    case eBuildingType::fishPond: return {18, 6, 1, -3};

    case eBuildingType::waterPark: return {5, 4, 1, -1};

    case eBuildingType::birdBath: return {3, 3, 1, -1};
    case eBuildingType::shortObelisk: return {6, 2, 1, -1};
    case eBuildingType::tallObelisk: return {8, 2, 1, 0};
    case eBuildingType::shellGarden: return {5, 5, 1, 0};
    case eBuildingType::sundial: return {7, 4, 1, -2};
    case eBuildingType::dolphinSculpture: return {12, 4, 2, -3};
    case eBuildingType::orrery: return {9, 6, 1, -1};
    case eBuildingType::spring: return {10, 5, 2, -1};
    case eBuildingType::topiary: return {15, 3, 1, 0};
    case eBuildingType::baths: return {15, 6, 1, -1};
    case eBuildingType::stoneCircle: return {24, 4, 2, -2};

    case eBuildingType::templeAphrodite:
    case eBuildingType::templeApollo:
    case eBuildingType::templeAres:
    case eBuildingType::templeArtemis:
    case eBuildingType::templeAthena:
    case eBuildingType::templeAtlas:
    case eBuildingType::templeDemeter:
    case eBuildingType::templeDionysus:
    case eBuildingType::templeHades:
    case eBuildingType::templeHephaestus:
    case eBuildingType::templeHera:
    case eBuildingType::templeHermes:
    case eBuildingType::templePoseidon:
    case eBuildingType::templeZeus: return {20, 6, 1, -2};

    case eBuildingType::modestPyramid: return {15, 4, 1, -2};
    case eBuildingType::pyramid: return {20, 4, 1, -2};
    case eBuildingType::greatPyramid: return {25, 5, 1, -2};
    case eBuildingType::majesticPyramid: return {30, 6, 1, -2};

    case eBuildingType::smallMonumentToTheSky: return {16, 4, 1, -2};
    case eBuildingType::monumentToTheSky: return {20, 5, 1, -2};
    case eBuildingType::grandMonumentToTheSky: return {25, 6, 1, -2};

    case eBuildingType::minorShrineAphrodite: // 3x3
    case eBuildingType::minorShrineApollo:
    case eBuildingType::minorShrineAres:
    case eBuildingType::minorShrineArtemis:
    case eBuildingType::minorShrineAthena:
    case eBuildingType::minorShrineAtlas:
    case eBuildingType::minorShrineDemeter:
    case eBuildingType::minorShrineDionysus:
    case eBuildingType::minorShrineHades:
    case eBuildingType::minorShrineHephaestus:
    case eBuildingType::minorShrineHera:
    case eBuildingType::minorShrineHermes:
    case eBuildingType::minorShrinePoseidon:
    case eBuildingType::minorShrineZeus: return {20, 4, 2, -2};

    case eBuildingType::shrineAphrodite: // 6x6
    case eBuildingType::shrineApollo:
    case eBuildingType::shrineAres:
    case eBuildingType::shrineArtemis:
    case eBuildingType::shrineAthena:
    case eBuildingType::shrineAtlas:
    case eBuildingType::shrineDemeter:
    case eBuildingType::shrineDionysus:
    case eBuildingType::shrineHades:
    case eBuildingType::shrineHephaestus:
    case eBuildingType::shrineHera:
    case eBuildingType::shrineHermes:
    case eBuildingType::shrinePoseidon:
    case eBuildingType::shrineZeus: return {25, 5, 2, -2};

    case eBuildingType::majorShrineAphrodite: // 8x8
    case eBuildingType::majorShrineApollo:
    case eBuildingType::majorShrineAres:
    case eBuildingType::majorShrineArtemis:
    case eBuildingType::majorShrineAthena:
    case eBuildingType::majorShrineAtlas:
    case eBuildingType::majorShrineDemeter:
    case eBuildingType::majorShrineDionysus:
    case eBuildingType::majorShrineHades:
    case eBuildingType::majorShrineHephaestus:
    case eBuildingType::majorShrineHera:
    case eBuildingType::majorShrineHermes:
    case eBuildingType::majorShrinePoseidon:
    case eBuildingType::majorShrineZeus: return {30, 6, 2, -2};

    case eBuildingType::pyramidOfThePantheon: return {30, 6, 1, -2};
    case eBuildingType::altarOfOlympus:  return {25, 5, 1, -2};
    case eBuildingType::templeOfOlympus:  return {30, 6, 1, -3};
    case eBuildingType::observatoryKosmika:  return {25, 6, 1, -1};
    case eBuildingType::museumAtlantika: return {25, 6, 1, -1};

    case eBuildingType::artisansGuild: return {-6, 3, 2, 1};

    case eBuildingType::achillesHall:
    case eBuildingType::atalantaHall:
    case eBuildingType::bellerophonHall:
    case eBuildingType::herculesHall:
    case eBuildingType::jasonHall:
    case eBuildingType::odysseusHall:
    case eBuildingType::perseusHall:
    case eBuildingType::theseusHall: return {15, 6, 2, -2};

    case eBuildingType::triremeWharf: return {-12, 4, 2, 2};

    case eBuildingType::chariotFactory: return {-10, 4, 1, 1};

    default: return {0, 0};
    }
}

eHeat eHeatGetters::housing(const eBuildingType type) {
    const bool a = eBuilding::sAestheticsBuilding(type);
    if(a) return {0, 0};
    const bool s = eBuilding::sSanctuaryBuilding(type);
    if(s) return {0, 0};
    switch(type) {
    case eBuildingType::commonHouse: return {10, 5};
    case eBuildingType::eliteHousing: return {10, 5};
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeat eHeatGetters::industry(const eBuildingType type) {
    const bool a = eBuilding::sAestheticsBuilding(type);
    if(a) return {0, 0};
    const bool s = eBuilding::sSanctuaryBuilding(type);
    if(s) return {0, 0};
    switch(type) {
    case eBuildingType::timberMill: return {10, 5};
    case eBuildingType::masonryShop: return {10, 5};
    case eBuildingType::mint: return {10, 5};
    case eBuildingType::foundry: return {10, 5};
    case eBuildingType::sculptureStudio: return {10, 5};
    case eBuildingType::armory: return {10, 5};
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeat eHeatGetters::farming(const eBuildingType type) {
    const bool a = eBuilding::sAestheticsBuilding(type);
    if(a) return {0, 0};
    const bool s = eBuilding::sSanctuaryBuilding(type);
    if(s) return {0, 0};
    switch(type) {
    case eBuildingType::wheatFarm: return {10, 5};
    case eBuildingType::carrotsFarm: return {10, 5};
    case eBuildingType::onionsFarm: return {10, 5};
    case eBuildingType::growersLodge: return {10, 5};
    case eBuildingType::cardingShed: return {10, 5};
    case eBuildingType::dairy: return {10, 5};
    case eBuildingType::urchinQuay: return {10, 5};
    case eBuildingType::fishery: return {10, 5};
    case eBuildingType::corral: return {10, 5};
    case eBuildingType::huntingLodge: return {10, 5};
    case eBuildingType::orangeTendersLodge: return {10, 5};
    case eBuildingType::oliveTree:
    case eBuildingType::orangeTree:
    case eBuildingType::vine: return {2, 3};
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeat eHeatGetters::storage(const eBuildingType type) {
    const bool a = eBuilding::sAestheticsBuilding(type);
    if(a) return {0, 0};
    const bool s = eBuilding::sSanctuaryBuilding(type);
    if(s) return {0, 0};
    switch(type) {
    case eBuildingType::granary: return {10, 5};
    case eBuildingType::warehouse: return {10, 5};
    case eBuildingType::tradePost: return {10, 5};
    case eBuildingType::oliveTree:
    case eBuildingType::orangeTree:
    case eBuildingType::vine:
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeat eHeatGetters::culture(const eBuildingType type) {
    const bool a = eBuilding::sAestheticsBuilding(type);
    if(a) return {0, 0};
    const bool s = eBuilding::sSanctuaryBuilding(type);
    if(s) return {0, 0};
    switch(type) {
    case eBuildingType::college: return {10, 5};
    case eBuildingType::podium: return {10, 5};
    case eBuildingType::gymnasium: return {10, 5};
    case eBuildingType::dramaSchool: return {10, 5};
    case eBuildingType::theater: return {10, 5};
    case eBuildingType::stadium: return {10, 5};
    case eBuildingType::oliveTree:
    case eBuildingType::orangeTree:
    case eBuildingType::vine:
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeatGetters::eHeatGetter eHeatGetters::godLeaning(const eGodType gt) {
    eHeatGetter hg;
    switch(gt) {
    case eGodType::aphrodite:
        hg = eHeatGetters::housing;
        break;
    case eGodType::apollo:
        hg = eHeatGetters::culture;
        break;
    case eGodType::ares:
        hg = eHeatGetters::industry;
        break;
    case eGodType::artemis:
        hg = eHeatGetters::farming;
        break;
    case eGodType::athena:
        hg = eHeatGetters::industry;
        break;
    case eGodType::atlas:
        hg = eHeatGetters::industry;
        break;
    case eGodType::demeter:
        hg = eHeatGetters::farming;
        break;
    case eGodType::dionysus:
        hg = eHeatGetters::farming;
        break;
    case eGodType::hades:
        hg = eHeatGetters::storage;
        break;
    case eGodType::hephaestus:
        hg = eHeatGetters::industry;
        break;
    case eGodType::hera:
        hg = eHeatGetters::farming;
        break;
    case eGodType::hermes:
        hg = eHeatGetters::culture;
        break;
    case eGodType::poseidon:
        hg = eHeatGetters::farming;
        break;
    case eGodType::zeus:
        hg = eHeatGetters::storage;
        break;
    }
    return hg;
}

eHeat eHeatGetters::empty(const eBuildingType type) {
    (void)type;
    switch(type) {
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {-2, 5};
    }
}

eHeat eHeatGetters::any(const eBuildingType type) {
    (void)type;
    switch(type) {
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::wall:
    case eBuildingType::gatehouse:
    case eBuildingType::tower:
    case eBuildingType::road:
    case eBuildingType::oliveTree:
    case eBuildingType::orangeTree:
    case eBuildingType::vine:
    case eBuildingType::sheep:
    case eBuildingType::cattle:
    case eBuildingType::goat:
    case eBuildingType::palaceTile:
        return {0, 0};
    default: return {2, 5};
    }
}

eHeat eHeatGetters::fertile(eTileBase* const tile) {
    const auto terr = tile->terrain();
    if(terr == eTerrain::fertile) {
        return {2, 8};
    } else {
        return {0, 0};
    }
}

eHeat eHeatGetters::notFertile(eTileBase* const tile) {
    const auto terr = tile->terrain();
    if(terr == eTerrain::fertile) {
        return {0, 0};
    } else {
        return {2, 8};
    }
}

bool eHeatGetters::isMonsterTarget(const eMonsterTarget t,
                                   const eBuildingType bt) {
    using eBT = eBuildingType;
    switch(t) {
    case eMonsterTarget::food:
        return farming(bt).fValue >= 10 || bt == eBT::granary;
    case eMonsterTarget::sea:
        return bt == eBT::fishery || bt == eBT::urchinQuay ||
               bt == eBT::triremeWharf || bt == eBT::tradePost;
    case eMonsterTarget::industry:
        return industry(bt).fValue >= 10 || bt == eBT::refinery ||
               bt == eBT::blackMarbleWorkshop;
    case eMonsterTarget::military:
    case eMonsterTarget::troops:
        return bt == eBT::armory || bt == eBT::horseRanch ||
               bt == eBT::triremeWharf || bt == eBT::chariotFactory;
    case eMonsterTarget::money:
        return bt == eBT::mint || bt == eBT::taxOffice ||
               bt == eBT::palace || bt == eBT::tradePost;
    case eMonsterTarget::common:
        return bt == eBT::commonHouse;
    case eMonsterTarget::aesthetic:
        return eBuilding::sAestheticsBuilding(bt);
    case eMonsterTarget::mythological:
        return eBuilding::sSanctuaryBuilding(bt) ||
               eBuilding::sHeroHall(bt);
    case eMonsterTarget::best:
    case eMonsterTarget::random:
        return false;
    }
    return false;
}

eHeatGetters::eHeatGetter eHeatGetters::monsterTarget(const eMonsterTarget t) {
    if(t == eMonsterTarget::best || t == eMonsterTarget::random) {
        return eHeatGetters::any;
    }
    return [t](const eBuildingType bt) -> eHeat {
        const auto a = any(bt);
        if(a.fValue <= 0) return a;
        if(isMonsterTarget(t, bt)) return {10, 5};
        return a;
    };
}
