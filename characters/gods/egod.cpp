#include "egod.h"

#include "eathena.h"
#include "edemeter.h"
#include "edionysus.h"
#include "ehermes.h"

#include "elanguage.h"

#include "buildings/ebuilding.h"

#include <algorithm>

#include "textures/egametextures.h"

eGod::eGod(eGameBoard& board, const eGodType gt) :
    eCharacter(board, sGodToCharacterType(gt)),
    mType(gt) {
    setAttack(0.5);
}

int sGodTextId(const eGodType type) {
    switch(type) {
    case eGodType::aphrodite:
        return 6;
    case eGodType::apollo:
        return 3;
    case eGodType::ares:
        return 5;
    case eGodType::artemis:
        return 4;
    case eGodType::athena:
        return 8;
    case eGodType::atlas:
        return 13;
    case eGodType::demeter:
        return 2;
    case eGodType::dionysus:
        return 10;
    case eGodType::hades:
        return 11;
    case eGodType::hephaestus:
        return 9;
    case eGodType::hera:
        return 12;
    case eGodType::hermes:
        return 7;
    case eGodType::poseidon:
        return 1;
    case eGodType::zeus:
    default:
        return 0;
    }
}

eGodType eGod::sCharacterToGodType(const eCharacterType type,
                                   bool* const valid) {
    if(valid) *valid = true;
    switch(type) {
    case eCharacterType::aphrodite: return eGodType::aphrodite;
    case eCharacterType::apollo: return eGodType::apollo;
    case eCharacterType::ares: return eGodType::ares;
    case eCharacterType::artemis: return eGodType::artemis;
    case eCharacterType::athena: return eGodType::athena;
    case eCharacterType::atlas: return eGodType::atlas;
    case eCharacterType::demeter: return eGodType::demeter;
    case eCharacterType::dionysus: return eGodType::dionysus;
    case eCharacterType::hades: return eGodType::hades;
    case eCharacterType::hephaestus: return eGodType::hephaestus;
    case eCharacterType::hera: return eGodType::hera;
    case eCharacterType::hermes: return eGodType::hermes;
    case eCharacterType::poseidon: return eGodType::poseidon;
    case eCharacterType::zeus: return eGodType::zeus;
    default:
        if(valid) *valid = false;
        return eGodType::aphrodite;
    }
}

eCharacterType eGod::sGodToCharacterType(const eGodType type) {
    switch(type) {
    case eGodType::aphrodite: return eCharacterType::aphrodite;
    case eGodType::apollo: return eCharacterType::apollo;
    case eGodType::ares: return eCharacterType::ares;
    case eGodType::artemis: return eCharacterType::artemis;
    case eGodType::athena: return eCharacterType::athena;
    case eGodType::atlas: return eCharacterType::atlas;
    case eGodType::demeter: return eCharacterType::demeter;
    case eGodType::dionysus: return eCharacterType::dionysus;
    case eGodType::hades: return eCharacterType::hades;
    case eGodType::hephaestus: return eCharacterType::hephaestus;
    case eGodType::hera: return eCharacterType::hera;
    case eGodType::hermes: return eCharacterType::hermes;
    case eGodType::poseidon: return eCharacterType::poseidon;
    case eGodType::zeus: return eCharacterType::zeus;
    default: return eCharacterType::aphrodite;
    }
}

stdsptr<eGod> eGod::sCreateGod(const eGodType type, eGameBoard& board) {
    switch(type) {
    case eGodType::aphrodite:
        return e::make_shared<eAphrodite>(board);
    case eGodType::apollo:
        return e::make_shared<eApollo>(board);
    case eGodType::ares:
        return e::make_shared<eAres>(board);
    case eGodType::artemis:
        return e::make_shared<eArtemis>(board);
    case eGodType::athena:
        return e::make_shared<eAthena>(board);
    case eGodType::atlas:
        return e::make_shared<eAtlas>(board);
    case eGodType::demeter:
        return e::make_shared<eDemeter>(board);
    case eGodType::dionysus:
        return e::make_shared<eDionysus>(board);
    case eGodType::hades:
        return e::make_shared<eHades>(board);
    case eGodType::hephaestus:
        return e::make_shared<eHephaestus>(board);
    case eGodType::hera:
        return e::make_shared<eHera>(board);
    case eGodType::hermes:
        return e::make_shared<eHermes>(board);
    case eGodType::poseidon:
        return e::make_shared<ePoseidon>(board);
    case eGodType::zeus:
        return e::make_shared<eZeus>(board);
    }
    return nullptr;
}

