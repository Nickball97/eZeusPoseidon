#include "ebuildingreader.h"
#include "buildings/sanctuaries/etempleobeliskbuilding.h"
#include "buildings/sanctuaries/etemplegrandstatuebuilding.h"

#include "buildings/allbuildings.h"
#include "buildings/ebuildingregistry.h"
#include "engine/egameboard.h"

template <typename T>
stdsptr<eBuilding> createVendor(eGameBoard& board,
                                eReadStream& src,
                                const eCityId cid) {
    const auto v = e::make_shared<T>(board, cid);
    int aid;
    src >> aid;
    int sid;
    src >> sid;
    src.addPostFunc([aid, sid, v]() {
        const auto& board = v->getBoard();
        const auto a = board.buildingWithIOID(aid);
        const auto aa = a->template ref<eAgoraBase>();
        v->setAgora(aa);
        aa->setBuilding(sid, v);
    });
    return v;
}

void readSanctBuildingMonument(
        eGameBoard& board, eReadStream& src,
        const stdsptr<eSanctBuilding>& ts) {
    src.readBuilding(&board, [ts](eBuilding* const bb) {
        const auto ss = static_cast<eMonument*>(bb);
        ts->setMonument(ss);
        ss->registerElement(ts);
    });
}

std::vector<eSanctCost> readPyramidElementCost(eReadStream& src) {
    int n;
    src >> n;
    std::vector<eSanctCost> result;
    for(int i = 0; i < n; i++) {
        auto& r = result.emplace_back();
        r.read(src);
    }
    return result;
}

