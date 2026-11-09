#include "esmallhouse.h"
#include "ehousemodel.h"

#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "engine/eplague.h"

#include "characters/esick.h"
#include "characters/edisgruntled.h"
#include "characters/actions/esickdisgruntledaction.h"
#include "characters/actions/eoutlawaction.h"

#include "buildings/epalace.h"

#include "elanguage.h"
#include "enumbers.h"
#include "fileIO/efileformat.h"

#include <algorithm>

eSmallHouse::eSmallHouse(eGameBoard& board, const eCityId cid) :
    eHouseBase(board, eBuildingType::commonHouse, 2, 2,
               {8, 16, 24, 32, 40, 48, 60}, cid) {
    eGameTextures::loadCommonHouse();
    if(atlantean()) {
        eGameTextures::loadPoseidonCommonHouse();
    }
}

eSmallHouse::~eSmallHouse() {
    if(mSick) mSick->kill();
    if(mDisg) mDisg->kill();
    auto& board = getBoard();
    board.unregisterCommonHouse(this);
}

std::shared_ptr<eTexture> eSmallHouse::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    const auto& texs = blds[sizeId];

    if(mPeople == 0) return blds[sizeId].fHouseSpace;
    const eTextureCollection* coll = nullptr;
    if(atlantean()) {
        const int id = mLevel - 2;
        if(id < 0) {
            coll = &texs.fCommonHouse[mLevel];
        } else {
            coll = &texs.fPoseidonCommonHouse[id];
        }
    } else {
        coll = &texs.fCommonHouse[mLevel];
    }
    const int texId = seed() % coll->size();
    return coll->getTexture(texId);
}

int eSmallHouse::provide(const eProvide p, const int n) {
    if(mPeople <= 0) return 0;
    int max = 8;
    int* value = nullptr;
    switch(p) {
    case eProvide::water:
        value = &mWater;
        break;
    case eProvide::hygiene:
        value = &mHygiene;
        max = 100;
        break;
    case eProvide::food:
        value = &mFood;
        max = 2*mPeople;
        break;
    case eProvide::fleece:
        value = &mFleece;
        break;
    case eProvide::oil:
        value = &mOil;
        break;

    case eProvide::philosopherInventor:
        value = &mPhilosophers;
        break;
    case eProvide::actorAstronomer:
        value = &mActors;
        break;
    case eProvide::gymnastScholar:
        value = &mAthletes;
        break;
    case eProvide::competitorCurator:
        value = &mCompetitors;
        break;

    case eProvide::taxes: {
        if(mPaidTaxes) return 0;
        auto& b = getBoard();
        const auto cid = cityId();
        const auto p = b.palace(cid);
        if(!p || p->cursed()) return 0;
        const auto pid = playerId();
        const auto diff = b.difficulty(pid);
        const int taxMult = eDifficultyHelpers::taxMultiplier(
                                diff, type(), mLevel);
        const double tax = eNumbers::sCommonHousingTaxMulitplier *
                           mPeople * taxMult * b.taxRateF(cid);
        const int iTax = std::round(tax);
        b.payTaxes(cid, iTax, mPeople);
        mPaidTaxes = iTax;
        return iTax;
    }

    case eProvide::satisfaction: {
        // a watchman lowers crime by the guardpost's risk reducer (Model
        // BUILD_GUARDPOST column j = 50) each time he passes (original exe
        // 0x004DFB70 -> 0x004A5F70)
        reduceCrime(50);
        if(mSatisfactionProvidedThisMonth) return 0;
        mSatisfactionProvidedThisMonth = true;
        const int p = eNumbers::sWatchmanSatisfactionProvide;
        mSatisfaction = std::clamp(mSatisfaction + p, 0, 100);
        return 0;
    }
    default:
        return eBuilding::provide(p, n);
    }
    int add = 0;
    if(value && max > *value) {
        add = std::clamp(n, 0, max - *value);
        *value += add;
    }
    updateLevel();
    return add;
}

