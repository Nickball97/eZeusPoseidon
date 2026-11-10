#ifndef EGIFTFROMEVENT_H
#define EGIFTFROMEVENT_H

#include "eresourcegrantedeventbase.h"

class eGiftFromEvent : public eResourceGrantedEventBase {
public:
    eGiftFromEvent(const eCityId cid,
                   const eGameEventBranch branch,
                   eGameBoard& board);

    std::string longName() const override;

    // Debt relief from an ally or the parent city: the same offer as a
    // cash gift, shown with the original's own "A helping hand from..."
    // messages (eventmsg "rescue_cash", offer type 0x17 in zeus.exe)
    void setRescue(const bool r);
    bool rescue() const { return mRescue; }

    void write(eWriteStream& dst) const override;
    void read(eReadStream& src) override;
protected:
    void setupPostponed(eResourceGrantedEventBase* const e) const override;
private:
    bool mRescue = false;
};

#endif // EGIFTFROMEVENT_H
