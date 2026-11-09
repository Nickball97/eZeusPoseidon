#ifndef EMONSTERTARGET_H
#define EMONSTERTARGET_H

// What an adventure's monster event aims at (original adventure data,
// same order as the ids stored there).
enum class eMonsterTarget {
    food,
    sea,
    industry,
    military,
    money,
    troops,
    common,
    aesthetic,
    mythological,
    best,
    random
};

#endif // EMONSTERTARGET_H
