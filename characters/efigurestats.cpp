#include "efigurestats.h"

#include "echaracter.h"
#include "engine/egameboard.h"
#include "engine/edifficulty.h"
#include "enumbers.h"

#include <algorithm>

namespace {
// {HP, attack, armor, armor vs missiles, missile attack, missile rate}
using eS = eFigureStats;

// The player's own units change with difficulty
// (Figure_model_VeryEasy/Easy/Normal/Hard/Impossible = Beginner..Olympian).
const eS sPlayerRabble[5] = { // "Rabble/Archer"
    {100, 5, 2, 4, 6, 70}, {90, 4, 1, 2, 6, 75}, {80, 4, 0, 1, 6, 80},
    {80, 4, 0, 1, 6, 85}, {80, 3, 0, 1, 6, 90}};
const eS sPlayerHoplite[5] = { // "Hoplite/Spearman"
    {180, 15, 8, 6, 0, 0}, {160, 14, 6, 6, 0, 0}, {140, 12, 6, 5, 0, 0},
    {120, 12, 6, 5, 0, 0}, {110, 12, 6, 5, 0, 0}};
const eS sPlayerHorseman[5] = { // "Horseman/Chariot"
    {300, 18, 7, 5, 0, 0}, {260, 16, 6, 4, 0, 0}, {240, 15, 4, 4, 0, 0},
    {200, 15, 4, 4, 0, 0}, {180, 15, 4, 4, 0, 0}};

// Enemies ("ALL ENEMIES"), the same at every difficulty.
bool sEnemy(const eCharacterType type, eS& s) {
    switch(type) {
    case eCharacterType::greekHoplite: s = {150, 15, 6, 2, 0, 0, 60}; return true;
    case eCharacterType::greekRockThrower: s = {100, 5, 2, 3, 6, 90, 20}; return true;
    case eCharacterType::greekHorseman: s = {250, 17, 4, 4, 0, 0, 20}; return true;
    case eCharacterType::trojanHoplite: s = {180, 15, 6, 2, 0, 0, 50}; return true; // Swordsman
    case eCharacterType::trojanSpearthrower: s = {100, 5, 2, 3, 6, 90, 20}; return true; // Javelineers
    case eCharacterType::trojanHorseman: s = {200, 17, 4, 4, 0, 0, 30}; return true;
    case eCharacterType::persianHoplite: s = {150, 15, 6, 2, 0, 0, 30}; return true; // Infantry
    case eCharacterType::persianArcher: s = {100, 5, 2, 3, 6, 90, 40}; return true; // Missile
    case eCharacterType::persianHorseman: s = {250, 17, 4, 4, 0, 0, 30}; return true; // Cavalry
    case eCharacterType::centaurHorseman: s = {150, 15, 6, 2, 0, 0, 50}; return true; // Swordsman
    case eCharacterType::centaurArcher: s = {100, 5, 2, 3, 6, 90, 50}; return true;
    case eCharacterType::egyptianHoplite: s = {150, 14, 4, 5, 0, 0, 50}; return true; // Infantry
    case eCharacterType::egyptianArcher: s = {100, 5, 2, 2, 6, 85, 30}; return true; // Missile
    case eCharacterType::egyptianChariot: s = {250, 17, 6, 5, 0, 0, 20}; return true; // Cavalry
    case eCharacterType::mayanHoplite: s = {180, 16, 4, 2, 0, 0, 75}; return true; // Infantry
    case eCharacterType::mayanArcher: s = {100, 5, 3, 3, 6, 90, 25}; return true; // Missile
    case eCharacterType::phoenicianArcher: s = {100, 6, 5, 5, 8, 75, 70}; return true; // Missile
    case eCharacterType::phoenicianHorseman: s = {250, 16, 7, 7, 0, 0, 30}; return true; // Cavalry
    case eCharacterType::oceanidHoplite: s = {200, 16, 8, 8, 0, 0, 50}; return true; // Infantry
    case eCharacterType::oceanidSpearthrower: s = {200, 8, 4, 4, 7, 90, 50}; return true; // Missile
    case eCharacterType::atlanteanHoplite: s = {140, 13, 5, 6, 0, 0, 40}; return true; // Infantry
    case eCharacterType::atlanteanArcher: s = {80, 4, 2, 2, 8, 75, 30}; return true; // Missile
    case eCharacterType::atlanteanChariot: s = {250, 18, 6, 4, 0, 0, 30}; return true; // Cavalry
    case eCharacterType::aresWarrior: s = {220, 16, 8, 8, 0, 0, 100}; return true; // Ares Warriors
    default: return false;
    }
}

// Calibration: a Hero-difficulty player hoplite keeps eZeus's strength.
const eS& sRefHoplite() { return sPlayerHoplite[2]; }
const eS& sRefRabble() { return sPlayerRabble[2]; }
}

