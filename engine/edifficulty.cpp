#include "edifficulty.h"

#include "characters/echaracterbase.h"

#include <array>

std::string eDifficultyHelpers::name(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return "beginner";
    case eDifficulty::mortal:
        return "mortal";
    case eDifficulty::hero:
        return "hero";
    case eDifficulty::titan:
        return "titan";
    case eDifficulty::olympian:
        return "olympian";
    }
    // not reached with a valid value: every case above returns
    return "";
}

int costBase(const eBuildingType type) {
    switch(type) {
    case eBuildingType::none:
        return 0;
    case eBuildingType::erase:
        return 1;
    case eBuildingType::road:
        return 2;
    case eBuildingType::bridge:
        return 5;
    case eBuildingType::commonHouse:
        return 10;
    case eBuildingType::eliteHousing:
        return 100;

    case eBuildingType::vine:
    case eBuildingType::oliveTree:
    case eBuildingType::orangeTree:
        return 3;

    case eBuildingType::goat:
    case eBuildingType::sheep:
        return 4;
    case eBuildingType::cattle:
        return 8;

    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
    case eBuildingType::wheatFarm:
        return 20;
    case eBuildingType::cardingShed:
        return 16;
    case eBuildingType::dairy:
        return 16;
    case eBuildingType::fishery:
        return 30;
    case eBuildingType::growersLodge:
        return 25;
    case eBuildingType::orangeTendersLodge:
        return 20;
    case eBuildingType::huntingLodge:
        return 20;
    case eBuildingType::corral:
        return 75;
    case eBuildingType::urchinQuay:
        return 30;

    case eBuildingType::foundry:
        return 60;
    case eBuildingType::masonryShop:
        return 75;
    case eBuildingType::refinery:
        return 65;
    case eBuildingType::blackMarbleWorkshop:
        return 75;
    case eBuildingType::mint:
        return 100;
    case eBuildingType::olivePress:
        return 45;
    case eBuildingType::sculptureStudio:
        return 100;
    case eBuildingType::timberMill:
        return 35;
    case eBuildingType::winery:
        return 45;

    case eBuildingType::wall:
        return 2;
    case eBuildingType::gatehouse:
        return 20;
    case eBuildingType::tower:
        return 50;


    case eBuildingType::granary:
        return 50;
    case eBuildingType::warehouse:
        return 25;

    case eBuildingType::tradePost:
        return 100;
    case eBuildingType::pier:
        return 100;


    case eBuildingType::commonAgora:
        return 25;
    case eBuildingType::grandAgora:
        return 50;
    case eBuildingType::foodVendor:
    case eBuildingType::fleeceVendor:
    case eBuildingType::oilVendor:
    case eBuildingType::wineVendor:
    case eBuildingType::armsVendor:
    case eBuildingType::horseTrainer:
    case eBuildingType::chariotVendor:
        return 10;

    case eBuildingType::fountain:
        return 16;
    case eBuildingType::hospital:
        return 35;
    case eBuildingType::maintenanceOffice:
        return 10;
    case eBuildingType::watchPost:
        return 20;

    case eBuildingType::palace:
        return 125;
    case eBuildingType::taxOffice:
        return 25;

    case eBuildingType::college:
        return 30;
    case eBuildingType::dramaSchool:
        return 16;
    case eBuildingType::gymnasium:
        return 30;
    case eBuildingType::stadium:
        return 200;
    case eBuildingType::podium:
        return 15;
    case eBuildingType::theater:
        return 60;

    case eBuildingType::bibliotheke:
        return 18;
    case eBuildingType::observatory:
        return 75;
    case eBuildingType::university:
        return 30;
    case eBuildingType::laboratory:
        return 65;
    case eBuildingType::inventorsWorkshop:
        return 40;
    case eBuildingType::museum:
        return 225;

    case eBuildingType::artisansGuild:
        return 35;

    case eBuildingType::armory:
        return 40;
    case eBuildingType::triremeWharf:
        return 75;
    case eBuildingType::horseRanch:
        return 100;

    case eBuildingType::park:
        return 6;
    case eBuildingType::doricColumn:
    case eBuildingType::ionicColumn:
    case eBuildingType::corinthianColumn:
        return 8;
    case eBuildingType::avenue:
        return 10;
    case eBuildingType::boulevard:
        return 15;

    case eBuildingType::bench:
        return 6;
    case eBuildingType::flowerGarden:
        return 20;
    case eBuildingType::gazebo:
        return 24;
    case eBuildingType::hedgeMaze:
        return 40;
    case eBuildingType::fishPond:
        return 60;

    case eBuildingType::waterPark:
        return 16;

    case eBuildingType::birdBath:
        return 6;
    case eBuildingType::shortObelisk:
        return 10;
    case eBuildingType::tallObelisk:
        return 12;
    case eBuildingType::shellGarden:
        return 20;
    case eBuildingType::sundial:
        return 20;
    case eBuildingType::dolphinSculpture:
        return 40;
    case eBuildingType::orrery:
        return 40;
    case eBuildingType::spring:
        return 40;
    case eBuildingType::topiary:
        return 38;
    case eBuildingType::baths:
        return 60;
    case eBuildingType::stoneCircle:
        return 60;

    case eBuildingType::modestPyramid:
        return 0;
    case eBuildingType::pyramid:
        return 0;
    case eBuildingType::greatPyramid:
        return 0;
    case eBuildingType::majesticPyramid:
        return 0;

    case eBuildingType::smallMonumentToTheSky:
        return 0;
    case eBuildingType::monumentToTheSky:
        return 0;
    case eBuildingType::grandMonumentToTheSky:
        return 0;

    case eBuildingType::minorShrineAphrodite:
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
    case eBuildingType::minorShrineZeus:
        return 0;
    case eBuildingType::shrineAphrodite:
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
    case eBuildingType::shrineZeus:
        return 0;
    case eBuildingType::majorShrineAphrodite:
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
    case eBuildingType::majorShrineZeus:
        return 0;

    case eBuildingType::pyramidOfThePantheon:
        return 0;
    case eBuildingType::altarOfOlympus:
        return 0;
    case eBuildingType::templeOfOlympus:
        return 0;
    case eBuildingType::observatoryKosmika:
        return 0;
    case eBuildingType::museumAtlantika:
        return 0;

    case eBuildingType::templeZeus:
        return 2920;
    case eBuildingType::templePoseidon:
        return 2480;
    case eBuildingType::templeHades:
        return 1320;
    case eBuildingType::templeHera:
        return 1760;
    case eBuildingType::templeDemeter:
        return 840;
    case eBuildingType::templeAthena:
        return 2160;
    case eBuildingType::templeArtemis:
        return 1520;
    case eBuildingType::templeApollo:
        return 920;
    case eBuildingType::templeAtlas:
        return 1760;
    case eBuildingType::templeAres:
        return 960;
    case eBuildingType::templeHephaestus:
        return 760;
    case eBuildingType::templeAphrodite:
        return 720;
    case eBuildingType::templeHermes:
        return 640;
    case eBuildingType::templeDionysus:
        return 400;

    case eBuildingType::chariotFactory:
        return 110;

    case eBuildingType::agoraSpace:
    case eBuildingType::ruins:
    case eBuildingType::placeholder:
    case eBuildingType::horseRanchEnclosure:
    case eBuildingType::commemorative:
    case eBuildingType::palaceTile:
    case eBuildingType::godMonument:
    case eBuildingType::godMonumentTile:

    case eBuildingType::temple:
    case eBuildingType::templeTile:
    case eBuildingType::templeStatue:
    case eBuildingType::templeGrandStatue:
    case eBuildingType::templeObelisk:
    case eBuildingType::templeMonument:
    case eBuildingType::templeAltar:

    case eBuildingType::achillesHall:
    case eBuildingType::atalantaHall:
    case eBuildingType::bellerophonHall:
    case eBuildingType::herculesHall:
    case eBuildingType::jasonHall:
    case eBuildingType::odysseusHall:
    case eBuildingType::perseusHall:
    case eBuildingType::theseusHall:

    case eBuildingType::pyramidPart:
    case eBuildingType::pyramidWall:
    case eBuildingType::pyramidTop:
    case eBuildingType::pyramidStatue:
    case eBuildingType::pyramidMonument:
    case eBuildingType::pyramidTile:
    case eBuildingType::pyramidAltar:
    case eBuildingType::pyramidTemple:
    case eBuildingType::pyramidObservatory:
    case eBuildingType::pyramidMuseum:
        return 0;

    case eBuildingType::hippodromePiece:
        return 30;
    case eBuildingType::crosswalk:
        return 25;
    }
    // not reached with a valid value: every case above returns
    return 0;
}

double costMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1.0;
    case eDifficulty::mortal:
        return 1.8;
    case eDifficulty::hero:
        return 2.25;
    case eDifficulty::titan:
        return 2.75;
    case eDifficulty::olympian:
        return 3.25;
    }
    return 1;
}

// Construction cost per difficulty (beginner, mortal, hero, titan, olympian),
// column "CST" of the original game data Model/Zeus_Model_VeryEasy, _Easy,
// _Normal, _Hard and _Impossible.txt. The original sets each value by hand;
// it is not a single multiplier per difficulty.
bool originalCost(const eBuildingType type, std::array<int, 5>& c) {
    switch(type) {
    case eBuildingType::commonHouse: c = {10, 15, 20, 25, 30}; return true; // BUILD_HOUSE1_HUT
    case eBuildingType::eliteHousing: c = {100, 150, 200, 250, 300}; return true; // BUILD_ELITE_VACANT
    case eBuildingType::flowerGarden: c = {20, 32, 40, 50, 60}; return true; // BUILD_FLOWER_GARDEN
    case eBuildingType::gazebo: c = {24, 36, 45, 58, 72}; return true; // BUILD_GAZEBO
    case eBuildingType::hedgeMaze: c = {40, 70, 85, 105, 125}; return true; // BUILD_HEDGE_MAZE
    case eBuildingType::fishPond: c = {60, 100, 125, 145, 185}; return true; // BUILD_FISH_POND
    case eBuildingType::tallObelisk: c = {12, 20, 24, 30, 36}; return true; // BUILD_TALL_OBELISK
    case eBuildingType::sundial: c = {20, 32, 40, 48, 60}; return true; // BUILD_SUNDIAL
    case eBuildingType::topiary: c = {38, 60, 75, 90, 110}; return true; // BUILD_TOPIARY
    case eBuildingType::spring: c = {40, 68, 85, 105, 130}; return true; // BUILD_SPRING
    case eBuildingType::stoneCircle: c = {60, 100, 120, 150, 180}; return true; // BUILD_STONE_CIRCLE
    case eBuildingType::shortObelisk: c = {10, 15, 18, 22, 27}; return true; // BUILD_SHORT_OBELISK
    case eBuildingType::waterPark: c = {16, 25, 32, 38, 48}; return true; // BUILD_WATER_PARK
    case eBuildingType::road: c = {2, 2, 3, 4, 5}; return true; // BUILD_ROAD
    case eBuildingType::erase: c = {1, 2, 2, 2, 3}; return true; // BUILD_DIG
    case eBuildingType::wheatFarm: c = {20, 36, 45, 55, 65}; return true; // BUILD_WHEAT_FARM
    case eBuildingType::onionsFarm: c = {20, 36, 45, 55, 65}; return true; // BUILD_ONION_FARM
    case eBuildingType::carrotsFarm: c = {20, 36, 45, 55, 65}; return true; // BUILD_CARROT_FARM
    case eBuildingType::vine: c = {3, 5, 6, 7, 10}; return true; // BUILD_GRAPEVINE
    case eBuildingType::oliveTree: c = {3, 5, 6, 7, 10}; return true; // BUILD_OLIVE_TREE
    case eBuildingType::growersLodge: c = {25, 40, 50, 63, 75}; return true; // BUILD_GROWERS_LODGE
    case eBuildingType::goat: c = {4, 7, 8, 10, 15}; return true; // BUILD_GOAT
    case eBuildingType::sheep: c = {4, 7, 8, 10, 15}; return true; // BUILD_SHEEP
    case eBuildingType::cardingShed: c = {16, 30, 35, 42, 50}; return true; // BUILD_SHEEP_FARM
    case eBuildingType::dairy: c = {16, 30, 35, 42, 50}; return true; // BUILD_GOAT_FARM
    case eBuildingType::fishery: c = {30, 50, 65, 76, 100}; return true; // BUILD_FISHING_WHARF
    case eBuildingType::urchinQuay: c = {30, 50, 65, 76, 100}; return true; // BUILD_URCHIN_COLLECTOR
    case eBuildingType::huntingLodge: c = {20, 32, 40, 50, 60}; return true; // BUILD_HUNTING_LODGE
    case eBuildingType::mint: c = {100, 160, 200, 240, 300}; return true; // BUILD_MINT
    case eBuildingType::masonryShop: c = {75, 120, 150, 180, 225}; return true; // BUILD_MARBLE_QUARRY
    case eBuildingType::foundry: c = {60, 105, 120, 120, 180}; return true; // BUILD_FOUNDRY
    case eBuildingType::timberMill: c = {35, 60, 75, 90, 125}; return true; // BUILD_TIMBER_MILL
    case eBuildingType::winery: c = {45, 72, 90, 105, 135}; return true; // BUILD_WINERY
    case eBuildingType::olivePress: c = {45, 72, 90, 105, 135}; return true; // BUILD_OIL_PRESS
    case eBuildingType::sculptureStudio: c = {100, 160, 200, 240, 300}; return true; // BUILD_SCULPTURE_STUDIO
    case eBuildingType::artisansGuild: c = {35, 60, 75, 90, 125}; return true; // BUILD_ARTISANS_GUILD
    case eBuildingType::granary: c = {50, 80, 100, 120, 150}; return true; // BUILD_GRANARY
    case eBuildingType::warehouse: c = {25, 40, 50, 60, 75}; return true; // BUILD_STOREHOUSE
    case eBuildingType::pier: c = {100, 160, 200, 240, 300}; return true; // BUILD_DOCK
    case eBuildingType::tradePost: c = {100, 160, 200, 240, 300}; return true; // BUILD_TRADING_POST
    case eBuildingType::commonAgora: c = {25, 40, 50, 60, 75}; return true; // BUILD_AGORA_SMALL
    case eBuildingType::grandAgora: c = {50, 80, 100, 120, 150}; return true; // BUILD_AGORA_LARGE
    case eBuildingType::foodVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_FOOD_VENDOR
    case eBuildingType::fleeceVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_FLEECE_VENDOR
    case eBuildingType::oilVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_OIL_VENDOR
    case eBuildingType::wineVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_WINE_VENDOR
    case eBuildingType::armsVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_ARMS_VENDOR
    case eBuildingType::horseTrainer: c = {10, 16, 20, 24, 30}; return true; // BUILD_HORSE_TRAINER
    case eBuildingType::fountain: c = {16, 30, 35, 42, 50}; return true; // BUILD_FOUNTAIN
    case eBuildingType::hospital: c = {35, 56, 70, 85, 105}; return true; // BUILD_INFIRMARY
    case eBuildingType::college: c = {30, 50, 65, 75, 100}; return true; // BUILD_COLLEGE
    case eBuildingType::dramaSchool: c = {16, 30, 35, 42, 50}; return true; // BUILD_DRAMA_SCHOOL
    case eBuildingType::gymnasium: c = {30, 60, 75, 90, 120}; return true; // BUILD_GYMNASIUM
    case eBuildingType::stadium: c = {200, 320, 400, 500, 600}; return true; // BUILD_STADIUM
    case eBuildingType::podium: c = {15, 24, 30, 35, 45}; return true; // BUILD_PODIUM
    case eBuildingType::theater: c = {60, 100, 120, 145, 180}; return true; // BUILD_THEATRE
    case eBuildingType::palace: c = {125, 175, 250, 300, 400}; return true; // BUILD_PALACE
    case eBuildingType::avenue: c = {10, 16, 20, 25, 30}; return true; // BUILD_AVENUE
    case eBuildingType::boulevard: c = {15, 24, 30, 36, 45}; return true; // BUILD_BOULEVARD
    case eBuildingType::bridge: c = {8, 12, 15, 16, 20}; return true; // BUILD_LARGE_BRIDGE
    case eBuildingType::maintenanceOffice: c = {10, 16, 20, 24, 30}; return true; // BUILD_SUPER_OFFICE
    case eBuildingType::taxOffice: c = {25, 40, 50, 60, 75}; return true; // BUILD_TAX_OFFICE
    case eBuildingType::watchPost: c = {20, 32, 40, 48, 60}; return true; // BUILD_GUARDPOST
    case eBuildingType::bench: c = {6, 10, 12, 15, 20}; return true; // BUILD_BENCH
    case eBuildingType::park: c = {6, 10, 12, 15, 20}; return true; // BUILD_GARDENS
    case eBuildingType::doricColumn: c = {8, 12, 16, 18, 24}; return true; // BUILD_DORIC_COLUMN
    case eBuildingType::wall: c = {2, 3, 5, 6, 8}; return true; // BUILD_WALL
    case eBuildingType::gatehouse: c = {20, 60, 80, 100, 120}; return true; // BUILD_GATEHOUSE
    case eBuildingType::tower: c = {50, 80, 100, 120, 150}; return true; // BUILD_TOWER
    case eBuildingType::horseRanch: c = {100, 160, 200, 240, 300}; return true; // BUILD_HORSE_RANCH
    case eBuildingType::armory: c = {40, 65, 85, 100, 125}; return true; // BUILD_ARMORY
    case eBuildingType::triremeWharf: c = {75, 125, 150, 200, 225}; return true; // BUILD_WARSHIP_WHARF
    case eBuildingType::ionicColumn: c = {8, 12, 16, 18, 24}; return true; // BUILD_IONIC_COLUMN
    case eBuildingType::corinthianColumn: c = {8, 12, 16, 18, 24}; return true; // BUILD_CORINTHIAN_COLUMN
    case eBuildingType::dolphinSculpture: c = {40, 64, 80, 80, 120}; return true; // BUILD_DOLPHIN_SCULPTURE
    case eBuildingType::orrery: c = {45, 72, 90, 90, 135}; return true; // BUILD_ORRERY
    case eBuildingType::shellGarden: c = {20, 32, 40, 40, 60}; return true; // BUILD_SHELL_GARDEN
    case eBuildingType::baths: c = {60, 95, 115, 115, 175}; return true; // BUILD_BATHS
    case eBuildingType::birdBath: c = {6, 10, 12, 12, 18}; return true; // BUILD_BIRD_BATH
    case eBuildingType::hippodromePiece: c = {30, 45, 60, 75, 100}; return true; // BUILD_RACE_TRACK
    case eBuildingType::crosswalk: c = {25, 35, 50, 60, 75}; return true; // BUILD_CROSSWALK
    case eBuildingType::bibliotheke: c = {18, 27, 35, 45, 50}; return true; // BUILD_BIBLIOTHEKE
    case eBuildingType::observatory: c = {75, 120, 150, 175, 225}; return true; // BUILD_OBSERVATORY
    case eBuildingType::university: c = {30, 50, 65, 80, 100}; return true; // BUILD_UNIVERSITY
    case eBuildingType::laboratory: c = {65, 95, 120, 140, 180}; return true; // BUILD_LABORATORY
    case eBuildingType::inventorsWorkshop: c = {40, 70, 85, 100, 130}; return true; // BUILD_INVENTORS_WORKSHOP
    case eBuildingType::museum: c = {225, 325, 400, 500, 575}; return true; // BUILD_MUSUEM
    case eBuildingType::refinery: c = {65, 100, 120, 135, 180}; return true; // BUILD_ORICHALC_REFINERY
    case eBuildingType::chariotFactory: c = {110, 175, 225, 250, 300}; return true; // BUILD_CHARIOT_WORKSHOP
    case eBuildingType::blackMarbleWorkshop: c = {75, 120, 150, 180, 225}; return true; // BUILD_BLACK_MARBLE_SHOP
    case eBuildingType::orangeTendersLodge: c = {20, 33, 45, 58, 65}; return true; // BUILD_FRUIT_GROWERS_LODGE
    case eBuildingType::chariotVendor: c = {10, 16, 20, 24, 30}; return true; // BUILD_CHARIOT_VENDOR
    case eBuildingType::corral: c = {75, 124, 150, 170, 230}; return true; // BUILD_CORRAL_SLAUGHTER
    case eBuildingType::orangeTree: c = {3, 5, 6, 7, 10}; return true; // BUILD_FRUIT_TREE
    case eBuildingType::cattle: c = {8, 12, 15, 18, 22}; return true; // BUILD_CATTLE
    default: return false;
    }
}