eGod::eTexPtr eGod::sGodMissile(const eGodType gt) {
    switch(gt) {
    case eGodType::aphrodite:
        eGameTextures::loadGodBlueMissile();
        return &eDestructionTextures::fGodBlueMissile;
    case eGodType::apollo:
        eGameTextures::loadGodOrangeArrow();
        return &eDestructionTextures::fGodOrangeArrow;
    case eGodType::ares:
        eGameTextures::loadGodOrangeMissile();
        return &eDestructionTextures::fGodOrangeMissile;
    case eGodType::artemis:
        eGameTextures::loadGodBlueArrow();
        return &eDestructionTextures::fGodBlueArrow;
    case eGodType::athena:
        eGameTextures::loadGodRedMissile();
        return &eDestructionTextures::fGodRedMissile;
    case eGodType::atlas:
        eGameTextures::loadGodPinkMissile();
        return &eDestructionTextures::fGodPinkMissile;
    case eGodType::demeter:
        eGameTextures::loadGodGreenMissile();
        return &eDestructionTextures::fGodGreenMissile;
    case eGodType::dionysus:
        eGameTextures::loadGodPurpleMissile();
        return &eDestructionTextures::fGodPurpleMissile;
    case eGodType::hades:
        eGameTextures::loadGodOrangeMissile();
        return &eDestructionTextures::fGodOrangeMissile;
    case eGodType::hephaestus:
        eGameTextures::loadGodOrangeMissile();
        return &eDestructionTextures::fGodOrangeMissile;
    case eGodType::hera:
        eGameTextures::loadGodRedMissile();
        return &eDestructionTextures::fGodRedMissile;
    case eGodType::hermes:
        eGameTextures::loadGodBlueMissile();
        return &eDestructionTextures::fGodBlueMissile;
    case eGodType::poseidon:
        eGameTextures::loadGodBlueMissile();
        return &eDestructionTextures::fGodBlueMissile;
    case eGodType::zeus:
        eGameTextures::loadGodBlueMissile();
        return &eDestructionTextures::fGodBlueMissile;
    }
    return nullptr;
}

