#include "esupplychangeevent.h"
#include "emessages.h"

#include "engine/egameboard.h"
#include "engine/eeventdata.h"
#include "engine/eevent.h"
#include "elanguage.h"

eSupplyChangeEvent::eSupplyChangeEvent(
        const eCityId cid,
        const eGameEventBranch branch,
        eGameBoard& board) :
    eSupplyDemandChangeEvent(cid, eGameEventType::supplyChange,
                             branch, board) {}

void eSupplyChangeEvent::trigger() {
    chooseCity();
    const auto city = this->city();
    if(!city) return;
    chooseType();
    chooseCount();
    const auto type = this->resourceType();
    const int by = this->count();
    auto& sells = city->sells();
    for(auto& s : sells) {
        if(s.fType != type) continue;
        s.changeMax(by);
        break;
    }
    const auto board = gameBoard();
    if(!board) return;
    eEventData ed((ePlayerCityTarget()));
    ed.fCity = city;
    ed.fResourceType = type;
    const auto e = by > 0 ? eEvent::supplyIncrease :
                            eEvent::supplyDecrease;
    board->event(e, ed);
}

std::string eSupplyChangeEvent::longName() const {
    auto tmpl = eLanguage::text("supply_change_long_name");
    longNameReplaceResource("%1", tmpl);
    longNameReplaceCity("%2", tmpl);
    return tmpl;
}

bool eSupplyChangeEvent::oraclePrediction(eEventData& ed, eMessageType& msg) {
    oracleChooseCity();
    if(!mCity) return false;
    oracleChooseType();
    oracleChooseCount();
    ed.fCity = mCity;
    ed.fResourceType = resourceType();
    msg = eMessages::oracleMessage("supply_change", count() > 0 ? "_I" : "_D");
    return true;
}
