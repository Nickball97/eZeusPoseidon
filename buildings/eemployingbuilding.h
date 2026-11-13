#ifndef EEMPLOYINGBUILDING_H
#define EEMPLOYINGBUILDING_H

#include "ebuildingwithresource.h"

class eEmployingBuilding : public eBuildingWithResource {
public:
    eEmployingBuilding(eGameBoard& board,
                       const eBuildingType type,
                       const int sw, const int sh,
                       const int maxEmployees,
                       const eCityId cid);
    ~eEmployingBuilding();

    int employed() const { return mEmployed; }
    void setEmployed(const int e);
    void setMaxEmployees(const int m) { mMaxEmployees = m; }
    int maxEmployees() const { return mMaxEmployees; }
    double vacanciesFilledFraction() const;
    double effectiveness() const;
    // speed-up from a god's blessing for this building type
    static double sBlessMultiplier(const eBuildingType type);
    // false: a blessing does not speed the building up; instead its next
    // output is multiplied once (see useBlessing)
    virtual bool blessingBoostsSpeed() const { return true; }
    // normal output, multiplied if blessed; the blessing is then used up
    int useBlessing(const int normal);
    bool shutDown() const { return mShutDown; }
    void setShutDown(const bool sd);

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    bool mShutDown = false;
    int mMaxEmployees;
    int mEmployed = 0;
};

#endif // EEMPLOYINGBUILDING_H