std::map<std::pair<eGodType, eGodType>, eGodType> gFightWinner {
    {{eGodType::zeus, eGodType::zeus}, eGodType::zeus},
    {{eGodType::zeus, eGodType::poseidon}, eGodType::zeus},
    {{eGodType::zeus, eGodType::demeter}, eGodType::zeus},
    {{eGodType::zeus, eGodType::apollo}, eGodType::zeus},
    {{eGodType::zeus, eGodType::artemis}, eGodType::zeus},
    {{eGodType::zeus, eGodType::ares}, eGodType::zeus},
    {{eGodType::zeus, eGodType::aphrodite}, eGodType::zeus},
    {{eGodType::zeus, eGodType::hermes}, eGodType::zeus},
    {{eGodType::zeus, eGodType::athena}, eGodType::zeus},
    {{eGodType::zeus, eGodType::hephaestus}, eGodType::zeus},
    {{eGodType::zeus, eGodType::dionysus}, eGodType::zeus},
    {{eGodType::zeus, eGodType::hades}, eGodType::zeus},
    {{eGodType::zeus, eGodType::hera}, eGodType::hera},
    {{eGodType::zeus, eGodType::atlas}, eGodType::zeus},

    {{eGodType::poseidon, eGodType::zeus}, eGodType::zeus},
    {{eGodType::poseidon, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::demeter}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::apollo}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::artemis}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::ares}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::aphrodite}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::hermes}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::athena}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::hephaestus}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::dionysus}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::hades}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::hera}, eGodType::poseidon},
    {{eGodType::poseidon, eGodType::atlas}, eGodType::poseidon},

    {{eGodType::demeter, eGodType::zeus}, eGodType::zeus},
    {{eGodType::demeter, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::demeter, eGodType::demeter}, eGodType::demeter},
    {{eGodType::demeter, eGodType::apollo}, eGodType::demeter},
    {{eGodType::demeter, eGodType::artemis}, eGodType::demeter},
    {{eGodType::demeter, eGodType::ares}, eGodType::demeter},
    {{eGodType::demeter, eGodType::aphrodite}, eGodType::demeter},
    {{eGodType::demeter, eGodType::hermes}, eGodType::demeter},
    {{eGodType::demeter, eGodType::athena}, eGodType::demeter},
    {{eGodType::demeter, eGodType::hephaestus}, eGodType::demeter},
    {{eGodType::demeter, eGodType::dionysus}, eGodType::demeter},
    {{eGodType::demeter, eGodType::hades}, eGodType::hades},
    {{eGodType::demeter, eGodType::hera}, eGodType::hera},
    {{eGodType::demeter, eGodType::atlas}, eGodType::demeter},

    {{eGodType::apollo, eGodType::zeus}, eGodType::zeus},
    {{eGodType::apollo, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::apollo, eGodType::demeter}, eGodType::demeter},
    {{eGodType::apollo, eGodType::apollo}, eGodType::apollo},
    {{eGodType::apollo, eGodType::artemis}, eGodType::artemis},
    {{eGodType::apollo, eGodType::ares}, eGodType::apollo},
    {{eGodType::apollo, eGodType::aphrodite}, eGodType::apollo},
    {{eGodType::apollo, eGodType::hermes}, eGodType::apollo},
    {{eGodType::apollo, eGodType::athena}, eGodType::athena},
    {{eGodType::apollo, eGodType::hephaestus}, eGodType::apollo},
    {{eGodType::apollo, eGodType::dionysus}, eGodType::apollo},
    {{eGodType::apollo, eGodType::hades}, eGodType::hades},
    {{eGodType::apollo, eGodType::hera}, eGodType::hera},
    {{eGodType::apollo, eGodType::atlas}, eGodType::apollo},

    {{eGodType::artemis, eGodType::zeus}, eGodType::zeus},
    {{eGodType::artemis, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::artemis, eGodType::demeter}, eGodType::demeter},
    {{eGodType::artemis, eGodType::apollo}, eGodType::artemis},
    {{eGodType::artemis, eGodType::artemis}, eGodType::artemis},
    {{eGodType::artemis, eGodType::ares}, eGodType::artemis},
    {{eGodType::artemis, eGodType::aphrodite}, eGodType::artemis},
    {{eGodType::artemis, eGodType::hermes}, eGodType::artemis},
    {{eGodType::artemis, eGodType::athena}, eGodType::athena},
    {{eGodType::artemis, eGodType::hephaestus}, eGodType::artemis},
    {{eGodType::artemis, eGodType::dionysus}, eGodType::artemis},
    {{eGodType::artemis, eGodType::hades}, eGodType::hades},
    {{eGodType::artemis, eGodType::hera}, eGodType::hera},
    {{eGodType::artemis, eGodType::atlas}, eGodType::artemis},

    {{eGodType::ares, eGodType::zeus}, eGodType::zeus},
    {{eGodType::ares, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::ares, eGodType::demeter}, eGodType::demeter},
    {{eGodType::ares, eGodType::apollo}, eGodType::apollo},
    {{eGodType::ares, eGodType::artemis}, eGodType::artemis},
    {{eGodType::ares, eGodType::ares}, eGodType::ares},
    {{eGodType::ares, eGodType::aphrodite}, eGodType::aphrodite},
    {{eGodType::ares, eGodType::hermes}, eGodType::ares},
    {{eGodType::ares, eGodType::athena}, eGodType::athena},
    {{eGodType::ares, eGodType::hephaestus}, eGodType::ares},
    {{eGodType::ares, eGodType::dionysus}, eGodType::ares},
    {{eGodType::ares, eGodType::hades}, eGodType::hades},
    {{eGodType::ares, eGodType::hera}, eGodType::hera},
    {{eGodType::ares, eGodType::atlas}, eGodType::atlas},

    {{eGodType::aphrodite, eGodType::zeus}, eGodType::zeus},
    {{eGodType::aphrodite, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::aphrodite, eGodType::demeter}, eGodType::demeter},
    {{eGodType::aphrodite, eGodType::apollo}, eGodType::apollo},
    {{eGodType::aphrodite, eGodType::artemis}, eGodType::artemis},
    {{eGodType::aphrodite, eGodType::ares}, eGodType::aphrodite},
    {{eGodType::aphrodite, eGodType::aphrodite}, eGodType::aphrodite},
    {{eGodType::aphrodite, eGodType::hermes}, eGodType::aphrodite},
    {{eGodType::aphrodite, eGodType::athena}, eGodType::athena},
    {{eGodType::aphrodite, eGodType::hephaestus}, eGodType::aphrodite},
    {{eGodType::aphrodite, eGodType::dionysus}, eGodType::aphrodite},
    {{eGodType::aphrodite, eGodType::hades}, eGodType::hades},
    {{eGodType::aphrodite, eGodType::hera}, eGodType::hera},
    {{eGodType::aphrodite, eGodType::atlas}, eGodType::atlas},

    {{eGodType::hermes, eGodType::zeus}, eGodType::zeus},
    {{eGodType::hermes, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::hermes, eGodType::demeter}, eGodType::demeter},
    {{eGodType::hermes, eGodType::apollo}, eGodType::apollo},
    {{eGodType::hermes, eGodType::artemis}, eGodType::artemis},
    {{eGodType::hermes, eGodType::ares}, eGodType::ares},
    {{eGodType::hermes, eGodType::aphrodite}, eGodType::aphrodite},
    {{eGodType::hermes, eGodType::hermes}, eGodType::hermes},
    {{eGodType::hermes, eGodType::athena}, eGodType::athena},
    {{eGodType::hermes, eGodType::hephaestus}, eGodType::hephaestus},
    {{eGodType::hermes, eGodType::dionysus}, eGodType::hermes},
    {{eGodType::hermes, eGodType::hades}, eGodType::hades},
    {{eGodType::hermes, eGodType::hera}, eGodType::hera},
    {{eGodType::hermes, eGodType::atlas}, eGodType::atlas},

    {{eGodType::athena, eGodType::zeus}, eGodType::zeus},
    {{eGodType::athena, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::athena, eGodType::demeter}, eGodType::demeter},
    {{eGodType::athena, eGodType::apollo}, eGodType::athena},
    {{eGodType::athena, eGodType::artemis}, eGodType::athena},
    {{eGodType::athena, eGodType::ares}, eGodType::athena},
    {{eGodType::athena, eGodType::aphrodite}, eGodType::athena},
    {{eGodType::athena, eGodType::hermes}, eGodType::athena},
    {{eGodType::athena, eGodType::athena}, eGodType::athena},
    {{eGodType::athena, eGodType::hephaestus}, eGodType::athena},
    {{eGodType::athena, eGodType::dionysus}, eGodType::athena},
    {{eGodType::athena, eGodType::hades}, eGodType::hades},
    {{eGodType::athena, eGodType::hera}, eGodType::hera},
    {{eGodType::athena, eGodType::atlas}, eGodType::athena},

    {{eGodType::hephaestus, eGodType::zeus}, eGodType::zeus},
    {{eGodType::hephaestus, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::hephaestus, eGodType::demeter}, eGodType::demeter},
    {{eGodType::hephaestus, eGodType::apollo}, eGodType::apollo},
    {{eGodType::hephaestus, eGodType::artemis}, eGodType::artemis},
    {{eGodType::hephaestus, eGodType::ares}, eGodType::ares},
    {{eGodType::hephaestus, eGodType::aphrodite}, eGodType::aphrodite},
    {{eGodType::hephaestus, eGodType::hermes}, eGodType::hephaestus},
    {{eGodType::hephaestus, eGodType::athena}, eGodType::athena},
    {{eGodType::hephaestus, eGodType::hephaestus}, eGodType::hephaestus},
    {{eGodType::hephaestus, eGodType::dionysus}, eGodType::hephaestus},
    {{eGodType::hephaestus, eGodType::hades}, eGodType::hades},
    {{eGodType::hephaestus, eGodType::hera}, eGodType::hera},
    {{eGodType::hephaestus, eGodType::atlas}, eGodType::atlas},

    {{eGodType::dionysus, eGodType::zeus}, eGodType::zeus},
    {{eGodType::dionysus, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::dionysus, eGodType::demeter}, eGodType::demeter},
    {{eGodType::dionysus, eGodType::apollo}, eGodType::apollo},
    {{eGodType::dionysus, eGodType::artemis}, eGodType::artemis},
    {{eGodType::dionysus, eGodType::ares}, eGodType::ares},
    {{eGodType::dionysus, eGodType::aphrodite}, eGodType::aphrodite},
    {{eGodType::dionysus, eGodType::hermes}, eGodType::hermes},
    {{eGodType::dionysus, eGodType::athena}, eGodType::athena},
    {{eGodType::dionysus, eGodType::hephaestus}, eGodType::hephaestus},
    {{eGodType::dionysus, eGodType::dionysus}, eGodType::dionysus},
    {{eGodType::dionysus, eGodType::hades}, eGodType::hades},
    {{eGodType::dionysus, eGodType::hera}, eGodType::hera},
    {{eGodType::dionysus, eGodType::atlas}, eGodType::atlas},

    {{eGodType::hades, eGodType::zeus}, eGodType::zeus},
    {{eGodType::hades, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::hades, eGodType::demeter}, eGodType::hades},
    {{eGodType::hades, eGodType::apollo}, eGodType::hades},
    {{eGodType::hades, eGodType::artemis}, eGodType::hades},
    {{eGodType::hades, eGodType::ares}, eGodType::hades},
    {{eGodType::hades, eGodType::aphrodite}, eGodType::hades},
    {{eGodType::hades, eGodType::hermes}, eGodType::hades},
    {{eGodType::hades, eGodType::athena}, eGodType::hades},
    {{eGodType::hades, eGodType::hephaestus}, eGodType::hades},
    {{eGodType::hades, eGodType::dionysus}, eGodType::hades},
    {{eGodType::hades, eGodType::hades}, eGodType::hades},
    {{eGodType::hades, eGodType::hera}, eGodType::hades},
    {{eGodType::hades, eGodType::atlas}, eGodType::hades},

    {{eGodType::hera, eGodType::zeus}, eGodType::hera},
    {{eGodType::hera, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::hera, eGodType::demeter}, eGodType::hera},
    {{eGodType::hera, eGodType::apollo}, eGodType::hera},
    {{eGodType::hera, eGodType::artemis}, eGodType::hera},
    {{eGodType::hera, eGodType::ares}, eGodType::hera},
    {{eGodType::hera, eGodType::aphrodite}, eGodType::hera},
    {{eGodType::hera, eGodType::hermes}, eGodType::hera},
    {{eGodType::hera, eGodType::athena}, eGodType::hera},
    {{eGodType::hera, eGodType::hephaestus}, eGodType::hera},
    {{eGodType::hera, eGodType::dionysus}, eGodType::hera},
    {{eGodType::hera, eGodType::hades}, eGodType::hades},
    {{eGodType::hera, eGodType::hera}, eGodType::hera},
    {{eGodType::hera, eGodType::atlas}, eGodType::hera},

    {{eGodType::atlas, eGodType::zeus}, eGodType::zeus},
    {{eGodType::atlas, eGodType::poseidon}, eGodType::poseidon},
    {{eGodType::atlas, eGodType::demeter}, eGodType::demeter},
    {{eGodType::atlas, eGodType::apollo}, eGodType::apollo},
    {{eGodType::atlas, eGodType::artemis}, eGodType::artemis},
    {{eGodType::atlas, eGodType::ares}, eGodType::atlas},
    {{eGodType::atlas, eGodType::aphrodite}, eGodType::atlas},
    {{eGodType::atlas, eGodType::hermes}, eGodType::atlas},
    {{eGodType::atlas, eGodType::athena}, eGodType::athena},
    {{eGodType::atlas, eGodType::hephaestus}, eGodType::atlas},
    {{eGodType::atlas, eGodType::dionysus}, eGodType::atlas},
    {{eGodType::atlas, eGodType::hades}, eGodType::hades},
    {{eGodType::atlas, eGodType::hera}, eGodType::hera},
    {{eGodType::atlas, eGodType::atlas}, eGodType::atlas},
};