void eSmallHouse::timeChanged(const int by) {
    if(mPeople <= 0) {
        mHygiene = 100;
        if(mPlague) {
            auto& board = getBoard();
            board.healHouse(this);
        }
        setDisgruntled(false);
        mSatisfaction = 100;
        mCrime = 0;
        mFoodSatisfaction = 100;
        mWaterSatisfaction = 100;
        mWorkSatisfaction = 100;
        mTaxSatisfaction = 100;
        eHouseBase::timeChanged(by);
        return;
    }
    mUpdateCulture += by;
    if(mUpdateWater > eNumbers::sHouseWaterDecrementPeriod) {
        mUpdateWater = 0;
        mWater = std::max(0, mWater - 1);
    }
    mUpdateHygiene += by;
    if(mUpdateHygiene > eNumbers::sHouseHygieneDecrementPeriod) {
        mUpdateHygiene = 0;
        const int subWater = (100 - mWaterSatisfaction)/20;
        const int subFood = (100 - mFoodSatisfaction)/20;
        const int sub = 5 + subWater + subFood;
        mHygiene = std::max(0, mHygiene - sub);
    }

    auto& b = getBoard();
    // Plague (original exe 0x00447F70): a house stays sick for 32 cycles
    // (about 2 months), then recovers. Outbreaks start in eBoardCity.
    if(mPlague) {
        mPlagueTime += by;
        const int cycles32 = (32*365*eNumbers::sDayLength)/(12*16);
        if(mPlagueTime >= cycles32) {
            mPlagueTime = 0;
            b.healHouse(this);
        }
    }

    const auto cid = cityId();
    // Outlaws (original exe 0x005BFDE0): every cycle (16 a month), a
    // 1-in-200 draw for each crime threshold the house has reached, highest
    // first: 100 vandal, 90 thief, 80 mugger, 70 disgruntled citizen. Only
    // one outlaw from a house at a time.
    {
        mOutlawCycle += by;
        const double cycle = 365.*eNumbers::sDayLength/(12*16);
        while(mOutlawCycle >= cycle) {
            mOutlawCycle -= cycle;
            if(mPeople <= 0) continue;
            static const std::pair<int, eCharacterType> sOutlaws[4] = {
                {100, eCharacterType::vandal}, {90, eCharacterType::thief},
                {80, eCharacterType::mugger}, {70, eCharacterType::disgruntled}};
            for(const auto& o : sOutlaws) {
                if(mCrime < o.first) continue;
                if(mDisg) break;
                if(eRand::rand() % 200 == 0) spawnOutlaw(o.second);
            }
        }
    }

    {
        const auto s = b.sanctuary(cid, eGodType::aphrodite);
        const bool aphr = s && s->finished();
        if(!aphr) {
            const double pm = eNumbers::sHouseLeaveRiskPeriodMultiplier;
            const double pbi = eNumbers::sHouseLeaveRiskPeriodBaseIncrement;
            const double pe = eNumbers::sHouseLeaveRiskPeriodExponent;
            const int m4 = pm*pow(pbi + mSatisfaction, pe);
            const auto pid = playerId();
            const auto diff = b.difficulty(pid);
            const double leaveRisk = eDifficultyHelpers::crimeRisk(diff);
            if(leaveRisk > 0. && by) {
                const int leavePeriod = std::round(m4/(by*leaveRisk));
                if(leavePeriod && eRand::rand() % leavePeriod == 0) {
                    leave();
                }
            }
        }
    }

    mUpdateLevel += by;
    // checked every cycle (16 a month), as houses_evolve does
    const int lupdate = (365*eNumbers::sDayLength)/(12*16);
    if(mUpdateLevel > lupdate) {
        mUpdateLevel -= lupdate;
        updateLevel(true);
    }
    mUpdateSatisfaction += by;
    const int supdate = eNumbers::sHouseSatisfactionUpdatePeriod;
    if(mUpdateSatisfaction > supdate) {
        mUpdateSatisfaction = 0;
        updateSatisfaction();
    }

    if(mPlague && !mSick) {
        mSpawnSick += by;
        const int s = eNumbers::sHouseSickSpawnPeriod;
        if(mSpawnSick > s) {
            mSpawnSick = 0;
            spawnSick();
        }
    }

    eHouseBase::timeChanged(by);
}

void eSmallHouse::nextMonth() {
    mPaidTaxesLastMonth = mPaidTaxes;
    mPaidTaxes = 0;
    mSatisfactionProvidedThisMonth = false;
    const int cfood = round(mPeople*0.25);
    const int cfleece = (mLevel > 2 && mPeople > 0) ? 2 : 0;
    const int coil = (mLevel > 4 && mPeople > 0) ? 2 : 0;
    mFood = std::max(0, mFood - cfood);
    mFleece = std::max(0, mFleece - cfleece);
    mOil = std::max(0, mOil - coil);
}

void eSmallHouse::setPlague(const bool p) {
    mPlague = p;
    mSpawnSick = eRand::rand() % 10000;
}

void eSmallHouse::crimeUpdate(const int mood, const bool active,
                              const bool dionysus) {
    if(mPeople <= 0) {
        mCrime = 0;
    } else {
        const auto d = getBoard().difficulty(playerId());
        if(active) {
            // the original's special case (a god's sanctuary; taken to be
            // Dionysus, "no unrest") lowers crime by 20 instead
            if(dionysus) mCrime -= 20;
            else mCrime += eHouseModel::smallCrime(d, mLevel) + (40 - mood)/2;
        }
        mCrime = std::max(mCrime, eHouseModel::smallCrimeBase(d, mLevel));
        mCrime = std::min(mCrime, 100);
    }
    setDisgruntled(mCrime >= 70);
}

