#include "ehasresourceobject.h"

#include "ehasnonbusyresourceobject.h"

stdsptr<eHasResourceObject> eHasResourceObject::sCreate(
        const eHasResourceObjectType type) {
    switch(type) {
    case eHasResourceObjectType::nonBusy:
        return std::make_shared<eHasNonBusyResourceObject>();
    case eHasResourceObjectType::fish:
    case eHasResourceObjectType::urchin:
    case eHasResourceObjectType::silver:
    case eHasResourceObjectType::copper:
    case eHasResourceObjectType::forest:
    case eHasResourceObjectType::marble:
    case eHasResourceObjectType::orichalc:
    case eHasResourceObjectType::blackMarble:
        return std::make_shared<eHasResourceObject>(type);
    }
    // not reached with a valid value: every case above returns
    return std::make_shared<eHasNonBusyResourceObject>();
}

stdsptr<eHasResourceObject> eHasResourceObject::sCreateNonBusy(
        const stdsptr<eHasResourceObject>& other) {
    return std::make_shared<eHasNonBusyResourceObject>(other);
}
