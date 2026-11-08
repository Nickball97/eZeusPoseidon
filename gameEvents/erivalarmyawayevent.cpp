#include "erivalarmyawayevent.h"
#include "emessages.h"

#include "engine/egameboard.h"
#include "engine/eeventdata.h"
#include "engine/eevent.h"
#include "elanguage.h"

eRivalArmyAwayEvent::eRivalArmyAwayEvent(
        const eCityId cid,
        const eGameEventBranch branch,
        eGameBoard& board) :
    eGameEvent(cid, eGameEventType::rivalArmyAway,
               branch, board),
    eCityEventValue(board) {}

void eRivalArmyAwayEvent::trigger() {
    chooseCity();
    if(!mCity) return;
    const auto board = gameBoard();
    if(!board) return;
    eEventData ed((ePlayerCityTarget()));
    ed.fCity = mCity;
    const int str = mCity->militaryStrength();
    mCity->setMilitaryStrength(str - 1);
    board->event(eEvent::rivalArmyAway, ed);
}

std::string eRivalArmyAwayEvent::longName() const {
    return eLanguage::text("rival_army_away_long_name");
}

void eRivalArmyAwayEvent::write(eWriteStream& dst) const {
    eGameEvent::write(dst);
    eCityEventValue::write(dst);
}

void eRivalArmyAwayEvent::read(eReadStream& src) {
    eGameEvent::read(src);
    eCityEventValue::read(src, *gameBoard());
}

bool eRivalArmyAwayEvent::oraclePrediction(eEventData& ed, eMessageType& msg) {
    oracleChooseCity();
    if(!mCity) return false;
    ed.fCity = mCity;
    msg = eMessages::oracleMessage("rival_away");
    return true;
}