int eDifficultyHelpers::buildingCost(
        const eDifficulty diff,
        const eBuildingType type) {
    const int base = costBase(type);
    const int min = static_cast<int>(eBuildingType::templeAphrodite);
    const int max = static_cast<int>(eBuildingType::templeZeus);
    const int bi = static_cast<int>(type);
    if(bi >= min && bi <= max) return base;
    std::array<int, 5> orig;
    if(originalCost(type, orig)) {
        return orig[static_cast<int>(diff)];
    }
    const double mult = costMultiplier(diff);
    return std::round(mult*base);
}

// The original puts the fire risk on the agora as one building (6/12/12/12/20)
// and gives its stalls none (Model rows BUILD_AGORA_* and BUILD_*_VENDOR).
// FRI per difficulty (beginner, mortal, hero, titan, olympian), column
// "FRI" of the original Model/Zeus_Model_*.txt (VeryEasy..Impossible).
bool originalFireRisk(const eBuildingType type, std::array<int, 5>& r) {
    switch(type) {
    case eBuildingType::commonAgora: r = {6, 12, 12, 12, 20}; return true; // BUILD_AGORA_SMALL
    case eBuildingType::grandAgora: r = {6, 12, 12, 12, 20}; return true; // BUILD_AGORA_LARGE
    case eBuildingType::foodVendor:
    case eBuildingType::fleeceVendor:
    case eBuildingType::oilVendor:
    case eBuildingType::wineVendor:
    case eBuildingType::armsVendor:
    case eBuildingType::horseTrainer:
    case eBuildingType::chariotVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_*_VENDOR
    case eBuildingType::commonHouse: r = {8, 12, 15, 18, 20}; return true; // BUILD_HOUSE1_HUT
    case eBuildingType::eliteHousing: r = {8, 12, 15, 18, 20}; return true; // BUILD_ELITE_VACANT
    case eBuildingType::flowerGarden: r = {0, 0, 0, 0, 0}; return true; // BUILD_FLOWER_GARDEN
    case eBuildingType::gazebo: r = {0, 0, 0, 0, 0}; return true; // BUILD_GAZEBO
    case eBuildingType::hedgeMaze: r = {0, 0, 0, 0, 0}; return true; // BUILD_HEDGE_MAZE
    case eBuildingType::fishPond: r = {0, 0, 0, 0, 0}; return true; // BUILD_FISH_POND
    case eBuildingType::tallObelisk: r = {0, 0, 0, 0, 0}; return true; // BUILD_TALL_OBELISK
    case eBuildingType::sundial: r = {0, 0, 0, 0, 0}; return true; // BUILD_SUNDIAL
    case eBuildingType::topiary: r = {0, 0, 0, 0, 0}; return true; // BUILD_TOPIARY
    case eBuildingType::spring: r = {0, 0, 0, 0, 0}; return true; // BUILD_SPRING
    case eBuildingType::stoneCircle: r = {0, 0, 0, 0, 0}; return true; // BUILD_STONE_CIRCLE
    case eBuildingType::shortObelisk: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHORT_OBELISK
    case eBuildingType::waterPark: r = {0, 0, 0, 0, 0}; return true; // BUILD_WATER_PARK
    case eBuildingType::road: r = {0, 0, 0, 0, 0}; return true; // BUILD_ROAD
    case eBuildingType::erase: r = {0, 0, 0, 0, 0}; return true; // BUILD_DIG
    case eBuildingType::wheatFarm: r = {5, 10, 10, 12, 20}; return true; // BUILD_WHEAT_FARM
    case eBuildingType::onionsFarm: r = {0, 5, 5, 5, 5}; return true; // BUILD_ONION_FARM
    case eBuildingType::carrotsFarm: r = {0, 5, 5, 5, 5}; return true; // BUILD_CARROT_FARM
    case eBuildingType::vine: r = {0, 0, 0, 0, 0}; return true; // BUILD_GRAPEVINE
    case eBuildingType::oliveTree: r = {0, 0, 0, 0, 0}; return true; // BUILD_OLIVE_TREE
    case eBuildingType::growersLodge: r = {5, 10, 10, 12, 20}; return true; // BUILD_GROWERS_LODGE
    case eBuildingType::goat: r = {0, 0, 0, 0, 0}; return true; // BUILD_GOAT
    case eBuildingType::sheep: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHEEP
    case eBuildingType::cardingShed: r = {5, 10, 10, 15, 20}; return true; // BUILD_SHEEP_FARM
    case eBuildingType::dairy: r = {5, 10, 10, 15, 20}; return true; // BUILD_GOAT_FARM
    case eBuildingType::fishery: r = {5, 10, 10, 15, 20}; return true; // BUILD_FISHING_WHARF
    case eBuildingType::urchinQuay: r = {5, 10, 10, 15, 20}; return true; // BUILD_URCHIN_COLLECTOR
    case eBuildingType::huntingLodge: r = {8, 15, 15, 18, 25}; return true; // BUILD_HUNTING_LODGE
    case eBuildingType::mint: r = {0, 0, 0, 0, 0}; return true; // BUILD_MINT
    case eBuildingType::masonryShop: r = {0, 0, 0, 0, 0}; return true; // BUILD_MARBLE_QUARRY
    case eBuildingType::foundry: r = {0, 0, 0, 0, 0}; return true; // BUILD_FOUNDRY
    case eBuildingType::timberMill: r = {10, 20, 20, 20, 25}; return true; // BUILD_TIMBER_MILL
    case eBuildingType::winery: r = {0, 0, 0, 0, 0}; return true; // BUILD_WINERY
    case eBuildingType::olivePress: r = {0, 0, 0, 0, 0}; return true; // BUILD_OIL_PRESS
    case eBuildingType::sculptureStudio: r = {0, 2, 2, 5, 8}; return true; // BUILD_SCULPTURE_STUDIO
    case eBuildingType::artisansGuild: r = {5, 10, 10, 12, 20}; return true; // BUILD_ARTISANS_GUILD
    case eBuildingType::granary: r = {8, 12, 16, 16, 20}; return true; // BUILD_GRANARY
    case eBuildingType::warehouse: r = {5, 10, 10, 15, 18}; return true; // BUILD_STOREHOUSE
    case eBuildingType::pier: r = {5, 10, 10, 15, 18}; return true; // BUILD_DOCK
    case eBuildingType::tradePost: r = {5, 10, 10, 15, 10}; return true; // BUILD_TRADING_POST
    case eBuildingType::fountain: r = {0, 0, 0, 0, 0}; return true; // BUILD_FOUNTAIN
    case eBuildingType::hospital: r = {5, 10, 10, 15, 15}; return true; // BUILD_INFIRMARY
    case eBuildingType::college: r = {0, 0, 0, 0, 0}; return true; // BUILD_COLLEGE
    case eBuildingType::dramaSchool: r = {0, 0, 0, 0, 0}; return true; // BUILD_DRAMA_SCHOOL
    case eBuildingType::gymnasium: r = {0, 0, 0, 0, 0}; return true; // BUILD_GYMNASIUM
    case eBuildingType::stadium: r = {0, 0, 0, 0, 0}; return true; // BUILD_STADIUM
    case eBuildingType::podium: r = {0, 0, 0, 0, 0}; return true; // BUILD_PODIUM
    case eBuildingType::theater: r = {0, 0, 0, 0, 0}; return true; // BUILD_THEATRE
    case eBuildingType::palace: r = {0, 5, 5, 10, 15}; return true; // BUILD_PALACE
    case eBuildingType::avenue: r = {0, 0, 0, 0, 0}; return true; // BUILD_AVENUE
    case eBuildingType::boulevard: r = {0, 0, 0, 0, 0}; return true; // BUILD_BOULEVARD
    case eBuildingType::bridge: r = {0, 0, 0, 0, 0}; return true; // BUILD_LARGE_BRIDGE
    case eBuildingType::maintenanceOffice: r = {0, 5, 5, 8, 10}; return true; // BUILD_SUPER_OFFICE
    case eBuildingType::taxOffice: r = {5, 10, 10, 12, 15}; return true; // BUILD_TAX_OFFICE
    case eBuildingType::watchPost: r = {0, 0, 0, 0, 0}; return true; // BUILD_GUARDPOST
    case eBuildingType::bench: r = {0, 0, 0, 0, 0}; return true; // BUILD_BENCH
    case eBuildingType::park: r = {0, 0, 0, 0, 0}; return true; // BUILD_GARDENS
    case eBuildingType::doricColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_DORIC_COLUMN
    case eBuildingType::wall: r = {0, 0, 0, 0, 0}; return true; // BUILD_WALL
    case eBuildingType::gatehouse: r = {0, 0, 0, 0, 0}; return true; // BUILD_GATEHOUSE
    case eBuildingType::tower: r = {0, 0, 0, 0, 0}; return true; // BUILD_TOWER
    case eBuildingType::horseRanch: r = {5, 10, 10, 10, 20}; return true; // BUILD_HORSE_RANCH
    case eBuildingType::armory: r = {8, 12, 15, 20, 25}; return true; // BUILD_ARMORY
    case eBuildingType::triremeWharf: r = {5, 10, 10, 15, 20}; return true; // BUILD_WARSHIP_WHARF
    case eBuildingType::ionicColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_IONIC_COLUMN
    case eBuildingType::corinthianColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_CORINTHIAN_COLUMN
    case eBuildingType::dolphinSculpture: r = {0, 0, 0, 0, 0}; return true; // BUILD_DOLPHIN_SCULPTURE
    case eBuildingType::orrery: r = {0, 0, 0, 0, 0}; return true; // BUILD_ORRERY
    case eBuildingType::shellGarden: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHELL_GARDEN
    case eBuildingType::baths: r = {0, 0, 0, 0, 0}; return true; // BUILD_BATHS
    case eBuildingType::birdBath: r = {0, 0, 0, 0, 0}; return true; // BUILD_BIRD_BATH
    case eBuildingType::hippodromePiece: r = {0, 0, 0, 0, 0}; return true; // BUILD_RACE_TRACK
    case eBuildingType::crosswalk: r = {0, 0, 0, 0, 0}; return true; // BUILD_CROSSWALK
    case eBuildingType::bibliotheke: r = {5, 10, 15, 18, 25}; return true; // BUILD_BIBLIOTHEKE
    case eBuildingType::observatory: r = {0, 0, 0, 0, 0}; return true; // BUILD_OBSERVATORY
    case eBuildingType::university: r = {0, 0, 0, 0, 0}; return true; // BUILD_UNIVERSITY
    case eBuildingType::laboratory: r = {8, 12, 20, 20, 20}; return true; // BUILD_LABORATORY
    case eBuildingType::inventorsWorkshop: r = {8, 12, 20, 15, 20}; return true; // BUILD_INVENTORS_WORKSHOP
    case eBuildingType::museum: r = {0, 8, 10, 10, 10}; return true; // BUILD_MUSUEM
    case eBuildingType::refinery: r = {0, 0, 5, 5, 10}; return true; // BUILD_ORICHALC_REFINERY
    case eBuildingType::chariotFactory: r = {6, 12, 20, 20, 22}; return true; // BUILD_CHARIOT_WORKSHOP
    case eBuildingType::blackMarbleWorkshop: r = {0, 0, 0, 0, 0}; return true; // BUILD_BLACK_MARBLE_SHOP
    case eBuildingType::orangeTendersLodge: r = {8, 7, 10, 18, 15}; return true; // BUILD_FRUIT_GROWERS_LODGE
    case eBuildingType::corral: r = {8, 7, 10, 18, 15}; return true; // BUILD_CORRAL_SLAUGHTER
    case eBuildingType::orangeTree: r = {0, 0, 0, 0, 0}; return true; // BUILD_FRUIT_TREE
    case eBuildingType::cattle: r = {0, 0, 0, 0, 0}; return true; // BUILD_CATTLE
    default: return false;
    }
}