eGodType eGod::sFightWinner(const eGodType g1, const eGodType g2) {
    return gFightWinner[{g1, g2}];
}

std::string eGod::sFightResultString(const eGodType g1, const eGodType g2) {
    const auto w = sFightWinner(g1, g2);
    const auto l = g1 == w ? g2 : g1;
    const int wi = sGodTextId(w);
    const int li = sGodTextId(l);
    const int group = 332;
    const int string = 14*wi + li;
    return eLanguage::zeusText(group, string);
}

int sGodAttackSpriteLength(const eGodType gt) {
    switch(gt) {
    case eGodType::aphrodite: return eGodTextures::sAphroditeFBTime;
    case eGodType::apollo: return eGodTextures::sApolloFTime;
    case eGodType::ares: return eGodTextures::sAresFBTime;
    case eGodType::artemis: return eGodTextures::sArtemisFTime;
    case eGodType::athena: return eGodTextures::sAthenaFTime;
    case eGodType::atlas: return eGodTextures::sAtlasFTime;
    case eGodType::demeter: return eGodTextures::sDemeterFBTime;
    case eGodType::dionysus: return eGodTextures::sDionysusFBTime;
    case eGodType::hades: return eGodTextures::sHadesFBTime;
    case eGodType::hephaestus: return eGodTextures::sHephaestusFBTime;
    case eGodType::hera: return eGodTextures::sHeraFBTime;
    case eGodType::hermes: return eGodTextures::sHermesFBTime;
    case eGodType::poseidon: return eGodTextures::sPoseidonFBTime;
    case eGodType::zeus: return eGodTextures::sZeusFTime;
    }
    return 0;
}

