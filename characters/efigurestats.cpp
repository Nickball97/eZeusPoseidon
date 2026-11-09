#include "efigurestats.h"

#include "echaracter.h"
#include "efightingcharacter.h"
#include "engine/egameboard.h"
#include "engine/edifficulty.h"
#include "enumbers.h"

#include <algorithm>

namespace {
using eS = eFigureStats;

// A row of the file, in its own column order:
// HP, attack, armor, armor vs missiles, missile attack, missile range,
// missile rate of fire, speed, frequency.
eS row(const int hp, const int att, const int arm, const int avm,
       const int ma, const int mr, const int mrf, const int spd,
       const int frq = 0) {
    eS s;
    s.fHP = hp;
    s.fAttack = att;
    s.fArmor = arm;
    s.fMissileArmor = avm;
    s.fMissileAttack = ma;
    s.fMissileRange = mr;
    s.fMissileRate = mrf;
    s.fSpeed = spd;
    s.fFrequency = frq;
    return s;
}

// speed only: eZeus handles these characters' fights itself
eS speedOnly(const int spd) {
    eS s;
    s.fSpeed = spd;
    s.fCombat = false;
    return s;
}

// The file's speed table (value -> tiles relative to normal walking).
double speedValue(const int v) {
    static const double t[19] = {
        0., 1./4, 1./3, 1./2, 2./3, 3./4, 1., 5./4, 4./3, 3./2,
        5./3, 7./4, 2., 9./4, 7./3, 5./2, 8./3, 11./4, 3.};
    return t[std::clamp(v, 0, 18)];
}

// Rows that change with difficulty (Figure_model_VeryEasy/Easy/Normal/
// Hard/Impossible = Beginner/Mortal/Hero/Titan/Olympian).
eS playerRabble(const int d) { // "Rabble/Archer"
    const eS r[5] = {row(100, 5, 2, 4, 6, 10, 70, 8), row(90, 4, 1, 2, 6, 10, 75, 8),
                     row(80, 4, 0, 1, 6, 10, 80, 8), row(80, 4, 0, 1, 6, 10, 85, 8),
                     row(80, 3, 0, 1, 6, 10, 90, 8)};
    return r[d];
}
eS playerHoplite(const int d) { // "Hoplite/Spearman"
    const eS r[5] = {row(180, 15, 8, 6, 0, 0, 0, 6), row(160, 14, 6, 6, 0, 0, 0, 6),
                     row(140, 12, 6, 5, 0, 0, 0, 6), row(120, 12, 6, 5, 0, 0, 0, 6),
                     row(110, 12, 6, 5, 0, 0, 0, 6)};
    return r[d];
}
eS playerHorseman(const int d) { // "Horseman/Chariot"
    const eS r[5] = {row(300, 18, 7, 5, 0, 0, 0, 12), row(260, 16, 6, 4, 0, 0, 0, 12),
                     row(240, 15, 4, 4, 0, 0, 0, 12), row(200, 15, 4, 4, 0, 0, 0, 12),
                     row(180, 15, 4, 4, 0, 0, 0, 12)};
    return r[d];
}
eS wallSentry(const int d) {
    const eS r[5] = {row(100, 5, 0, 4, 8, 12, 50, 5), row(100, 5, 0, 4, 8, 11, 50, 5),
                     row(100, 5, 0, 4, 8, 10, 60, 5), row(100, 5, 0, 4, 8, 10, 65, 5),
                     row(100, 5, 0, 4, 8, 10, 75, 5)};
    return r[d];
}
eS trireme(const int d) {
    const eS r[5] = {row(300, 80, 5, 6, 15, 8, 50, 7), row(300, 80, 4, 6, 15, 8, 55, 7),
                     row(300, 80, 3, 6, 15, 8, 60, 7), row(300, 80, 2, 6, 15, 8, 60, 7),
                     row(300, 80, 2, 6, 15, 8, 65, 7)};
    return r[d];
}
eS wolf(const int d) {
    const eS r[5] = {row(15, 5, 0, 0, 0, 0, 0, 7), row(15, 5, 0, 0, 0, 0, 0, 7),
                     row(20, 5, 0, 0, 0, 0, 0, 7), row(25, 6, 0, 0, 0, 0, 0, 7),
                     row(30, 6, 0, 0, 0, 0, 0, 7)};
    return r[d];
}
eS cattle(const int d) {
    const int hp[5] = {100, 90, 80, 75, 60};
    return row(hp[d], 0, 5, 10, 0, 0, 0, 5);
}

// Calibration: a Hero-difficulty player hoplite keeps eZeus's strength.
const eS sRefHoplite = playerHoplite(2);
const eS sRefRabble = playerRabble(2);
}