void eSmallHouse::reduceCrime(const int by) {
    if(mPeople <= 0) return;
    const auto d = getBoard().difficulty(playerId());
    mCrime = std::max(mCrime - by, eHouseModel::smallCrimeBase(d, mLevel));
    setDisgruntled(mCrime >= 70);
}

void eSmallHouse::setDisgruntled(const bool d) {
    mDisgruntled = d;
    mSpawnDisg = __INT_MAX__/2;
}

bool eSmallHouse::lowFood() const {
    if(!mFood) return true;
    const int cfood = round(mPeople*0.25);
    return mFood < cfood;
}

// Original house desirability by difficulty (eHouseModel): reaching level L
// needs smallEvolve(L); a house already at level L or above keeps it while
// appeal stays at or above smallDevolve(L).
bool eSmallHouse::appealFor(const int l) const {
    const double appeal = eHouseBase::appeal();
    const auto d = getBoard().difficulty(playerId());
    const int need = level() >= l ? eHouseModel::smallDevolve(d, l) :
                                    eHouseModel::smallEvolve(d, l);
    return appeal >= need - aphroditeAppealBonus();
}

bool eSmallHouse::cultureFor(const int l) const {
    return culturePoints() >= eHouseModel::smallCulture(l);
}

eHouseMissing eSmallHouse::missing() const {
    if(!appealFor(1)) return eHouseMissing::appeal;
    if(mFood <= 0) return eHouseMissing::food;
    if(!appealFor(2)) return eHouseMissing::appeal;
    if(mWater <= 0) return eHouseMissing::water;
    if(!cultureFor(2)) return eHouseMissing::venues;
    if(mFleece <= 0) return eHouseMissing::fleece;
    if(!appealFor(3)) return eHouseMissing::appeal;
    if(!cultureFor(4)) return eHouseMissing::venues;
    if(!appealFor(4)) return eHouseMissing::appeal;
    if(mOil <= 0) return eHouseMissing::oil;
    if(!appealFor(5)) return eHouseMissing::appeal;
    if(!cultureFor(6)) return eHouseMissing::venues;
    if(!appealFor(6)) return eHouseMissing::appeal;
    return eHouseMissing::nothing;
}

void eSmallHouse::read(eReadStream& src) {
    eHouseBase::read(src);

    src >> mSatisfactionProvidedThisMonth;
    src >> mUpdateSatisfaction;
    src >> mSatisfaction;
    src >> mFoodSatisfaction;
    src >> mWaterSatisfaction;
    src >> mWorkSatisfaction;
    src >> mTaxSatisfaction;

    src >> mUpdateWater;
    src >> mUpdateHygiene;
    src >> mUpdateLevel;

    src >> mWater;
    src >> mHygiene;
    src >> mPlague;
    src >> mDisgruntled;

    src >> mSpawnSick;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mSick = static_cast<eSick*>(c);
    });
    src >> mSpawnDisg;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mDisg = static_cast<eDisgruntled*>(c);
    });
    if(src.formatVersion() >= eFileFormat::cityMoodCrime) {
        src >> mCrime;
        src >> mOutlawCycle;
        src >> mFailedChecks;
    }
    if(src.formatVersion() >= eFileFormat::cityPlague) {
        src >> mPlagueTime;
    }
}

void eSmallHouse::write(eWriteStream& dst) const {
    eHouseBase::write(dst);

    dst << mSatisfactionProvidedThisMonth;
    dst << mUpdateSatisfaction;
    dst << mSatisfaction;
    dst << mFoodSatisfaction;
    dst << mWaterSatisfaction;
    dst << mWorkSatisfaction;
    dst << mTaxSatisfaction;

    dst << mUpdateWater;
    dst << mUpdateHygiene;
    dst << mUpdateLevel;

    dst << mWater;
    dst << mHygiene;
    dst << mPlague;
    dst << mDisgruntled;

    dst << mSpawnSick;
    dst.writeCharacter(mSick.get());
    dst << mSpawnDisg;
    dst.writeCharacter(mDisg.get());
    dst << mCrime;
    dst << mOutlawCycle;
    dst << mFailedChecks;
    dst << mPlagueTime;
}

std::string eSmallHouse::sName(const int level) {
    return eLanguage::zeusText(28, 2 + level);
}

void eSmallHouse::stepToward(const int target, const bool timed) {
    // Original houses (exe 0x004FE360...): a house moves one level at a
    // time, and only drops a level after failing its needs on 3 checks in
    // a row; a passing check resets the count. Only the regular check
    // counts failures; deliveries can only raise a house.
    if(target > mLevel) {
        mFailedChecks = 0;
        setLevel(mLevel + 1);
    } else if(target < mLevel) {
        if(!timed) return;
        if(++mFailedChecks >= 3) {
            mFailedChecks = 0;
            setLevel(mLevel - 1);
        }
    } else if(timed) {
        mFailedChecks = 0;
    }
}

