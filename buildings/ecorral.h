#ifndef ECORRAL_H
#define ECORRAL_H

#include "eresourcebuildingbase.h"
#include "characters/echaracter.h"

class eCorral : public eResourceBuildingBase {
public:
    eCorral(eGameBoard& board, const eCityId cid);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    std::vector<eOverlay> getOverlays(const eTileSize size) const override;

    // a blessing gives one bigger delivery (see useBlessing)
    bool blessingLastsUntilUsed() const override { return true; }
    bool blessingBoostsSpeed() const override { return false; }
    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    void timeChanged(const int by) override;

    int cattleCount() const { return mNCattle; }
    void addCattle();
    bool noCattle() const { return mNoCattle; }
    void setNoCattle(const bool c);
private:
    bool killCattle();

    bool takeCattle();
    void replaceCattle();

    bool mNoCattle = true;
    double mProcessing = 0;
    int mNCattle = 0;
    double mTakeWait = 0;
    double mKillWait = 0;
    double mReplaceWait = 0;
    // cattle fetchers out (not saved; after loading only new ones count)
    std::vector<stdptr<eCharacter>> mFetchers;
};

#endif // ECORRAL_H