const eFigureStats eFigureStats::sAmazonWarrior = row(120, 12, 4, 2, 0, 0, 0, 7, 25);
const eFigureStats eFigureStats::sAmazonArcher = row(80, 5, 2, 3, 8, 11, 75, 7, 75);

eFigureStats eFigureStats::sTowerSentry(const eDifficulty diff) {
    const int d = std::clamp(static_cast<int>(diff), 0, 4);
    const int ma[5] = {60, 60, 60, 50, 45};
    return row(120, 6, 0, 5, ma[d], 13, 100, 0);
}

eDifficulty eFigureStats::sDifficulty(eGameBoard& board) {
    return board.difficulty(board.personPlayer());
}

bool eFigureStats::sGet(const eCharacterType type,
                        const eDifficulty diff,
                        eFigureStats& s) {
    const int d = std::clamp(static_cast<int>(diff), 0, 4);
    using eT = eCharacterType;
    switch(type) {
    // the player's soldiers
    case eT::rockThrower: case eT::archerPoseidon: s = playerRabble(d); return true;
    case eT::hoplite: case eT::hoplitePoseidon: s = playerHoplite(d); return true;
    case eT::horseman: case eT::chariotPoseidon: s = playerHorseman(d); return true;

    // enemies ("ALL ENEMIES"), the same at every difficulty
    case eT::greekHoplite: s = row(150, 15, 6, 2, 0, 0, 0, 6, 60); return true;
    case eT::greekRockThrower: s = row(100, 5, 2, 3, 6, 11, 90, 6, 20); return true;
    case eT::greekHorseman: s = row(250, 17, 4, 4, 0, 0, 0, 11, 20); return true;
    case eT::trojanHoplite: s = row(180, 15, 6, 2, 0, 0, 0, 6, 50); return true; // Swordsman
    case eT::trojanSpearthrower: s = row(100, 5, 2, 3, 6, 11, 90, 6, 20); return true; // Javelineers
    case eT::trojanHorseman: s = row(200, 17, 4, 4, 0, 0, 0, 11, 30); return true;
    case eT::persianHoplite: s = row(150, 15, 6, 2, 0, 0, 0, 6, 30); return true;
    case eT::persianArcher: s = row(100, 5, 2, 3, 6, 11, 90, 6, 40); return true;
    case eT::persianHorseman: s = row(250, 17, 4, 4, 0, 0, 0, 11, 30); return true;
    case eT::centaurHorseman: s = row(150, 15, 6, 2, 0, 0, 0, 9, 50); return true; // Swordsman
    case eT::centaurArcher: s = row(100, 5, 2, 3, 6, 11, 90, 9, 50); return true;
    case eT::egyptianHoplite: s = row(150, 14, 4, 5, 0, 0, 0, 6, 50); return true;
    case eT::egyptianArcher: s = row(100, 5, 2, 2, 6, 10, 85, 6, 30); return true;
    case eT::egyptianChariot: s = row(250, 17, 6, 5, 0, 0, 0, 11, 20); return true;
    case eT::mayanHoplite: s = row(180, 16, 4, 2, 0, 0, 0, 7, 75); return true;
    case eT::mayanArcher: s = row(100, 5, 3, 3, 6, 9, 90, 7, 25); return true;
    case eT::phoenicianArcher: s = row(100, 6, 5, 5, 8, 11, 75, 5, 70); return true;
    case eT::phoenicianHorseman: s = row(250, 16, 7, 7, 0, 0, 0, 11, 30); return true;
    case eT::oceanidHoplite: s = row(200, 16, 8, 8, 0, 0, 0, 9, 50); return true;
    case eT::oceanidSpearthrower: s = row(200, 8, 4, 4, 7, 9, 90, 9, 50); return true;
    case eT::atlanteanHoplite: s = row(140, 13, 5, 6, 0, 0, 0, 6, 40); return true;
    case eT::atlanteanArcher: s = row(80, 4, 2, 2, 8, 11, 75, 6, 30); return true;
    case eT::atlanteanChariot: s = row(250, 18, 6, 4, 0, 0, 0, 11, 30); return true;
    case eT::aresWarrior: s = row(220, 16, 8, 8, 0, 0, 0, 8, 100); return true;
    case eT::enemyBoat: s = row(100, 0, 3, 7, 0, 0, 0, 6, 100); return true; // Transport

    // the player's other fighters and boats
    case eT::trireme: s = trireme(d); return true;
    case eT::archer: s = wallSentry(d); return true;
    case eT::watchman: s = row(20, 6, 1, 1, 0, 0, 0, 6); return true;
    case eT::hunter: s = row(50, 3, 1, 0, 4, 3, 75, 8); return true;
    case eT::eliteCitizen: s = row(25, 6, 2, 2, 0, 0, 0, 6); return true; // Nobleman
    case eT::shepherd: s = row(15, 6, 0, 0, 0, 0, 0, 6); return true;
    case eT::goatherd: s = row(15, 6, 0, 0, 0, 0, 0, 6); return true; // Goat Herder
    case eT::butcher: s = row(15, 4, 0, 0, 0, 0, 0, 6); return true; // Rancher
    case eT::lumberjack: s = row(10, 4, 0, 0, 0, 0, 0, 6); return true; // Wood Cutter
    case eT::actor: s = row(100, 7, 0, 0, 0, 0, 0, 6); return true;
    case eT::gymnast: s = row(25, 0, 0, 0, 0, 0, 0, 6); return true; // Athlete
    case eT::competitor: s = row(25, 0, 0, 0, 0, 0, 0, 6); return true;
    case eT::disgruntled: s = row(12, 0, 0, 0, 0, 0, 0, 6); return true;
    case eT::mugger:
    case eT::thief:
    case eT::vandal: s = row(10, 0, 0, 0, 0, 0, 0, 6); return true;
    case eT::sick: s = row(10, 0, 0, 0, 0, 0, 0, 1); return true; // Plagued Walker
    case eT::urchinGatherer: s = row(50, 0, 0, 8, 0, 0, 0, 6); return true;
    case eT::fishingBoat: s = row(50, 0, 0, 0, 0, 0, 0, 6); return true;
    case eT::tradeBoat: s = row(50, 0, 0, 0, 0, 0, 0, 11); return true; // Trade ship

    // everyday walkers: 10 hit points, normal speed
    case eT::settler: // Immigrant / Emigrant
    case eT::homeless:
    case eT::cartTransporter: // Cart pusher
    case eT::grower:
    case eT::taxCollector: // Clerk
    case eT::fireFighter: // Superintendent
    case eT::porter: // Storehouseman
    case eT::peddler: // Agora Vendors
    case eT::trader: // Caravan: Merchant
    case eT::donkey: // Caravan: Donkey
    case eT::philosopher:
    case eT::healer:
    case eT::waterDistributor: // Water Bearer
    case eT::artisan:
    case eT::marbleMiner: case eT::bronzeMiner:
    case eT::silverMiner: case eT::orichalcMiner: // Miner
    case eT::scholar: case eT::astronomer:
    case eT::inventor: case eT::curator:
        s = row(10, 0, 0, 0, 0, 0, 0, 6);
        return true;

    // animals
    case eT::sheep: s = row(3, 0, 0, 0, 0, 0, 0, 6); return true;
    case eT::goat: s = row(6, 0, 0, 0, 0, 0, 0, 5); return true;
    // Boar: speed from the file; hit points and attack are handled with
    // hunting (eZeus's 100 HP; attack 0, or 10 when angry, see
    // eHuntAction and sDamage)
    case eT::boar: s = speedOnly(7); return true;
    case eT::wolf: s = wolf(d); return true;
    case eT::cattle1: case eT::cattle2:
    case eT::cattle3: case eT::bull: s = cattle(d); return true;

    // gods, monsters and heroes: eZeus decides who they kill and that
    // nothing hurts them, so only their speed ("God", "Monster", "Hero")
    case eT::aphrodite: case eT::apollo: case eT::ares: case eT::artemis:
    case eT::athena: case eT::atlas: case eT::demeter: case eT::dionysus:
    case eT::hades: case eT::hephaestus: case eT::hera: case eT::hermes:
    case eT::poseidon: case eT::zeus:
    case eT::calydonianBoar: case eT::cerberus: case eT::chimera:
    case eT::cyclops: case eT::dragon: case eT::echidna: case eT::harpies:
    case eT::hector: case eT::hydra: case eT::kraken: case eT::maenads:
    case eT::medusa: case eT::minotaur: case eT::scylla: case eT::sphinx:
    case eT::talos:
    case eT::achilles: case eT::atalanta: case eT::bellerophon:
    case eT::hercules: case eT::jason: case eT::odysseus:
    case eT::perseus: case eT::theseus:
        s = speedOnly(7);
        return true;
    default:
        return false;
    }
}