int eGod::sGodAttackTime(const eGodType gt) {
    const int sl = sGodAttackSpriteLength(gt);
    return sl*eCharacter::sTextureTimeDivisor;
}

int sGodBlessSpriteLength(const eGodType gt) {
    switch(gt) {
    case eGodType::aphrodite: return eGodTextures::sAphroditeFBTime;
    case eGodType::apollo: return eGodTextures::sApolloBTime;
    case eGodType::ares: return eGodTextures::sAresFBTime;
    case eGodType::artemis: return eGodTextures::sArtemisBTime;
    case eGodType::athena: return eGodTextures::sAthenaBTime;
    case eGodType::atlas: return eGodTextures::sAtlasBTime;
    case eGodType::demeter: return eGodTextures::sDemeterFBTime;
    case eGodType::dionysus: return eGodTextures::sDionysusFBTime;
    case eGodType::hades: return eGodTextures::sHadesFBTime;
    case eGodType::hephaestus: return eGodTextures::sHephaestusFBTime;
    case eGodType::hera: return eGodTextures::sHeraFBTime;
    case eGodType::hermes: return eGodTextures::sHermesFBTime;
    case eGodType::poseidon: return eGodTextures::sPoseidonFBTime;
    case eGodType::zeus: return eGodTextures::sZeusBTime;
    }
    return 0;
}