// DRI per difficulty (beginner, mortal, hero, titan, olympian), column
// "DRI" of the original Model/Zeus_Model_*.txt (VeryEasy..Impossible).
bool originalDamageRisk(const eBuildingType type, std::array<int, 5>& r) {
    switch(type) {
    case eBuildingType::commonHouse: r = {0, 0, 0, 0, 0}; return true; // BUILD_HOUSE1_HUT
    case eBuildingType::eliteHousing: r = {0, 0, 0, 0, 0}; return true; // BUILD_ELITE_VACANT
    case eBuildingType::flowerGarden: r = {0, 0, 0, 0, 0}; return true; // BUILD_FLOWER_GARDEN
    case eBuildingType::gazebo: r = {0, 0, 0, 0, 0}; return true; // BUILD_GAZEBO
    case eBuildingType::hedgeMaze: r = {0, 0, 0, 0, 0}; return true; // BUILD_HEDGE_MAZE
    case eBuildingType::fishPond: r = {0, 0, 0, 0, 0}; return true; // BUILD_FISH_POND
    case eBuildingType::tallObelisk: r = {0, 0, 0, 0, 0}; return true; // BUILD_TALL_OBELISK
    case eBuildingType::sundial: r = {0, 0, 0, 0, 0}; return true; // BUILD_SUNDIAL
    case eBuildingType::topiary: r = {0, 0, 0, 0, 0}; return true; // BUILD_TOPIARY
    case eBuildingType::spring: r = {0, 0, 0, 0, 0}; return true; // BUILD_SPRING
    case eBuildingType::stoneCircle: r = {0, 0, 0, 0, 0}; return true; // BUILD_STONE_CIRCLE
    case eBuildingType::shortObelisk: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHORT_OBELISK
    case eBuildingType::waterPark: r = {0, 0, 0, 0, 0}; return true; // BUILD_WATER_PARK
    case eBuildingType::road: r = {0, 0, 0, 0, 0}; return true; // BUILD_ROAD
    case eBuildingType::erase: r = {0, 0, 0, 0, 0}; return true; // BUILD_DIG
    case eBuildingType::wheatFarm: r = {0, 5, 5, 5, 10}; return true; // BUILD_WHEAT_FARM
    case eBuildingType::onionsFarm: r = {5, 10, 10, 12, 20}; return true; // BUILD_ONION_FARM
    case eBuildingType::carrotsFarm: r = {5, 10, 10, 12, 20}; return true; // BUILD_CARROT_FARM
    case eBuildingType::vine: r = {0, 0, 0, 0, 0}; return true; // BUILD_GRAPEVINE
    case eBuildingType::oliveTree: r = {0, 0, 0, 0, 0}; return true; // BUILD_OLIVE_TREE
    case eBuildingType::growersLodge: r = {0, 0, 0, 0, 0}; return true; // BUILD_GROWERS_LODGE
    case eBuildingType::goat: r = {0, 0, 0, 0, 0}; return true; // BUILD_GOAT
    case eBuildingType::sheep: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHEEP
    case eBuildingType::cardingShed: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHEEP_FARM
    case eBuildingType::dairy: r = {0, 0, 0, 0, 0}; return true; // BUILD_GOAT_FARM
    case eBuildingType::fishery: r = {5, 10, 10, 15, 20}; return true; // BUILD_FISHING_WHARF
    case eBuildingType::urchinQuay: r = {5, 10, 10, 15, 20}; return true; // BUILD_URCHIN_COLLECTOR
    case eBuildingType::huntingLodge: r = {0, 5, 5, 5, 5}; return true; // BUILD_HUNTING_LODGE
    case eBuildingType::mint: r = {10, 15, 20, 20, 25}; return true; // BUILD_MINT
    case eBuildingType::masonryShop: r = {10, 15, 20, 20, 25}; return true; // BUILD_MARBLE_QUARRY
    case eBuildingType::foundry: r = {10, 15, 20, 20, 25}; return true; // BUILD_FOUNDRY
    case eBuildingType::timberMill: r = {0, 0, 0, 0, 0}; return true; // BUILD_TIMBER_MILL
    case eBuildingType::winery: r = {5, 10, 10, 15, 20}; return true; // BUILD_WINERY
    case eBuildingType::olivePress: r = {8, 15, 15, 18, 20}; return true; // BUILD_OIL_PRESS
    case eBuildingType::sculptureStudio: r = {10, 15, 20, 22, 25}; return true; // BUILD_SCULPTURE_STUDIO
    case eBuildingType::artisansGuild: r = {0, 0, 0, 0, 0}; return true; // BUILD_ARTISANS_GUILD
    case eBuildingType::granary: r = {8, 12, 16, 20, 20}; return true; // BUILD_GRANARY
    case eBuildingType::warehouse: r = {0, 0, 0, 0, 0}; return true; // BUILD_STOREHOUSE
    case eBuildingType::pier: r = {0, 0, 0, 0, 0}; return true; // BUILD_DOCK
    case eBuildingType::tradePost: r = {0, 0, 0, 0, 0}; return true; // BUILD_TRADING_POST
    case eBuildingType::commonAgora: r = {0, 0, 0, 0, 0}; return true; // BUILD_AGORA_SMALL
    case eBuildingType::grandAgora: r = {0, 0, 0, 0, 0}; return true; // BUILD_AGORA_LARGE
    case eBuildingType::foodVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_FOOD_VENDOR
    case eBuildingType::fleeceVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_FLEECE_VENDOR
    case eBuildingType::oilVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_OIL_VENDOR
    case eBuildingType::wineVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_WINE_VENDOR
    case eBuildingType::armsVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_ARMS_VENDOR
    case eBuildingType::horseTrainer: r = {0, 0, 0, 0, 0}; return true; // BUILD_HORSE_TRAINER
    case eBuildingType::fountain: r = {8, 8, 8, 12, 8}; return true; // BUILD_FOUNTAIN
    case eBuildingType::hospital: r = {5, 10, 10, 15, 15}; return true; // BUILD_INFIRMARY
    case eBuildingType::college: r = {6, 12, 12, 15, 18}; return true; // BUILD_COLLEGE
    case eBuildingType::dramaSchool: r = {8, 10, 15, 18, 18}; return true; // BUILD_DRAMA_SCHOOL
    case eBuildingType::gymnasium: r = {0, 5, 5, 7, 10}; return true; // BUILD_GYMNASIUM
    case eBuildingType::stadium: r = {0, 8, 8, 12, 15}; return true; // BUILD_STADIUM
    case eBuildingType::podium: r = {0, 5, 5, 7, 10}; return true; // BUILD_PODIUM
    case eBuildingType::theater: r = {8, 10, 15, 18, 22}; return true; // BUILD_THEATRE
    case eBuildingType::palace: r = {0, 5, 5, 10, 15}; return true; // BUILD_PALACE
    case eBuildingType::avenue: r = {0, 0, 0, 0, 0}; return true; // BUILD_AVENUE
    case eBuildingType::boulevard: r = {0, 0, 0, 0, 0}; return true; // BUILD_BOULEVARD
    case eBuildingType::bridge: r = {0, 0, 0, 0, 0}; return true; // BUILD_LARGE_BRIDGE
    case eBuildingType::maintenanceOffice: r = {0, 0, 0, 0, 0}; return true; // BUILD_SUPER_OFFICE
    case eBuildingType::taxOffice: r = {0, 0, 0, 0, 0}; return true; // BUILD_TAX_OFFICE
    case eBuildingType::watchPost: r = {5, 10, 10, 12, 15}; return true; // BUILD_GUARDPOST
    case eBuildingType::bench: r = {0, 0, 0, 0, 0}; return true; // BUILD_BENCH
    case eBuildingType::park: r = {0, 0, 0, 0, 0}; return true; // BUILD_GARDENS
    case eBuildingType::doricColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_DORIC_COLUMN
    case eBuildingType::wall: r = {0, 0, 0, 0, 0}; return true; // BUILD_WALL
    case eBuildingType::gatehouse: r = {0, 0, 0, 0, 0}; return true; // BUILD_GATEHOUSE
    case eBuildingType::tower: r = {0, 0, 0, 0, 0}; return true; // BUILD_TOWER
    case eBuildingType::horseRanch: r = {0, 0, 0, 0, 0}; return true; // BUILD_HORSE_RANCH
    case eBuildingType::armory: r = {0, 0, 0, 0, 0}; return true; // BUILD_ARMORY
    case eBuildingType::triremeWharf: r = {5, 10, 10, 15, 20}; return true; // BUILD_WARSHIP_WHARF
    case eBuildingType::ionicColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_IONIC_COLUMN
    case eBuildingType::corinthianColumn: r = {0, 0, 0, 0, 0}; return true; // BUILD_CORINTHIAN_COLUMN
    case eBuildingType::dolphinSculpture: r = {0, 0, 0, 0, 0}; return true; // BUILD_DOLPHIN_SCULPTURE
    case eBuildingType::orrery: r = {0, 0, 0, 0, 0}; return true; // BUILD_ORRERY
    case eBuildingType::shellGarden: r = {0, 0, 0, 0, 0}; return true; // BUILD_SHELL_GARDEN
    case eBuildingType::baths: r = {0, 0, 0, 0, 0}; return true; // BUILD_BATHS
    case eBuildingType::birdBath: r = {0, 0, 0, 0, 0}; return true; // BUILD_BIRD_BATH
    case eBuildingType::hippodromePiece: r = {0, 0, 0, 0, 0}; return true; // BUILD_RACE_TRACK
    case eBuildingType::crosswalk: r = {0, 0, 0, 0, 0}; return true; // BUILD_CROSSWALK
    case eBuildingType::bibliotheke: r = {0, 0, 0, 0, 0}; return true; // BUILD_BIBLIOTHEKE
    case eBuildingType::observatory: r = {8, 12, 20, 20, 20}; return true; // BUILD_OBSERVATORY
    case eBuildingType::university: r = {6, 8, 12, 15, 18}; return true; // BUILD_UNIVERSITY
    case eBuildingType::laboratory: r = {0, 0, 0, 0, 0}; return true; // BUILD_LABORATORY
    case eBuildingType::inventorsWorkshop: r = {0, 0, 0, 0, 0}; return true; // BUILD_INVENTORS_WORKSHOP
    case eBuildingType::museum: r = {8, 10, 15, 18, 20}; return true; // BUILD_MUSUEM
    case eBuildingType::refinery: r = {6, 12, 20, 20, 20}; return true; // BUILD_ORICHALC_REFINERY
    case eBuildingType::chariotFactory: r = {0, 8, 10, 10, 10}; return true; // BUILD_CHARIOT_WORKSHOP
    case eBuildingType::blackMarbleWorkshop: r = {10, 12, 20, 20, 25}; return true; // BUILD_BLACK_MARBLE_SHOP
    case eBuildingType::orangeTendersLodge: r = {0, 0, 0, 0, 0}; return true; // BUILD_FRUIT_GROWERS_LODGE
    case eBuildingType::chariotVendor: r = {0, 0, 0, 0, 0}; return true; // BUILD_CHARIOT_VENDOR
    case eBuildingType::corral: r = {0, 0, 5, 5, 5}; return true; // BUILD_CORRAL_SLAUGHTER
    case eBuildingType::orangeTree: r = {0, 0, 0, 0, 0}; return true; // BUILD_FRUIT_TREE
    case eBuildingType::cattle: r = {0, 0, 0, 0, 0}; return true; // BUILD_CATTLE
    default: return false;
    }
}

