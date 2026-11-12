#ifndef EFARMBASE_H
#define EFARMBASE_H

#include "eresourcebuildingbase.h"

#include "textures/ebuildingtextures.h"
#include "engine/edate.h"

class eFarmBase : public eResourceBuildingBase {
public:
    eFarmBase(eGameBoard& board,
              const eBuildingType type,
              const int sw, const int sh,
              const eResourceType resType,
              const eCityId cid);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    void timeChanged(const int by) override;
    // a blessing lasts until used and multiplies the next output once
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }

    // for the info window: how far the crop has grown towards a full
    // harvest (0-1), and the month it is brought in
    double growth() const;
    eMonth harvestMonth() const;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    const std::vector<eBuildingTextures>& mTextures;
    double mNextRipe = 0;
    int mRipe = 0; // 0 - 5
};

#endif // EFARMBASE_H