stdsptr<eBuilding> eBuildingReader::sRead(
        eGameBoard& board, const eBuildingType type,
        eReadStream& src) {
    const auto& wrld = board.world();
    eCityId cid;
    src >> cid;
    stdsptr<eBuilding> b;
    switch(type) {
    case eBuildingType::commonAgora: {
        eAgoraOrientation o;
        src >> o;
        b = e::make_shared<eCommonAgora>(o, board, cid);
    } break;
    case eBuildingType::grandAgora: {
        eAgoraOrientation o;
        src >> o;
        b = e::make_shared<eGrandAgora>(o, board, cid);
    } break;
    case eBuildingType::agoraSpace:
        return nullptr;
    case eBuildingType::stadium: {
        bool rotated;
        src >> rotated;
        b = e::make_shared<eStadium>(board, rotated, cid);
    } break;
    case eBuildingType::palace: {
        bool rotated;
        src >> rotated;
        b = e::make_shared<ePalace>(board, rotated, cid);
    } break;
    case eBuildingType::palaceTile: {
        bool other;
        src >> other;
        const auto pt = e::make_shared<ePalaceTile>(board, other, cid);
        b = pt;
        src.readBuilding(&board, [pt](eBuilding* const bb) {
             pt->setPalace(static_cast<ePalace*>(bb));
        });
    } break;

    case eBuildingType::urchinQuay: {
        eDiagonalOrientation o;
        src >> o;
        b = e::make_shared<eUrchinQuay>(board, o, cid);
    } break;
    case eBuildingType::fishery: {
        eDiagonalOrientation o;
        src >> o;
        b = e::make_shared<eFishery>(board, o, cid);
    } break;
    case eBuildingType::triremeWharf: {
        eDiagonalOrientation o;
        src >> o;
        b = e::make_shared<eTriremeWharf>(board, o, cid);
    } break;

    case eBuildingType::pier: {
        eDiagonalOrientation o;
        src >> o;
        const auto p = e::make_shared<ePier>(board, o, cid);
        b = p;
        src.readBuilding(&board, [p](eBuilding* const bb) {
             p->setTradePost(bb);
        });
    } break;

    case eBuildingType::tradePost: {
        eTradePostType tpt;
        src >> tpt;
        eDiagonalOrientation o;
        src >> o;

        eCityId ctid;
        src >> ctid;
        const auto ct = wrld.cityWithId(ctid);

        const auto tp = e::make_shared<eTradePost>(board, *ct, cid, tpt);
        b = tp;
        tp->setOrientation(o);

        if(tpt == eTradePostType::pier) {
            src.readBuilding(&board, [tp](eBuilding* const bb) {
                 tp->setUnpackBuilding(bb);
            });
        }
    } break;

    case eBuildingType::sheep: {
        b = e::make_shared<eAnimalBuilding>(
                board, nullptr, eBuildingType::sheep, cid);
    } break;
    case eBuildingType::goat: {
        b = e::make_shared<eAnimalBuilding>(
                board, nullptr, eBuildingType::goat, cid);
    } break;
    case eBuildingType::cattle: {
        b = e::make_shared<eAnimalBuilding>(
                board, nullptr, eBuildingType::cattle, cid);
    } break;

    case eBuildingType::gatehouse: {
        bool rotated;
        src >> rotated;
        b = e::make_shared<eGatehouse>(board, rotated, cid);
    } break;

    case eBuildingType::horseRanch: {
        const auto hr = e::make_shared<eHorseRanch>(board, cid);
        b = hr;

        src.readBuilding(&board, [hr](eBuilding* const bb) {
             hr->setEnclosure(static_cast<eHorseRanchEnclosure*>(bb));
        });
    } break;
    case eBuildingType::horseRanchEnclosure: {
        const auto hre = e::make_shared<eHorseRanchEnclosure>(board, cid);
        b = hre;
        src.readBuilding(&board, [hre](eBuilding* const bb) {
            hre->setRanch(static_cast<eHorseRanch*>(bb));
        });
    } break;

    case eBuildingType::foodVendor: {
        b = createVendor<eFoodVendor>(board, src, cid);
    } break;
    case eBuildingType::fleeceVendor: {
        b = createVendor<eFleeceVendor>(board, src, cid);
    } break;
    case eBuildingType::oilVendor: {
        b = createVendor<eOilVendor>(board, src, cid);
    } break;
    case eBuildingType::wineVendor: {
        b = createVendor<eWineVendor>(board, src, cid);
    } break;
    case eBuildingType::armsVendor: {
        b = createVendor<eArmsVendor>(board, src, cid);
    } break;
    case eBuildingType::horseTrainer: {
        b = createVendor<eHorseVendor>(board, src, cid);
    } break;
    case eBuildingType::chariotVendor: {
        b = createVendor<eChariotVendor>(board, src, cid);
    } break;

    case eBuildingType::boulevard: {
        b = e::make_shared<eAvenue>(board, cid, eBuildingType::boulevard);
    } break;

    case eBuildingType::commemorative: {
        int id;
        src >> id;
        b = e::make_shared<eCommemorative>(id, board, cid);
    } break;

    case eBuildingType::godMonument: {
        eGodType type;
        src >> type;
        eGodQuestId id;
        src >> id;
        b = e::make_shared<eGodMonument>(type, id, board, cid);
    } break;
    case eBuildingType::godMonumentTile: {
        const auto pt = e::make_shared<eGodMonumentTile>(board, cid);
        b = pt;
        src.readBuilding(&board, [pt](eBuilding* const bb) {
             pt->setMonument(static_cast<eGodMonument*>(bb));
        });
    } break;

    case eBuildingType::waterPark: {
        int id;
        src >> id;
        const auto wp = e::make_shared<eWaterPark>(board, cid);
        b = wp;
        wp->setId(id);
    } break;

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
    case eBuildingType::templeZeus: {
        int sw;
        src >> sw;
        int sh;
        src >> sh;

        b = eSanctuary::sCreate(type, sw, sh, board, cid);
    } break;
    case eBuildingType::templeStatue: {
        eGodType godType;
        src >> godType;
        int id;
        src >> id;
        const auto ts = e::make_shared<eTempleStatueBuilding>(godType, id, board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::templeMonument: {
        eGodType godType;
        src >> godType;
        int id;
        src >> id;
        const auto ts = e::make_shared<eTempleMonumentBuilding>(godType, id, board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::templeAltar: {
        const auto ts = e::make_shared<eTempleAltarBuilding>(board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::templeGrandStatue: {
        const auto ts = e::make_shared<eTempleGrandStatueBuilding>(board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::templeObelisk: {
        int id;
        src >> id;
        const auto ts = e::make_shared<eTempleObeliskBuilding>(id, board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::temple: {
        const auto ts = e::make_shared<eTempleBuilding>(board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::templeTile: {
        int id;
        src >> id;
        const auto ts = e::make_shared<eTempleTileBuilding>(id, board, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;

    case eBuildingType::modestPyramid:
    case eBuildingType::pyramid:
    case eBuildingType::greatPyramid:
    case eBuildingType::majesticPyramid:

    case eBuildingType::smallMonumentToTheSky:
    case eBuildingType::monumentToTheSky:
    case eBuildingType::grandMonumentToTheSky:

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

    case eBuildingType::pyramidOfThePantheon:
    case eBuildingType::altarOfOlympus:
    case eBuildingType::templeOfOlympus:
    case eBuildingType::observatoryKosmika:
    case eBuildingType::museumAtlantika: {
        int sw;
        int sh;
        ePyramid::sDimensions(type, sw, sh);
        b = e::make_shared<ePyramid>(board, type, sw, sh, cid);
    } break;
    case eBuildingType::pyramidWall: {
        eOrientation o;
        src >> o;
        int elevation;
        src >> elevation;
        int special;
        src >> special;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidWall>(
                            costs, board, o, elevation, special, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidTop: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidTop>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidTile: {
        int elevation;
        src >> elevation;
        int type;
        src >> type;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidTile>(
                            costs, board, elevation, type, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidPart: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidBuildingPart>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidAltar: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidAltar>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidStatue: {
        int elevation;
        src >> elevation;
        eGodType type;
        src >> type;
        int id;
        src >> id;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidStatue>(
                            costs, board, elevation, type, id, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidMonument: {
        int elevation;
        src >> elevation;
        eGodType type;
        src >> type;
        int id;
        src >> id;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidMonument>(
                            costs, board, elevation, type, id, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidTemple: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidTemple>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidObservatory: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidObservatory>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;
    case eBuildingType::pyramidMuseum: {
        int elevation;
        src >> elevation;
        const auto costs = readPyramidElementCost(src);
        const auto ts = e::make_shared<ePyramidMuseum>(
                            costs, board, elevation, cid);
        b = ts;
        readSanctBuildingMonument(board, src, ts);
    } break;

    case eBuildingType::achillesHall:
    case eBuildingType::atalantaHall:
    case eBuildingType::bellerophonHall:
    case eBuildingType::herculesHall:
    case eBuildingType::jasonHall:
    case eBuildingType::odysseusHall:
    case eBuildingType::perseusHall:
    case eBuildingType::theseusHall: {
        const auto hero = eHerosHall::sHallTypeToHeroType(type);
        b = e::make_shared<eHerosHall>(hero, board, cid);
    } break;

    case eBuildingType::none:
    case eBuildingType::erase:
    case eBuildingType::bridge:
    case eBuildingType::crosswalk:
        return nullptr;
    default:
        // every plain kind of building comes from the table of building
        // kinds (buildings/ebuildingregistry.cpp)
        b = eBuildingRegistry::sCreate(type, board, cid);
        break;
    }
    if(b) b->read(src);
    return b;
}