int fireRiskBase(const eBuildingType type) {
    switch(type) {
    case eBuildingType::commonHouse:
        return 8;
    case eBuildingType::eliteHousing:
        return 8;

    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
    case eBuildingType::wheatFarm:
        return 5;
    case eBuildingType::cardingShed:
        return 5;
    case eBuildingType::dairy:
        return 5;
    case eBuildingType::fishery:
        return 5;
    case eBuildingType::growersLodge:
        return 5;
    case eBuildingType::orangeTendersLodge:
        return 8;
    case eBuildingType::huntingLodge:
        return 8;
    case eBuildingType::corral:
        return 8;
    case eBuildingType::urchinQuay:
        return 5;

    case eBuildingType::sculptureStudio:
        return 1;
    case eBuildingType::timberMill:
        return 10;

    case eBuildingType::granary:
        return 8;
    case eBuildingType::warehouse:
        return 5;

    case eBuildingType::foodVendor:
    case eBuildingType::fleeceVendor:
    case eBuildingType::oilVendor:
    case eBuildingType::wineVendor:
    case eBuildingType::armsVendor:
    case eBuildingType::horseTrainer:
    case eBuildingType::chariotVendor:
        return 6;

    case eBuildingType::bibliotheke:
        return 5;
    case eBuildingType::inventorsWorkshop:
    case eBuildingType::laboratory:
        return 8;
    case eBuildingType::museum:
        return 4;

    case eBuildingType::hospital:
        return 5;
    case eBuildingType::maintenanceOffice:
        return 3;

    case eBuildingType::palace:
        return 3;
    case eBuildingType::taxOffice:
        return 5;

    case eBuildingType::artisansGuild:
        return 5;

    case eBuildingType::armory:
        return 8;
    case eBuildingType::triremeWharf:
        return 5;
    case eBuildingType::horseRanch:
        return 5;
    case eBuildingType::chariotFactory:
        return 6;

    default:
        return 0;
    }
}

double fireRiskMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1.0;
    case eDifficulty::mortal:
        return 1.5;
    case eDifficulty::hero:
        return 1.875;
    case eDifficulty::titan:
        return 2.25;
    case eDifficulty::olympian:
        return 2.5;
    }
    return 1;
}

int eDifficultyHelpers::fireRisk(
        const eDifficulty diff,
        const eBuildingType type) {
    std::array<int, 5> orig;
    if(originalFireRisk(type, orig)) {
        return orig[static_cast<int>(diff)];
    }
    const double mult = fireRiskMultiplier(diff);
    const int base = fireRiskBase(type);
    return std::round(mult*base);
}

double plagueRiskMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1.0;
    case eDifficulty::mortal:
        return 1.5;
    case eDifficulty::hero:
        return 1.875;
    case eDifficulty::titan:
        return 2.25;
    case eDifficulty::olympian:
        return 2.5;
    }
    return 1;
}

double eDifficultyHelpers::plagueRisk(const eDifficulty diff) {
    return plagueRiskMultiplier(diff);
}

double crimeRiskMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1.0;
    case eDifficulty::mortal:
        return 1.5;
    case eDifficulty::hero:
        return 1.875;
    case eDifficulty::titan:
        return 2.25;
    case eDifficulty::olympian:
        return 2.5;
    }
    return 1;
}