void eFigureStats::sApply(eCharacter* const c, const bool setHP) {
    eFigureStats s;
    if(!sGet(c->type(), sDifficulty(c->getBoard()), s)) return;
    sApply(c, s, setHP);
}

void eFigureStats::sApply(eCharacter* const c, const eFigureStats& s,
                          const bool setHP) {
    if(s.fSpeed >= 0) c->setSpeed(speedValue(s.fSpeed));
    if(!s.fCombat) return;
    if(setHP) {
        const double hpScale = eNumbers::sHopliteHP/double(sRefHoplite.fHP);
        c->setHP(s.fHP*hpScale);
    }
    c->setAttack(s.fAttack);
    c->setFigureStats(s.fArmor, s.fMissileArmor, s.fMissileAttack,
                      s.fMissileRate, s.fFrequency, s.fMissileRange);
    if(s.fMissileRange > 0) {
        if(const auto f = dynamic_cast<eFightingCharacter*>(c)) {
            f->setRange(s.fMissileRange);
        }
    }
}

namespace {
// Missile damage less armor versus missiles (the Caesar III engine rule,
// as in melee). Calibrated so a Hero player rabble against a Hero player
// hoplite deals eZeus's rabble attack; more often for a lower rate.
double missileDamage(const int ma, const int rate,
                     const eCharacter* const target, const int by) {
    const double r = rate > 0 ? double(sRefRabble.fMissileRate)/rate : 1.;
    if(target && target->hasFigureStats()) {
        const int net = std::max(0, ma - target->missileArmor());
        const int refNet = sRefRabble.fMissileAttack - sRefHoplite.fMissileArmor;
        return by*r*net*eNumbers::sRabbleAttack/refNet;
    }
    return by*r*ma*eNumbers::sRabbleAttack/sRefRabble.fMissileAttack;
}
}

