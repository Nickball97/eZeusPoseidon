#ifndef EHUNTINGLODGE_H
#define EHUNTINGLODGE_H

#include "eresourcecollectbuildingbase.h"

class eHunter;

class eHuntingLodge : public eResourceCollectBuildingBase {
public:
    eHuntingLodge(eGameBoard& board, const eCityId cid);
    ~eHuntingLodge();

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    void timeChanged(const int by) override;

    // a blessing gives one bigger delivery (see useBlessing)
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }
    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    void addRaw() override {}
private:
    bool spawn();

    stdptr<eHunter> mHunter;
    double mSpawnTime = 1000000;
};

#endif // EHUNTINGLODGE_H
