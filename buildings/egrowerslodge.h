#ifndef EGROWERSLODGE_H
#define EGROWERSLODGE_H

#include "eemployingbuilding.h"

#include "textures/ebuildingtextures.h"
#include "characters/egrower.h"

class eCartTransporter;

class eGrowersLodge : public eEmployingBuilding {
public:
    eGrowersLodge(eGameBoard& board, const eGrowerType type,
                  const eCityId cid);
    ~eGrowersLodge();

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    void timeChanged(const int by) override;

    int add(const eResourceType type, const int count) override;
    int take(const eResourceType type, const int count) override;

    int count(const eResourceType type) const override;
    int spaceLeft(const eResourceType type) const override;

    std::vector<eCartTask> cartTasks() const override;

    // a blessing gives one bigger delivery (see useBlessing)
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }
    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    using eGrowerPtr = stdptr<eGrower> eGrowersLodge::*;
    bool spawnGrower(const eGrowerPtr grower);

    void setNoTarget(const bool t);
    bool noTarget() const { return mNoTarget; }
private:
    // "the growers' lodge can only have 5 loads of both in stock"
    // (Citadel of Poseidon)
    const int mMaxResource = 5;

    const eGrowerType mType;

    bool mNoTarget = false;

    bool mSpawnEnabled = true;

    int mGrapes{0};
    int mOlives{0};
    int mOranges{0};

    stdptr<eCartTransporter> mCart;

    double mSpawnTime = 1000000;
    double mHarvestSpawnTime = 0;

    stdptr<eGrower> mGrower;
    // "The growers' lodge sends out 3 harvesters in the harvest time"
    // (Citadel of Poseidon): two more join the tender in season
    std::vector<stdptr<eGrower>> mHarvesters;
    int harvesterCount();
    bool harvestSeason() const;
    eGrower* spawnGrowerChar();
};

#endif // EGROWERSLODGE_H
