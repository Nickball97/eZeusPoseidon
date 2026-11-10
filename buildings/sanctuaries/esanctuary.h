#ifndef ESANCTUARY_H
#define ESANCTUARY_H

#include "../eemployingbuilding.h"
#include "../emonument.h"

#include "esanctuaryblueprint.h"
#include "characters/gods/egod.h"
#include "engine/edate.h"

class eBuildingTextures;
class eTempleAltarBuilding;
enum class eSacrifice;

enum class eHelpDenialReason {
    tooSoon,
    noTarget,
    error
};

class eSanctuary : public eMonument {
public:
    eSanctuary(eGameBoard& board,
               const eBuildingType type,
               const int sw, const int sh,
               const int maxEmployees,
               const eCityId cid);
    ~eSanctuary();

    void erase() override;

    void timeChanged(const int by) override;
    void nextMonth() override;

    void buildingProgressed() override;

    // Scales the pieces' marble and wood so the whole sanctuary costs the
    // original totals. Call once, after all pieces are registered.
    void applyOriginalMaterialTotals();

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;

    std::vector<eTile*> warriorTiles() const;
    void addWarriorTile(eTile* const t);
    void addSpecialTile(eTile* const t);

    bool askForAttack(const eCityId cid, eHelpDenialReason& reason);
    bool askForHelp(eHelpDenialReason& reason);

    eGod* god() const { return mGod; }

    eGodType godType() const;

    eGod* spawnGod();
    void spawnPatrolingGod();

    void godComeback();
    bool godAbroad() const { return mGodAbroad; }

    // Citadel of Poseidon (Atlantean Promontory): ModelZeus_Model_*.txt
    // BUILD_LARGE_SANC_POSEIDON2 (cost 2080, 80 workers, same on every
    // difficulty) and HeavenGames (initial marble 32)
    static constexpr int sCitadelCost = 2080;
    static constexpr int sCitadelInitialMarble = 32;
    static constexpr int sCitadelEmployees = 80;

    double helpTimeFraction() const;
    double helpAttackTimeFraction() const;

    // Zeus' Stronghold and the Oracle of Apollo have an Oracle (Zeus
    // manual, Consulting the Oracle); it waits as long as prayer does
    bool hasOracle() const;
    bool oracleReady() const;
    void oracleConsulted() { mOracleTimer = 0; }
    // shows the prediction (or that the Oracle is silent); a prediction
    // restarts the wait
    void consultOracle();
    double oracleTimeFraction() const;

    static stdsptr<eSanctuary> sCreate(
            const eBuildingType type,
            const int sw, const int sh,
            eGameBoard& board, const eCityId cid);

    static void sLoadMonumentTextures(const eGodType type);

    void setSpawnWait(const int w);

    // Poseidon's sanctuaries (Greek and Atlantean) send the Kraken to
    // patrol the city's waters (Zeus manual; Citadel per Nicolás)
    void spawnKraken();
    void spawnGuardian();

    bool sacrificing() const;

    // Priests and sacrifices, after the original game (zeus.exe
    // 0x004E97D0 sends priests, 0x004EA1A0 sacrifices, 0x004EA2B0 a
    // priest gone; see original-exe-findings.md):
    // - every cycle (16 a month) a finished, fully staffed sanctuary
    //   counts a timer down; while it is below 20 it sends a priest, up
    //   to 3 out at once for Zeus, Poseidon, Hades and Hera, 2 for
    //   Demeter, Apollo, Artemis, Athena and Atlas and 1 for the others
    // - a sacrifice sets the timer to 24 (the next priest leaves 5
    //   cycles later); above 24 it goes back to 22
    // - a priest at the altar waits until every other priest sent out
    //   has come back and no sacrifice is being shown; a sacrifice is
    //   shown until the sanctuary sends a priest into its last free slot
    eTempleAltarBuilding* altar() const;
    eTile* priestTile() const;
    int maxPriests() const;
    void priestArrived(eCharacter* const p);
    bool tryToSacrifice(const eSacrifice s);
    bool sacrificeShown() const { return mSacrificeShown; }
private:
    void priestCycle();
    void sendPriest();

    struct eSanctPriest {
        stdptr<eCharacter> fC;
        bool fArrived = false;
    };
    std::vector<eSanctPriest> mPriests;
    int mPriestTimer = 22;
    int mPriestCycleTime = 0;
    int mPriestsEnRoute = 0;
    bool mSacrificeShown = false;

    stdptr<eGod> mGod;
    bool mGodAbroad = false;
    int mWolfHuntCheck = 0; // not saved
    int mSpawnWait = 0;

    bool mAskedForHelp = false;
    int mCheckHelpNeeded = 0;
    int mHelpTimer = 10000000;
    int mOracleTimer = 10000000;
    stdptr<eCharacter> mKraken;
    int mKrakenWait = 0;
    // Ares' Dragon or Hades' Cerberus guarding the city
    stdptr<eCharacter> mGuardian;
    int mGuardianWait = 0;

    std::vector<eTile*> mWarriorTiles;
    std::vector<eTile*> mSpecialTiles;
};

#endif // ESANCTUARY_H
