#ifndef ESMALLHOUSE_H
#define ESMALLHOUSE_H

#include "ehousebase.h"

class eSick;
class eDisgruntled;

class eSmallHouse : public eHouseBase {
public:
    eSmallHouse(eGameBoard& board, const eCityId cid);
    ~eSmallHouse();

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;

    int provide(const eProvide p, const int n) override;

    void timeChanged(const int by) override;

    void nextMonth() override;

    int water() const { return mWater; }
    int hygiene() const { return mHygiene; }

    void setPlague(const bool p);
    bool plague() const { return mPlague; }

    void setDisgruntled(const bool d);
    bool disgruntled() const { return mDisgruntled; }

    // Original crime (exe 0x00566F20, 0x004A5F70, 0x005BFDE0): 0-100.
    int crime() const { return mCrime; }
    // twice a month: + crime increment + (40 - mood)/2 (or -20 with
    // Dionysus), never below the level's crime base nor above 100
    void crimeUpdate(const int mood, const bool active, const bool dionysus);
    // a watchman passing lowers it by the guardpost's risk reducer (50)
    void reduceCrime(const int by);

    bool lowFood() const;
    bool lowFleece() const { return mFleece < 2; }
    bool lowOil() const { return mOil < 2; }

    int foodSatisfaction() const { return mFoodSatisfaction; }
    int waterSatisfaction() const { return mWaterSatisfaction; }
    int workSatisfaction() const { return mWorkSatisfaction; }
    int taxSatisfaction() const { return mTaxSatisfaction; }
    int satisfaction() const { return mSatisfaction; }

    eHouseMissing missing() const override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    static std::string sName(const int level);
private:
    void updateLevel(const bool timed = false);
    void stepToward(const int target, const bool timed);
    bool appealFor(const int l) const;
    bool cultureFor(const int l) const;
    void updateSatisfaction();

    void spawnCharacter(const stdsptr<eCharacter>& c);
    void spawnSick();
    void spawnOutlaw(const eCharacterType type);

    bool mSatisfactionProvidedThisMonth = false;

    int mUpdateSatisfaction = 0;
    int mSatisfaction = 100;
    int mFoodSatisfaction = 100;
    int mWaterSatisfaction = 100;
    int mWorkSatisfaction = 100;
    int mTaxSatisfaction = 100;

    int mUpdateWater = 0;
    int mUpdateHygiene = 8;
    int mUpdateLevel = 0;

    int mWater = 0;
    int mHygiene = 100; // 0-100
    bool mPlague = false;
    bool mDisgruntled = false;

    int mSpawnSick = 0;
    stdptr<eSick> mSick;
    int mSpawnDisg = 0;
    stdptr<eDisgruntled> mDisg;

    int mCrime = 0;
    double mOutlawCycle = 0;
    int mFailedChecks = 0; // level checks failed in a row
};

#endif // ESMALLHOUSE_H