bool eFigureStats::sGet(const eCharacterType type,
                        const eDifficulty diff,
                        eFigureStats& result) {
    const int d = std::clamp(static_cast<int>(diff), 0, 4);
    switch(type) {
    case eCharacterType::rockThrower:
    case eCharacterType::archerPoseidon:
        result = sPlayerRabble[d];
        return true;
    case eCharacterType::hoplite:
    case eCharacterType::hoplitePoseidon:
        result = sPlayerHoplite[d];
        return true;
    case eCharacterType::horseman:
    case eCharacterType::chariotPoseidon:
        result = sPlayerHorseman[d];
        return true;
    default:
        return sEnemy(type, result);
    }
}

void eFigureStats::sApply(eCharacter* const c) {
    auto& board = c->getBoard();
    const auto diff = board.difficulty(board.personPlayer());
    eFigureStats s;
    if(!sGet(c->type(), diff, s)) return;
    sApply(c, s);
}

const eFigureStats eFigureStats::sAmazonWarrior{120, 12, 4, 2, 0, 0, 25};
const eFigureStats eFigureStats::sAmazonArcher{80, 5, 2, 3, 8, 75, 75};

void eFigureStats::sApply(eCharacter* const c, const eFigureStats& s) {
    const double hpScale = eNumbers::sHopliteHP/double(sRefHoplite().fHP);
    c->setHP(s.fHP*hpScale);
    c->setAttack(s.fAttack);
    c->setFigureStats(s.fArmor, s.fMissileArmor,
                      s.fMissileAttack, s.fMissileRate, s.fFrequency);
}

double eFigureStats::sDamage(const eCharacter* const attacker,
                             const eCharacter* const target,
                             const bool ranged, const int by) {
    if(!attacker->hasFigureStats()) return by*attacker->attack();
    const bool armored = target && target->hasFigureStats();
    const auto& h = sRefHoplite();
    const auto& r = sRefRabble();
    if(ranged && attacker->missileAttack() > 0) {
        // Missile damage less armor versus missiles (Caesar III engine
        // rule, as in melee). Calibrated so a Hero player rabble against a
        // Hero player hoplite deals eZeus's rabble attack; more often for
        // a lower rate of fire.
        const int ma = attacker->missileAttack();
        const double rate = attacker->missileRate() > 0 ?
                                double(r.fMissileRate)/attacker->missileRate() : 1.;
        if(armored) {
            const int net = std::max(0, ma - target->missileArmor());
            const int refNet = r.fMissileAttack - h.fMissileArmor;
            return by*rate*net*eNumbers::sRabbleAttack/refNet;
        }
        return by*rate*ma*eNumbers::sRabbleAttack/r.fMissileAttack;
    }
    // Melee: attack less armor, never below zero (Caesar III engine,
    // Julius src/figure/combat.c). Calibrated so a Hero player hoplite
    // against another one deals eZeus's hoplite attack.
    // Attack speed: "Frequency", lower strikes more often. The player's
    // units have 0 there and strike at the baseline, taken to be a Greek
    // hoplite's 60 (assumption: no source gives the baseline).
    const int frq = attacker->frequency();
    const double speed = frq > 0 ? 60./frq : 1.;
    const double att = speed*attacker->attack();
    if(armored) {
        const double net = std::max(0., att - target->armor());
        const int refNet = h.fAttack - h.fArmor;
        return by*net*eNumbers::sHopliteAttack/refNet;
    }
    // buildings and unarmored characters: plain attack, so a Hero player
    // hoplite damages them as fast as before
    return by*att*eNumbers::sHopliteAttack/h.fAttack;
}
