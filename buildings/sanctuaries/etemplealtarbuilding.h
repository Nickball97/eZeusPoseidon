#ifndef ETEMPLEALTARBUILDING_H
#define ETEMPLEALTARBUILDING_H

#include "esanctbuilding.h"

enum class eSacrifice {
    none,
    sheep,
    goods,
    bull
};

class eTempleAltarBuilding : public eSanctBuilding {
public:
    eTempleAltarBuilding(eGameBoard& board, const eCityId cid);

    std::shared_ptr<eTexture>
    getTexture(const eTileSize size) const override;

    std::vector<eOverlay>
    getOverlays(const eTileSize size) const override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    // the kind of the last sacrifice; the sanctuary priests set it and
    // the sanctuary says whether it is still being shown (eSanctuary)
    void setSacrifice(const eSacrifice s) { mSacrifice = s; }
    eSacrifice sacrifice() const { return mSacrifice; }
    bool sacrificing() const;
private:
    eSacrifice mSacrifice = eSacrifice::none;
    int mSacrificeTime = 600000; // not used any more, kept in saves
};

#endif // ETEMPLEALTARBUILDING_H
