#ifndef ERESOURCEEVENTVALUE_H
#define ERESOURCEEVENTVALUE_H

#include "engine/eresourcetype.h"

class eWriteStream;
class eReadStream;

class eResourceEventValue {
public:
    eResourceType resourceType() const { return mResource; }

    const std::vector<eResourceType>& resourceTypes() const
    { return mResources; }
    eResourceType resourceType(const int id) const
    { return mResources[id]; }
    void setResourceType(const int id, const eResourceType type)
    { mResources[id] = type; }

    // Consult Oracle: picks the resource now; the event then keeps it
    void oracleChooseType();

    void write(eWriteStream& dst) const;
    void read(eReadStream& src);
protected:
    void chooseType();

    virtual void longNameReplaceResource(
            const std::string& id,
            std::string& tmpl) const;

    std::vector<eResourceType> mResources = {eResourceType::fleece,
                                             eResourceType::none,
                                             eResourceType::none};

    eResourceType mResource = eResourceType::fleece;
    bool mOracleType = false;
};

#endif // ERESOURCEEVENTVALUE_H
