#include "egiftfromevent.h"

#include "engine/egameboard.h"
#include "elanguage.h"
#include "engine/eeventdata.h"
#include "engine/eevent.h"
#include "fileIO/efileformat.h"

eGiftFromEvent::eGiftFromEvent(const eCityId cid,
                               const eGameEventBranch branch,
                               eGameBoard& board) :
    eResourceGrantedEventBase(
        cid,
        eEvent::giftCashAccepted,
        eEvent::giftAccepted,
        eEvent::giftPostponed,
        eEvent::giftRefused,
        eEvent::giftForfeited,
        eEvent::giftGranted,
        eEvent::giftLastChance,
        eEvent::giftInsufficientSpace,
        eEvent::giftPartialSpace,
        eGameEventType::giftFrom, branch, board) {}

std::string eGiftFromEvent::longName() const {
    auto tmpl = eLanguage::text("gift_of_from_long_name");
    longNameReplaceResource("%1", tmpl);
    return tmpl;
}

void eGiftFromEvent::setRescue(const bool r) {
    mRescue = r;
    if(r) {
        setEvents(eEvent::rescueCashAccepted,
                  eEvent::rescueAccepted,
                  eEvent::rescuePostponed,
                  eEvent::rescueRefused,
                  eEvent::rescueForfeited,
                  eEvent::rescueGranted,
                  eEvent::rescueLastChance,
                  eEvent::rescueInsufficientSpace,
                  eEvent::rescuePartialSpace);
    } else {
        setEvents(eEvent::giftCashAccepted,
                  eEvent::giftAccepted,
                  eEvent::giftPostponed,
                  eEvent::giftRefused,
                  eEvent::giftForfeited,
                  eEvent::giftGranted,
                  eEvent::giftLastChance,
                  eEvent::giftInsufficientSpace,
                  eEvent::giftPartialSpace);
    }
}

void eGiftFromEvent::setupPostponed(eResourceGrantedEventBase* const e) const {
    if(const auto g = dynamic_cast<eGiftFromEvent*>(e)) {
        g->setRescue(mRescue);
    }
}

void eGiftFromEvent::write(eWriteStream& dst) const {
    eResourceGrantedEventBase::write(dst);
    dst << mRescue;
}

void eGiftFromEvent::read(eReadStream& src) {
    eResourceGrantedEventBase::read(src);
    if(src.formatVersion() >= eFileFormat::giftRescue) {
        bool r;
        src >> r;
        setRescue(r);
    }
}

// zeus.exe (0x00474A80): taking rescue cash lowers the giving city's
// attitude by 10
void eGiftFromEvent::drachmasAccepted(const ePlayerId pid) {
    if(!mRescue) return;
    const auto c = city();
    if(c) c->incAttitude(-10., pid);
}