int eGod::sGodBlessTime(const eGodType gt) {
    const int sl = sGodBlessSpriteLength(gt);
    return sl*eCharacter::sTextureTimeDivisor;
}

int sGodAppearSpriteLength(const eGodType gt) {
    switch(gt) {
    case eGodType::aphrodite: return eGodTextures::sAphroditeDTime;
    case eGodType::apollo: return eGodTextures::sApolloDTime;
    case eGodType::ares: return eGodTextures::sAresDTime;
    case eGodType::artemis: return eGodTextures::sArtemisDTime;
    case eGodType::athena: return eGodTextures::sAthenaDTime;
    case eGodType::atlas: return eGodTextures::sAtlasDTime;
    case eGodType::demeter: return eGodTextures::sDemeterDTime;
    case eGodType::dionysus: return eGodTextures::sDionysusATime;
    case eGodType::hades: return eGodTextures::sHadesDTime;
    case eGodType::hephaestus: return eGodTextures::sHephaestusDTime;
    case eGodType::hera: return eGodTextures::sHeraDTime;
    case eGodType::hermes: return 0;
    case eGodType::poseidon: return eGodTextures::sPoseidonDTime;
    case eGodType::zeus: return eGodTextures::sZeusDTime;
    }
    return 0;
}

int eGod::sGodAppearTime(const eGodType gt) {
    const int sl = sGodAppearSpriteLength(gt);
    return sl*eCharacter::sTextureTimeDivisor;
}

// Buildings each god sanctifies (blesses) while walking around the city.
// Zeus manual, Mythology chapter (Sanctuaries), and Poseidon manual (Hera,
// Atlas, Apollo in Atlantis); HeavenGames' sanctuaries page agrees. Atlas
// follows HeavenGames and the Citadel of Poseidon (timber mill and refinery),
// per Nicolás; his finished sanctuary also speeds up marble quarrying.
bool eGod::sTarget(const eGodType gt, const eBuildingType bt) {
    switch(gt) {
    case eGodType::aphrodite: // "she sanctifies some of your houses"
        return bt == eBuildingType::commonHouse ||
               bt == eBuildingType::eliteHousing;
    case eGodType::apollo:
        // Podiums, Gymnasiums, Theaters and Stadium; in Atlantis any
        // Bibliotheke, Laboratory, Observatory or ... (the manual's
        // sentence is cut off; the Museum is kept as before).
        return bt == eBuildingType::podium ||
               bt == eBuildingType::gymnasium ||
               bt == eBuildingType::theater ||
               bt == eBuildingType::stadium ||
               bt == eBuildingType::bibliotheke ||
               bt == eBuildingType::laboratory ||
               bt == eBuildingType::observatory ||
               bt == eBuildingType::museum;
    case eGodType::ares:
        return false;
    case eGodType::artemis: // the Boar Hunter's Lodge
        return bt == eBuildingType::huntingLodge;
    case eGodType::athena: // Carding Sheds and Olive Presses
        return bt == eBuildingType::cardingShed ||
               bt == eBuildingType::olivePress;
    case eGodType::atlas: // Timber Mills and Refineries
        return bt == eBuildingType::timberMill ||
               bt == eBuildingType::refinery;
    case eGodType::demeter: // "your farms"
        return bt == eBuildingType::wheatFarm ||
               bt == eBuildingType::onionsFarm ||
               bt == eBuildingType::carrotsFarm;
    case eGodType::dionysus: // Wineries
        return bt == eBuildingType::winery;
    case eGodType::hades: // Tax Offices, Mints and Foundries
        return bt == eBuildingType::taxOffice ||
               bt == eBuildingType::mint ||
               bt == eBuildingType::foundry;
    case eGodType::hephaestus: // Armories and Sculpture Studios
        return bt == eBuildingType::armory ||
               bt == eBuildingType::sculptureStudio;
    case eGodType::hera: // Corrals and Orange Tenders' Lodges
        return bt == eBuildingType::corral ||
               bt == eBuildingType::orangeTendersLodge;
    case eGodType::hermes:
        return false;
    case eGodType::poseidon: // Fisheries, Urchin Quays, Horse Ranches
        return bt == eBuildingType::urchinQuay ||
               bt == eBuildingType::fishery ||
               bt == eBuildingType::horseRanch;
    case eGodType::zeus: // "will occasionally sanctify a building"
        return true;
    }
    return false;
}
// What an angry (attacking) god does to buildings. Zeus manual, Mythology
// chapter ("When Gods Attack"), and Poseidon manual (Hera, Atlas, Apollo
// in Atlantis).

