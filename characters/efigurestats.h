#ifndef EFIGURESTATS_H
#define EFIGURESTATS_H

class eCharacter;
enum class eCharacterType;
enum class eDifficulty;

// Combat strength of soldiers from the original game's
// Model\Figure_model_<difficulty>.txt (hit points, attack, armor, armor
// versus missiles, missile attack, missile rate of fire). Only the numbers
// are copied here, never the file.
//
// The original values are on their own scale; eZeus keeps its fighting
// pace by calibrating against a Hero-difficulty player hoplite (eZeus's
// 500 HP, 0.5 attack per tick against a hoplite of its own kind).
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

    // stats for this character type; false if the type has none
    static bool sGet(const eCharacterType type,
                     const eDifficulty diff,
                     eFigureStats& result);
    // sets HP and combat stats of c from the table (no-op if none)
    static void sApply(eCharacter* const c);
    static void sApply(eCharacter* const c, const eFigureStats& s);
    // Amazons are one character type with two kinds of soldier
    static const eFigureStats sAmazonWarrior;
    static const eFigureStats sAmazonArcher;

    // damage per tick dealt by attacker to target (target may be null for
    // a building); falls back to eZeus's own attack value for characters
    // without figure stats
    static double sDamage(const eCharacter* const attacker,
                          const eCharacter* const target,
                          const bool ranged, const int by);
};

#endif // EFIGURESTATS_H
