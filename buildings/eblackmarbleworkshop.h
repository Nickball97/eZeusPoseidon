#ifndef EBLACKMARBLEWORKSHOP_H
#define EBLACKMARBLEWORKSHOP_H

#include "emasonryshop.h"

// a masonry shop for black marble (see eMasonryShopBase)
class eBlackMarbleWorkshop : public eMasonryShopBase {
public:
    eBlackMarbleWorkshop(eGameBoard& board, const eCityId cid);
};

#endif // EBLACKMARBLEWORKSHOP_H