void eSmallHouse::updateLevel(const bool timed) {
    if(mFood > 0 && appealFor(1)) {
        if(mWater > 0 && cultureFor(2) && appealFor(2)) {
            if(mFleece > 0 && appealFor(3)) {
                if(cultureFor(4) && appealFor(4)) {
                    if(mOil > 0 && appealFor(5)) {
                        if(cultureFor(6) && appealFor(6)) {
                            return stepToward(6, timed);
                        }
                        return stepToward(5, timed);
                    }
                    return stepToward(4, timed);
                }
                return stepToward(3, timed);
            }
            return stepToward(2, timed);
        }
        return stepToward(1, timed);
    }
    stepToward(0, timed);
}

void eSmallHouse::updateSatisfaction() {
    const int weight = 9;
    const int div = weight + 1;

    const int foodSat = mFood == 0 ? 0 : 100;
    mFoodSatisfaction = (weight*mFoodSatisfaction + foodSat)/div;

    const int waterSat = mWater == 0 ? 0 : 100;
    mWaterSatisfaction = (weight*mWaterSatisfaction + waterSat)/div;

    auto& board = getBoard();
    const auto cid = cityId();
    const auto empData = board.employmentData(cid);
    const double ef = empData ? empData->employedFraction() : 0.;
    const int workSat = 100*std::pow(ef, 4);
    mWorkSatisfaction = (weight*mWorkSatisfaction + workSat)/div;

    // Model\Tax_Sentiment_Model.txt: tax sentiment is city wide, by tax
    // rate and difficulty; if the city-wide tax coverage is very low, the
    // "None" row is used.
    auto taxRate = board.taxRate(cid);
    const auto city = board.boardCityWithId(cid);
    const double coverage = city ? city->taxCoverage() : 0.;
    if(coverage < eNumbers::sVeryLowTaxCoverage) taxRate = eTaxRate::none;
    const auto pid = playerId();
    const auto diff = board.difficulty(pid);
    const int ts = eDifficultyHelpers::taxSentiment(diff, taxRate);
    const int taxSat = std::round(100.*(ts + 7.)/14.);
    mTaxSatisfaction = (weight*mTaxSatisfaction + taxSat)/div;

    const int m1 = std::min(mFoodSatisfaction, mWaterSatisfaction);
    const int m2 = std::min(m1, mWorkSatisfaction);
    const int satMin = mTaxSatisfaction - 50;
    const int satMax = 50 + mTaxSatisfaction;
    const int sat = std::clamp(m2, satMin, satMax);
    mSatisfaction = (weight*mSatisfaction + sat)/div;
}

void eSmallHouse::spawnCharacter(const stdsptr<eCharacter>& c) {
    auto ts = surroundingRoad(false, true);
    eTile* tile = nullptr;
    if(ts.empty()) {
        tile = centerTile();
    } else {
        tile = ts[eRand::rand() % ts.size()];
    }
    c->changeTile(tile);
    if(!tile) return;
    const auto ct = c->type();
    if(ct == eCharacterType::mugger ||
       ct == eCharacterType::thief ||
       ct == eCharacterType::vandal) {
        const auto a = e::make_shared<eOutlawAction>(c.get(), this);
        c->setAction(a);
    } else {
        const auto a = e::make_shared<eSickDisgruntledAction>(c.get(), this);
        c->setAction(a);
    }
}

void eSmallHouse::spawnSick() {
    if(mSick) mSick->kill();
    const auto c = e::make_shared<eSick>(getBoard());
    mSick = c.get();
    spawnCharacter(c);
    const auto tile = c->tile();
    if(!tile) return;
    const auto cid = cityId();
    c->setBothCityIds(cid);
}

void eSmallHouse::spawnOutlaw(const eCharacterType type) {
    if(mDisg) mDisg->kill();
    // which outlaw comes from the crime level (see timeChanged)
    stdsptr<eDisgruntled> c;
    auto& board = getBoard();
    switch(type) {
    case eCharacterType::vandal: c = e::make_shared<eVandal>(board); break;
    case eCharacterType::thief: c = e::make_shared<eThief>(board); break;
    case eCharacterType::mugger: c = e::make_shared<eMugger>(board); break;
    default: c = e::make_shared<eDisgruntled>(board); break;
    }
    mDisg = c.get();
    const auto cid = cityId();
    mDisg->setOnCityId(cid);
    mDisg->setCityId(eCityId::neutralAggresive);
    spawnCharacter(c);
}