double eDifficultyHelpers::crimeRisk(const eDifficulty diff) {
    return crimeRiskMultiplier(diff);
}

int damageRiskBase(const eBuildingType type) {
    switch(type) {
    case eBuildingType::carrotsFarm:
    case eBuildingType::onionsFarm:
    case eBuildingType::wheatFarm:
        return 5;

    case eBuildingType::fishery:
        return 5;
    case eBuildingType::huntingLodge:
        return 5;
    case eBuildingType::corral:
        return 5;
    case eBuildingType::urchinQuay:
        return 5;

    case eBuildingType::foundry:
        return 10;
    case eBuildingType::masonryShop:
        return 10;
    case eBuildingType::refinery:
        return 10;
    case eBuildingType::blackMarbleWorkshop:
        return 10;
    case eBuildingType::mint:
        return 10;
    case eBuildingType::olivePress:
        return 8;
    case eBuildingType::sculptureStudio:
        return 2;
    case eBuildingType::winery:
        return 5;

    case eBuildingType::granary:
        return 8;

    case eBuildingType::fountain:
        return 8;
    case eBuildingType::hospital:
        return 5;
    case eBuildingType::watchPost:
        return 5;

    case eBuildingType::palace:
        return 5;

    case eBuildingType::college:
        return 6;
    case eBuildingType::dramaSchool:
        return 8;
    case eBuildingType::gymnasium:
        return 5;
    case eBuildingType::stadium:
        return 8;
    case eBuildingType::podium:
        return 5;
    case eBuildingType::theater:
        return 8;

    case eBuildingType::museum:
        return 8;
    case eBuildingType::university:
        return 6;
    case eBuildingType::observatory:
        return 8;

    case eBuildingType::triremeWharf:
        return 5;
    case eBuildingType::chariotFactory:
        return 6;

    default:
        return 0;
    }
}

double damageRiskMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1.0;
    case eDifficulty::mortal:
        return 1.25;
    case eDifficulty::hero:
        return 1.5;
    case eDifficulty::titan:
        return 2.0;
    case eDifficulty::olympian:
        return 2.5;
    }
    return 1;
}

int eDifficultyHelpers::damageRisk(
        const eDifficulty diff,
        const eBuildingType type) {
    std::array<int, 5> orig;
    if(originalDamageRisk(type, orig)) {
        return orig[static_cast<int>(diff)];
    }
    const double mult = damageRiskMultiplier(diff);
    const int base = damageRiskBase(type);
    return std::round(mult*base);
}

int eDifficultyHelpers::taxMultiplier(
        const eDifficulty diff,
        const eBuildingType type,
        const int level) {
    if(type == eBuildingType::commonHouse) {
        if(level < 3) {
            return 1;
        } else if(level == 3) {
            // homestead: 1 only on Titan (tax.html and the Model files;
            // Olympian is 2)
            if(diff == eDifficulty::titan) {
                return 1;
            } else {
                return 2;
            }
        } else {
            return 2;
        }
    } else if(type == eBuildingType::eliteHousing) {
        // an abandoned elite house pays no tax (Model: vacant and
        // devolved rows have tax multiplier 0)
        if(level <= 0) return 0;
        switch(diff) {
        case eDifficulty::beginner:
            return 22;
        case eDifficulty::mortal:
            return 18;
        case eDifficulty::hero:
            return 16;
        case eDifficulty::titan:
            return 14;
        case eDifficulty::olympian:
            return 12;
        }
    }
    return 0;
}

int eDifficultyHelpers::taxSentiment(
        const eDifficulty diff,
        const eTaxRate taxRate) {
    const int id = static_cast<int>(taxRate);
    switch(diff) {
    case eDifficulty::beginner: {
        const int vals[]{7, 5, 3, 2, 0, -1, -2};
        return vals[id];
    }
    case eDifficulty::mortal: {
        const int vals[]{4, 3, 1, 0, 0, -2, -4};
        return vals[id];
    }
    case eDifficulty::hero: {
        const int vals[]{3, 2, 1, 0, -1, -3, -5};
        return vals[id];
    }
    case eDifficulty::titan: {
        const int vals[]{2, 1, 0, 0, -2, -4, -6};
        return vals[id];
    }
    case eDifficulty::olympian: {
        const int vals[]{1, 1, 0, 0, -3, -5, -7};
        return vals[id];
    }
    }
    // not reached with a valid value: every case above returns
    return 0;
}

double eDifficultyHelpers::workerFrac(
        const eDifficulty diff, const eWageRate wageRate) {
    switch(wageRate) {
    case eWageRate::none:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.42;
        case eDifficulty::mortal:
            return 0.37;
        case eDifficulty::hero:
            return 0.32;
        case eDifficulty::titan:
            return 0.29;
        case eDifficulty::olympian:
            return 0.27;
        }
    }
    case eWageRate::veryLow:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.46;
        case eDifficulty::mortal:
            return 0.41;
        case eDifficulty::hero:
            return 0.36;
        case eDifficulty::titan:
            return 0.33;
        case eDifficulty::olympian:
            return 0.31;
        }
    }
    case eWageRate::low:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.49;
        case eDifficulty::mortal:
            return 0.44;
        case eDifficulty::hero:
            return 0.39;
        case eDifficulty::titan:
            return 0.36;
        case eDifficulty::olympian:
            return 0.34;
        }
    }
    case eWageRate::normal:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.52;
        case eDifficulty::mortal:
            return 0.47;
        case eDifficulty::hero:
            return 0.42;
        case eDifficulty::titan:
            return 0.39;
        case eDifficulty::olympian:
            return 0.37;
        }
    }
    case eWageRate::high:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.55;
        case eDifficulty::mortal:
            return 0.50;
        case eDifficulty::hero:
            return 0.45;
        case eDifficulty::titan:
            return 0.42;
        case eDifficulty::olympian:
            return 0.4;
        }
    }
    case eWageRate::veryHigh:
    {
        switch(diff) {
        case eDifficulty::beginner:
            return 0.57;
        case eDifficulty::mortal:
            return 0.52;
        case eDifficulty::hero:
            return 0.47;
        case eDifficulty::titan:
            return 0.44;
        case eDifficulty::olympian:
            return 0.42;
        }
    }
    }
    return 0;
}


int soliderBribeMultiplier(const eDifficulty diff) {
    switch(diff) {
    case eDifficulty::beginner:
        return 1;
    case eDifficulty::mortal:
        return 3;
    case eDifficulty::hero:
        return 4;
    case eDifficulty::titan:
        return 5;
    case eDifficulty::olympian:
        return 7;
    }
    return 1;
}

int eDifficultyHelpers::soliderBribe(const eDifficulty diff,
                                     const eCharacterType type) {
    const int mult = soliderBribeMultiplier(diff);
    int base = 0;
    switch(type) {
    case eCharacterType::rockThrower:
        base = 25;
        break;
    case eCharacterType::hoplite:
        base = 25;
        break;
    case eCharacterType::horseman:
        base = 50;
        break;
    case eCharacterType::enemyWarship:
        // 800 drachmas a warship on Hero (Citadel of Poseidon, bribes)
        base = 200;
        break;
    default:
        return 0;
    }
    return mult*base;
}