// Buildings an angry god destroys on purpose. Only Zeus is described as
// destroying a particular building: "He'll destroy the city's Palace,
// then look for other buildings to crush" (the second part is his
// general attack, see sGeneralAttack).
bool eGod::sAttackTarget(const eGodType gt, const eBuildingType bt) {
    switch(gt) {
    case eGodType::zeus:
        return bt == eBuildingType::palace;
    case eGodType::demeter: // "destroy crops"; per Nicolas, the farm buildings themselves
        return bt == eBuildingType::wheatFarm ||
               bt == eBuildingType::carrotsFarm ||
               bt == eBuildingType::onionsFarm;
    case eGodType::poseidon:
        // "Poseidon destroys fisheries, urchin quays, piers and ships"
        // (Citadel of Poseidon); a dock's water part is the pier
        return bt == eBuildingType::fishery ||
               bt == eBuildingType::urchinQuay ||
               bt == eBuildingType::pier;
    default:
        return false;
    }
}

// Buildings an angry god curses; cursed buildings stop working until the
// curse is lifted.
int eGod::sCurseKind(const eGodType gt) {
    switch(gt) {
    case eGodType::zeus: return 10;
    case eGodType::poseidon: return 1;
    case eGodType::demeter:
    case eGodType::artemis: return 0;
    case eGodType::apollo:
    case eGodType::dionysus: return 6;
    case eGodType::hephaestus:
    case eGodType::atlas: return 2;
    case eGodType::athena: return 3;
    case eGodType::hades: return 4;
    case eGodType::hera: return 9;
    // Ares (list 10), Aphrodite (7) and Hermes (2) have 0 months
    default: return -1;
    }
}

bool eGod::sCurseKindTarget(const int kind, const eBuildingType bt) {
    using T = eBuildingType;
    const auto in = [bt](const std::initializer_list<T> l) {
        return std::find(l.begin(), l.end(), bt) != l.end();
    };
    const bool sanct = bt >= T::templeAphrodite && bt <= T::templeZeus;
    switch(kind) {
    case 0: // granary, farms, agoras, dairy, hunting, fishing, corral,
            // fruit growers' lodge
        return in({T::granary, T::wheatFarm, T::onionsFarm, T::carrotsFarm,
                   T::commonAgora, T::grandAgora, T::dairy,
                   T::huntingLodge, T::fishery, T::urchinQuay, T::corral,
                   T::orangeTendersLodge});
    case 1:
        return in({T::fishery, T::urchinQuay, T::triremeWharf, T::pier});
    case 2:
        return in({T::timberMill, T::masonryShop, T::foundry, T::mint,
                   T::cardingShed, T::sculptureStudio, T::olivePress,
                   T::winery, T::armory, T::warehouse, T::refinery,
                   T::chariotFactory, T::blackMarbleWorkshop});
    case 3:
        return in({T::armory, T::horseRanch, T::wall, T::tower,
                   T::gatehouse, T::triremeWharf, T::foundry,
                   T::chariotFactory, T::watchPost});
    case 4:
        return in({T::taxOffice, T::eliteHousing, T::mint, T::pier,
                   T::tradePost, T::hippodromePiece});
    case 5:
        return in({T::tower, T::triremeWharf});
    case 6:
        return in({T::commonHouse, T::watchPost, T::hospital,
                   T::maintenanceOffice, T::commonAgora, T::grandAgora,
                   T::fountain, T::bibliotheke, T::gymnasium});
    case 7:
        return in({T::doricColumn, T::ionicColumn, T::corinthianColumn,
                   T::bench, T::flowerGarden, T::gazebo, T::hedgeMaze,
                   T::fishPond, T::tallObelisk, T::sundial, T::topiary,
                   T::spring, T::stoneCircle, T::shortObelisk,
                   T::dolphinSculpture, T::orrery, T::shellGarden,
                   T::baths, T::birdBath, T::commemorative, T::waterPark});
    case 8:
        return sanct || eBuilding::sHeroHall(bt) ||
               in({T::artisansGuild, T::masonryShop});
    case 9:
        return sanct || eBuilding::sHeroHall(bt) ||
               in({T::palace, T::stadium, T::theater, T::hippodromePiece,
                   T::museum, T::observatory, T::eliteHousing});
    default:
        return false;
    }
}