double eFigureStats::sDamage(const eCharacter* const attacker,
                             const eCharacter* const target,
                             const bool ranged, const int by) {
    // Hunting (per Nicolas): a hunter brings down a boar or a deer in
    // about 330 ticks, as eZeus did (0.3 a tick against 100 HP), whether
    // with spears or in a scuffle.
    if(target && attacker->type() == eCharacterType::hunter &&
       (target->type() == eCharacterType::boar ||
        target->type() == eCharacterType::deer)) {
        return 0.3*by;
    }
    // An angry boar (attack 10) hits for its attack less the target's
    // armor, like any melee figure.
    if(attacker->type() == eCharacterType::boar) {
        const int armor = target && target->hasFigureStats() ? target->armor() : 0;
        const double net = std::max(0., attacker->attack() - armor);
        const int refNet = sRefHoplite.fAttack - sRefHoplite.fArmor;
        return by*net*eNumbers::sHopliteAttack/refNet;
    }
    if(!attacker->hasFigureStats()) return by*attacker->attack();
    if(ranged && attacker->missileAttack() > 0) {
        return missileDamage(attacker->missileAttack(),
                             attacker->missileRate(), target, by);
    }
    // Melee: attack less armor, never below zero (Caesar III engine,
    // Julius src/figure/combat.c). Attack speed: "Frequency", lower
    // strikes more often; the player's units have 0 there and strike at
    // the baseline, taken to be a Greek hoplite's 60 (an assumption: no
    // source gives the baseline). Calibrated so a Hero player hoplite
    // against another one deals eZeus's hoplite attack.
    const int frq = attacker->frequency();
    const double speed = frq > 0 ? 60./frq : 1.;
    const double att = speed*attacker->attack();
    if(target && target->hasFigureStats()) {
        const double net = std::max(0., att - speed*target->armor());
        const int refNet = sRefHoplite.fAttack - sRefHoplite.fArmor;
        return by*net*eNumbers::sHopliteAttack/refNet;
    }
    // buildings and characters without figure data: plain attack, so a
    // Hero player hoplite damages them as fast as before
    return by*att*eNumbers::sHopliteAttack/sRefHoplite.fAttack;
}

double eFigureStats::sTowerDamage(eGameBoard& board,
                                  const eCharacter* const target,
                                  const int by) {
    const auto s = sTowerSentry(sDifficulty(board));
    return missileDamage(s.fMissileAttack, s.fMissileRate, target, by);
}
