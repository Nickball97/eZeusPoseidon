#include "ezeussanctuary.h"

eZeusSanctuary::eZeusSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeZeus, sw, sh, 75, cid) {}

eAphroditeSanctuary::eAphroditeSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeAphrodite, sw, sh, 50, cid) {}

eApolloSanctuary::eApolloSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeApollo, sw, sh, 60, cid) {}

eAtlasSanctuary::eAtlasSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeAtlas, sw, sh, 55, cid) {}

eAthenaSanctuary::eAthenaSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeAthena, sw, sh, 60, cid) {}

eDemeterSanctuary::eDemeterSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeDemeter, sw, sh, 60, cid) {}

eDionysusSanctuary::eDionysusSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeDionysus, sw, sh, 45, cid) {}

eHadesSanctuary::eHadesSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeHades, sw, sh, 75, cid) {}

eHeraSanctuary::eHeraSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeHera, sw, sh, 65, cid) {}

eHermesSanctuary::eHermesSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templeHermes, sw, sh, 45, cid) {}

ePoseidonSanctuary::ePoseidonSanctuary(
        const int sw, const int sh, eGameBoard& board, const eCityId cid) :
    eSanctuary(board, eBuildingType::templePoseidon, sw, sh, 75, cid) {}