int eGod::sCurseMonths(const eGodType gt, const int difficulty) {
    const int d = std::clamp(difficulty, 0, 4);
    switch(gt) {
    case eGodType::apollo: {
        const int m[5] = {1, 1, 2, 2, 3};
        return m[d];
    }
    case eGodType::ares:
    case eGodType::aphrodite:
    case eGodType::hermes:
        return 0;
    default: {
        const int m[5] = {1, 2, 4, 6, 9};
        return m[d];
    }
    }
}

bool eGod::sCurseTarget(const eGodType gt, const eBuildingType bt) {
    const int kind = sCurseKind(gt);
    if(kind < 0) return false;
    if(kind < 10) return sCurseKindTarget(kind, bt);
    for(int k = 0; k < 10; k++) {
        if(sCurseKindTarget(k, bt)) return true;
    }
    return false;
}

// Gods that smash buildings and kill people wherever they walk. Zeus
// "wreaks general havoc"; Hephaestus sets buildings on fire. Ares and
// Artemis attack with armies (Sparti, Amazons) in the original; until
// those armies exist they keep attacking in person.
bool eGod::sGeneralAttack(const eGodType gt) {
    switch(gt) {
    case eGodType::zeus:
    case eGodType::hephaestus:
    case eGodType::ares:
    case eGodType::artemis:
        return true;
    default:
        return false;
    }
}

std::string eGod::sGodName(const eGodType gt) {
    const int group = 157;
    const int string = sGodTextId(gt);
    return eLanguage::zeusText(group, string);
}

void eGod::sGodStrings(std::vector<eGodType>& gods,
                       std::vector<std::string>& godNames) {
    gods.push_back(eGodType::aphrodite);
    gods.push_back(eGodType::apollo);
    gods.push_back(eGodType::ares);
    gods.push_back(eGodType::artemis);
    gods.push_back(eGodType::athena);
    gods.push_back(eGodType::atlas);
    gods.push_back(eGodType::demeter);
    gods.push_back(eGodType::dionysus);
    gods.push_back(eGodType::hades);
    gods.push_back(eGodType::hephaestus);
    gods.push_back(eGodType::hera);
    gods.push_back(eGodType::hermes);
    gods.push_back(eGodType::poseidon);
    gods.push_back(eGodType::zeus);
    for(const auto g : gods) {
        godNames.push_back(sGodName(g));
    }
}

void eGod::sLoadTextures(const eGodType g) {
    switch(g) {
    case eGodType::aphrodite:
        eGameTextures::loadAphrodite();
        break;
    case eGodType::apollo:
        eGameTextures::loadApollo();
        break;
    case eGodType::ares:
        eGameTextures::loadAres();
        break;
    case eGodType::artemis:
        eGameTextures::loadArtemis();
        break;
    case eGodType::athena:
        eGameTextures::loadAthena();
        break;
    case eGodType::atlas:
        eGameTextures::loadAtlas();
        break;
    case eGodType::demeter:
        eGameTextures::loadDemeter();
        break;
    case eGodType::dionysus:
        eGameTextures::loadDionysus();
        break;
    case eGodType::hades:
        eGameTextures::loadHades();
        break;
    case eGodType::hephaestus:
        eGameTextures::loadHephaestus();
        break;
    case eGodType::hera:
        eGameTextures::loadHera();
        break;
    case eGodType::hermes:
        eGameTextures::loadHermes();
        break;
    case eGodType::poseidon:
        eGameTextures::loadPoseidon();
        break;
    case eGodType::zeus:
        eGameTextures::loadZeus();
        break;
    }
}

void eGod::read(eReadStream& src) {
    eCharacter::read(src);
    src >> mAttitude;
}

void eGod::write(eWriteStream& dst) const {
    eCharacter::write(dst);
    dst << mAttitude;
}
