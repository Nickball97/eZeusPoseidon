#ifndef ESICKDISGRUNTLEDACTION_H
#define ESICKDISGRUNTLEDACTION_H

#include "eactionwithcomeback.h"

class eBuilding;

class eSickDisgruntledAction : public eActionWithComeback {
public:
    eSickDisgruntledAction(eCharacter* const c,
                           eBuilding* const ch);
    // also used for an elite house's nobleman (eEliteHousing)

    bool decide() override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
protected:
    void patrol();
    void goBackDecision(const stdsptr<eWalkableObject>& w =
                            eWalkableObject::sCreateRoadAvenue());
private:
    eBuilding* mBuilding = nullptr;

    bool mGoBackNext = false;
};

#endif // ESICKDISGRUNTLEDACTION_H
