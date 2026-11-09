#ifndef EFILEFORMAT_H
#define EFILEFORMAT_H

namespace eFileFormat {
    enum {
        initial,
        settlerEmigrant,
        cartTarget,
        oracleTimer,
        soldierMorale,
        atlanteanFire,
        towerOrichalc,
        poseidonKraken,
        eliteNoble,
        monsterTarget,
        monsterTargets,

        nextVersion
    };

    const int version = nextVersion - 1;
}

#endif // EFILEFORMAT_H
