#ifndef EFIGURESTATS_H
#define EFIGURESTATS_H

class eCharacter;
class eGameBoard;
enum class eCharacterType;
enum class eDifficulty;

// Walkers, animals, soldiers, boats, gods, monsters and heroes from the
// original game's Model\Figure_model_<difficulty>.txt: hit points, attack,
// armor, armor versus missiles, missile attack, missile range, missile
// rate of fire, speed and melee frequency (attack speed). The enemy
// numbers and the speed table match HeavenGames' enemy-information page.
// Only the numbers are copied here, never the file.
//
// The original hit points and attacks are on their own scale; eZeus keeps
// its fighting pace by calibrating against a Hero-difficulty player
// hoplite (eZeus's 500 HP, 0.5 attack per tick against its own kind).
// Speed uses the file's speed table (6 = normal walking speed, which in
// eZeus is speed 1).
struct eFigureStats {
    int fHP = 0;
    int fAttack = 0;
    int fArmor = 0;
    int fMissileArmor = 0;
    int fMissileAttack = 0;
    int fMissileRate = 0; // lower is more often
    // melee attack speed ("Frequency", lower is more often); 0 for the
    // player's own units, which strike at the baseline speed
    int fFrequency = 0;
    int fMissileRange = 0; // tiles
    int fSpeed = -1; // index in the file's speed table, -1 = keep eZeus's
    // false: only speed is taken (e.g. eZeus handles the fight itself)
    bool fCombat = true;

    // stats for this character type; false if the type has none
    static bool sGet(const eCharacterType type,
                     const eDifficulty diff,
                     eFigureStats& result);
    // applies the table to c; setHP false keeps c's hit points (loading)
    static void sApply(eCharacter* const c, const bool setHP = true);
    static void sApply(eCharacter* const c, const eFigureStats& s,
                       const bool setHP = true);
    // Amazons are one character type with two kinds of soldier
    static const eFigureStats sAmazonWarrior;
    static const eFigureStats sAmazonArcher;
    // an invader's warship ("<Nation> Warship" rows, the same at every
    // difficulty); nationality as eNationality's number
    static eFigureStats sEnemyWarship(const int nationality);
    // the tower's own shooter ("Tower Sentry")
    static eFigureStats sTowerSentry(const eDifficulty diff);
    static eDifficulty sDifficulty(eGameBoard& board);

    // damage per tick dealt by attacker to target (target may be null for
    // a building); falls back to eZeus's own attack value for characters
    // without figure stats
    static double sDamage(const eCharacter* const attacker,
                          const eCharacter* const target,
                          const bool ranged, const int by);
    // damage per tick of a tower's missiles against target
    static double sTowerDamage(eGameBoard& board,
                               const eCharacter* const target,
                               const int by);
};

#endif // EFIGURESTATS_H
