#include "efavorchangeevent.h"

#include "engine/egameboard.h"
#include "engine/eeventdata.h"
#include "engine/eevent.h"
#include "elanguage.h"

eFavorChangeEvent::eFavorChangeEvent(
        const eCityId cid,
        const eGameEventBranch branch,
        eGameBoard& board) :
    eEconomicMilitaryChangeEventBase(
        cid, eGameEventType::favorChange, branch, board) {}

void eFavorChangeEvent::trigger() {
    chooseCity();
    const auto city = this->city();
    chooseCount();
    const int by = this->count();
    if(!city || by == 0) return;
    const auto board = gameBoard();
    if(!board) return;
    const auto pid = board->cityIdToPlayerId(cityId());
    city->incAttitude(by, pid);
    eEventData ed((ePlayerCityTarget(pid)));
    ed.fCity = city;
    board->event(by > 0 ? eEvent::favorIncrease : eEvent::favorDecrease, ed);
}

std::string eFavorChangeEvent::longName() const {
    return eLanguage::text("favor_change_long_name");
}
