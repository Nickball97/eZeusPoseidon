#ifndef EPROCESSINGBUILDING_H
#define EPROCESSINGBUILDING_H

#include "eresourcebuildingbase.h"
#include "textures/ebuildingtextures.h"

class eCartTransporter;

class eProcessingBuilding : public eResourceBuildingBase {
public:
    using eBaseTex = std::shared_ptr<eTexture> eBuildingTextures::*;
    using eOverlays = eTextureCollection eBuildingTextures::*;
    eProcessingBuilding(eGameBoard& board,
                        const eBaseTex baseTex,
                        const double overlayX,
                        const double overlayY,
                        const eOverlays overlays,
                        const eBuildingType type,
                        const int sw, const int sh,
                        const int maxEmployees,
                        const eResourceType rawMaterial,
                        const eResourceType product,
                        const int rawUse,
                        const int time,
                        const eCityId cid);
    ~eProcessingBuilding();

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    void timeChanged(const int by) override;
    // a blessing lasts until used and multiplies the next output once
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }

    int add(const eResourceType type, const int count) override;

    int count(const eResourceType type) const override;
    int spaceLeft(const eResourceType type) const override;

    std::vector<eCartTask> cartTasks() const override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    int rawCount() const { return mRawCount; }
    // for the info window: progress of the current batch (0-1)
    double progress() const;
    // raw material it accepts (original exe 0x0048A880)
    void setMaxRaw(const int m) { mMaxRaw = m; }
private:
    const std::vector<eBuildingTextures>& mTextures;

    const eBaseTex mBaseTex;
    const eOverlays mOverlays;
    const double mOverlayX;
    const double mOverlayY;

    const eResourceType mRawMaterial;

    stdptr<eCartTransporter> mTakeCart;

    int mMaxRaw = 4;
    const int mRawUse;
    int mRawCount{0};

    const int mProcessWaitTime;
    double mProcessTime = 0;
};

#endif // EPROCESSINGBUILDING_H
