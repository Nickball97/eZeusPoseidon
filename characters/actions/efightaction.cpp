#include "efightaction.h"
#include "characters/efigurestats.h"

#include "characters/echaracter.h"

#include "characters/ebasicpatroler.h"
#include "characters/efightingpatroler.h"
#include "characters/eresourcecollector.h"
#include "characters/eanimal.h"

eFightAction::eFightAction(eCharacter* const c, eCharacter* const o) :
    eCharacterAction(c, eCharActionType::fightAction),
    mOpponent(o) {
    c->setActionType(eCharacterActionType::fight);
}

void eFightAction::increment(const int by) {
    const auto c = character();
    const bool dead = mOpponent ?
        mOpponent->defend(eFigureStats::sDamage(c, mOpponent.get(), false, by)) :
        true;
    if(dead || c->dead()) {
        setState(eCharacterActionState::finished);
    }
}

void eFightAction::read(eReadStream& src) {
    eCharacterAction::read(src);
    src.readCharacter(&board(), [this](eCharacter* const c) {
        mOpponent = c;
    });
    src >> mTime;
}

void eFightAction::write(eWriteStream& dst) const {
    eCharacterAction::write(dst);
    dst.writeCharacter(mOpponent);
    dst << mTime;
}
