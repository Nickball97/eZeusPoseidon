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
        multiCollectors,
        heroHallCurators,
        godHelpSecondResource,
        huntingLodgeHunters,
        domesticatedYearly,
        growerHarvesters,
        artisansSecond,
        sanctuaryGuardian,
        cityMoodCrime,
        cityPlague,
        eliteFailedChecks,
        buildingRiskCounters,
        worldCityRescue,
        cityPeaceMonths,
        invasionWarships,
        sanctuaryPriests,
        giftRescue,
        allowedSwitches,
        godCurseCount,
        sanctuaryActive,
        messageHistory,
        messagePanel,
        godCurseVisit,

        nextVersion
    };

    const int version = nextVersion - 1;
}

#endif // EFILEFORMAT_H
