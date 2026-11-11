#ifndef ERESOURCEGRANTEDEVENTBASE_H
#define ERESOURCEGRANTEDEVENTBASE_H

#include "egameevent.h"
#include "ecityeventvalue.h"
#include "eresourceeventvalue.h"
#include "ecounteventvalue.h"

#include "engine/eevent.h"

class eResourceGrantedEventBase : public eGameEvent,
                                  public eCityEventValue,
                                  public eResourceEventValue,
                                  public eCountEventValue {
public:
    eResourceGrantedEventBase(const eCityId cid,
                              const eEvent giftCashAccepted,
                              const eEvent giftAccepted,
                              const eEvent giftPostponed,
                              const eEvent giftRefused,
                              const eEvent giftForfeited,
                              const eEvent giftGranted,
                              const eEvent giftLastChance,
                              const eEvent giftInsufficientSpace,
                              const eEvent giftPartialSpace,
                              const eGameEventType type,
                              const eGameEventBranch branch,
                              eGameBoard& board);

    void initialize(const bool postpone,
                    const eResourceType res,
                    const int count,
                    const stdsptr<eWorldCity>& c);

    void trigger() override;

    void write(eWriteStream& dst) const override;
    void read(eReadStream& src) override;
protected:
    // lets a subclass pass its own settings on to a postponed offer
    virtual void setupPostponed(eResourceGrantedEventBase* const e) const { (void)e; }
    // called when the player takes drachmas from this offer
    virtual void drachmasAccepted(const ePlayerId pid) { (void)pid; }
    void setEvents(const eEvent giftCashAccepted,
                   const eEvent giftAccepted,
                   const eEvent giftPostponed,
                   const eEvent giftRefused,
                   const eEvent giftForfeited,
                   const eEvent giftGranted,
                   const eEvent giftLastChance,
                   const eEvent giftInsufficientSpace,
                   const eEvent giftPartialSpace);

    bool mPostpone = true;
private:
    eEvent mGiftCashAccepted;
    eEvent mGiftAccepted;
    eEvent mGiftPostponed;
    eEvent mGiftRefused;
    eEvent mGiftForfeited;
    eEvent mGiftGranted;
    eEvent mGiftLastChance;
    eEvent mGiftInsufficientSpace;
    eEvent mGiftPartialSpace;
};

#endif // ERESOURCEGRANTEDEVENTBASE_H
