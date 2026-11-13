#include "eblackmarbleworkshop.h"

eBlackMarbleWorkshop::eBlackMarbleWorkshop(eGameBoard& board, const eCityId cid) :
    eMasonryShopBase(board,
                     &eBuildingTextures::fBlackMarbleWorkshop,
                     &eBuildingTextures::fBlackMarbleWorkshopStones,
                     eBuildingType::blackMarbleWorkshop,
                     eHasResourceObjectType::blackMarble,
                     eResourceType::blackMarble,
                     eTileActionType::blackMasonry,
                     cid) {}
