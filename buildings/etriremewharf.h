#ifndef ETRIREMEWHARF_H
#define ETRIREMEWHARF_H

#include "eresourcebuildingbase.h"

class eTrireme;

class eTriremeWharf : public eEmployingBuilding {
public:
    eTriremeWharf(eGameBoard& board,
                  const eDiagonalOrientation o,
                  const eCityId cid);
    ~eTriremeWharf();

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    void timeChanged(const int by) override;

    int add(const eResourceType type, const int count) override;

    int count(const eResourceType type) const override;
    int spaceLeft(const eResourceType type) const override;

    std::vector<eCartTask> cartTasks() const override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    eDiagonalOrientation orientation() const { return mO; }

    void triremeCameBack();
    void triremeLeaving();
    bool abroad() const { return mAbroad; }
    bool hasTrireme() const;
    bool isAtWharf() const;

    eTile* triremeTile() const;
    // an Atlantean frigate still waiting for its Atlantean Fire launcher
    bool wantsOrichalc() const;
private:
    void spawnTrireme();

    const eDiagonalOrientation mO;

    bool mAbroad = false;

    stdptr<eCartTransporter> mTakeCart;
    stdptr<eTrireme> mTrireme;

    int mTriremeBuildingStage = 0;
    double mTriremeBuildingTime = 0;

    // "the wharf tries to hold 3 loads of both armor and wood in stock"
    // (Citadel of Poseidon)
    const int mMaxWood = 3;
    int mWoodCount{0};
    const int mMaxArmor = 3;
    int mArmorCount{0};
    // Atlantean cities: one load of orichalc fits the frigate with an
    // Atlantean Fire launcher (Poseidon manual, Atlantean Navy)
    int mOrichalcCount{0};
};

#endif // ETRIREMEWHARF_H
