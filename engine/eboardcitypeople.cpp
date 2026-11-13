// Part of eBoardCity: mood, health, plague, births, deaths and emigration.
// (Split from eboardcity.cpp on 13 Nov 2026; nothing but the file changed.)
#include "eboardcity.h"

#include "buildings/ebuilding.h"
#include "characters/echaracter.h"
#include "buildings/eemployingbuilding.h"
#include "buildings/estoragebuilding.h"
#include "buildings/esmallhouse.h"
#include "buildings/sanctuaries/esanctuary.h"
#include "engine/edifficulty.h"
#include "enumbers.h"
#include "buildings/eelitehousing.h"
#include "buildings/eheroshall.h"
#include "buildings/eagorabase.h"
#include "buildings/etradepost.h"
#include "buildings/eurchinquay.h"
#include "buildings/efishery.h"
#include "buildings/epier.h"
#include "buildings/eaestheticsbuilding.h"
#include "buildings/ehorseranch.h"
#include "buildings/ehorseranchenclosure.h"
#include "buildings/etriremewharf.h"
#include "buildings/pyramids/epyramid.h"

#include "evectorhelpers.h"

#include "gameEvents/einvasionevent.h"
#include "gameEvents/eplayerconquestevent.h"
#include "gameEvents/emonsterinvasioneventbase.h"
#include "gameEvents/egodvisitevent.h"
#include "gameEvents/earmyreturnevent.h"
#include "engine/emilitaryaid.h"
#include "spawners/ebanner.h"
#include "characters/esoldierbanner.h"
#include "einvasionhandler.h"

#include "elanguage.h"
#include "emessages.h"
#include "eeventdata.h"

#include "eplague.h"

#include "engine/eepisode.h"
#include "egameboard.h"

#include "eiteratesquare.h"
#include "etilehelper.h"

#include "spawners/espawner.h"
#include "buildings/ehippodrome.h"
#include "characters/gods/egod.h"
#include "epathfinder.h"
#include "eevent.h"

#include <algorithm>


void eBoardCity::startPlague(eSmallHouse* const h) {
    const auto plague = std::make_shared<ePlague>(mId, mBoard);
    plague->spreadFrom(h);
    mPlagues.push_back(plague);
}

stdsptr<ePlague> eBoardCity::plagueForHouse(eSmallHouse* const h) {
    if(!h) return nullptr;
    for(const auto& p : mPlagues) {
        if(p->hasHouse(h)) return p;
    }
    return nullptr;
}

void eBoardCity::healPlague(const stdsptr<ePlague>& p) {
    p->healAll();
    eVectorHelpers::remove(mPlagues, p);
}

const eBoardCity::ePlagues& eBoardCity::plagues() const {
    return mPlagues;
}

stdsptr<ePlague> eBoardCity::nearestPlague(
        const int tx, const int ty, int& dist) const {
    dist = __INT_MAX__/2;
    stdsptr<ePlague> result;
    for(const auto& p : mPlagues) {
        const auto& hs = p->houses();
        for(const auto h : hs) {
            const auto tt = h->centerTile();
            const int ttx = tt->x();
            const int tty = tt->y();
            const int d = sqrt((ttx - tx)*(ttx - tx) +
                               (tty - ty)*(tty - ty));
            if(d < dist) {
                dist = d;
                result = p;
            }
        }
    }
    return result;
}

eBuilding* eBoardCity::nearestEliteHouse(const int tx, const int ty) const {
    eBuilding* best = nullptr;
    int bestDist = __INT_MAX__;
    for(const auto b : mTimedBuildings) {
        if(b->type() != eBuildingType::eliteHousing) continue;
        const auto h = static_cast<eHouseBase*>(b);
        if(h->people() <= 0) continue;
        const auto ct = b->centerTile();
        if(!ct) continue;
        const int dx = ct->x() - tx;
        const int dy = ct->y() - ty;
        const int d = dx*dx + dy*dy;
        if(d < bestDist) {
            bestDist = d;
            best = b;
        }
    }
    return best;
}

// City mood, as the original game's sentiment (exe 0x005671A0, called
// twice a month; see mod-docs/original-exe-findings.md). Six factors are
// added each time, with a drift back toward the middle; house crime is
// updated with the mood from before the change.
void eBoardCity::updateMood() {
    const int pop = mPopData.population();
    if(pop >= 350) mMoodActive = true;
    const bool active = mMoodActive;
    const auto neg = [active](const int v) { return (v < 0 && !active) ? 0 : v; };

    const auto p = owningPlayer();
    const auto diff = p ? mBoard.difficulty(owningPlayerId()) : eDifficulty::hero;

    // taxes: the difficulty's tax sentiment table; under 11% tax coverage
    // the "no taxes" row is used (original exe 0x00567120)
    const auto taxRow = taxCoverage() < eNumbers::sVeryLowTaxCoverage ?
                            eTaxRate::none : mTaxRate;
    const int taxes = neg(eDifficultyHelpers::taxSentiment(diff, taxRow));
    // wages: none -10, very low -5, low -2, normal 0, high +2, very high +4
    static const int sWage[6] = {-10, -5, -2, 0, 2, 4};
    const int wages = neg(sWage[std::clamp(static_cast<int>(mWageRate), 0, 5)]);
    // unemployment
    int jobs = 0;
    {
        const int e = mEmplData.employable();
        const int u = mEmplData.unemployed();
        const int pct = e > 0 ? (100*u)/e : 0;
        if(pct < 5) jobs = 1;
        else if(pct < 11) jobs = 0;
        else if(pct < 18) jobs = -1;
        else if(pct < 26) jobs = -2;
        else jobs = -3;
        jobs = neg(jobs);
    }
    // food: average over lived-in houses; crime updated in the same pass
    const auto ds = sanctuary(eGodType::dionysus);
    const bool dionysus = ds && ds->active();
    int food = 0;
    {
        int sum = 0;
        int n = 0;
        for(const auto h : mHouses) {
            if(const auto sh = dynamic_cast<eSmallHouse*>(h)) {
                sh->crimeUpdate(mMood, active, dionysus);
            }
            if(h->people() <= 0) continue;
            sum += h->foodMoodFactor();
            n++;
        }
        if(n > 0) {
            food = sum/n;
            const int rem = std::abs(sum % n);
            if(rem > n/2) food += sum < 0 ? -1 : 1;
        }
        food = neg(food);
    }
    // debt: full years in debt (the original counts months in debt and
    // adds a year every 12, exe 0x00483B20; 0x00566EF0) and a negative
    // treasury
    int debt = 0;
    if(p && p->drachmas() < 0) {
        const auto& now = mBoard.date();
        const auto& since = p->inDebtSince();
        const int months = 12*(now.year() - since.year()) +
                           static_cast<int>(now.month()) -
                           static_cast<int>(since.month());
        const int years = months/12;
        if(years > 2) debt -= 4;
        if(years > 1) debt -= 3;
        if(years > 0) debt -= 2;
        debt -= 2;
    }
    // military service (original exe 0x00566E70): rabble, hoplite and
    // horseman companies called away from home lower mood in peacetime,
    // once there have been no enemies in the city for 2 months and no
    // invasion is due within 2 months (0x00483C10)
    int military = 0;
    if(mPeaceMonths >= 2) {
        bool invasionDue = false;
        const auto now = mBoard.date();
        for(const auto& e : mCityEvents.events()) {
            if(!e || e->type() != eGameEventType::invasion) continue;
            if(e->finished()) continue;
            const int days = e->nextDate() - now;
            if(days >= 0 && days < 60) {
                invasionDue = true;
                break;
            }
        }
        if(!invasionDue) {
            int n = 0;
            for(const auto& b : mSoldierBanners) {
                if(!b || b->isHome()) continue;
                const auto t = b->type();
                if(t == eBannerType::rockThrower ||
                   t == eBannerType::hoplite ||
                   t == eBannerType::horseman) n++;
            }
            military = n > 12 ? -5 : n > 8 ? -4 : n > 5 ? -3 :
                       n > 2 ? -2 : n > 0 ? -1 : 0;
        }
    }
    military = neg(military);

    const int sum = taxes + wages + jobs + food + debt + military;
    int drift;
    if(mMood < 11) drift = 4;
    else if(mMood < 21) drift = 3;
    else if(mMood < 31) drift = 2;
    else if(mMood < 41) drift = 1;
    else if(mMood < 61) drift = 0;
    else if(mMood < 71) drift = -1;
    else if(mMood < 81) drift = -2;
    else if(mMood < 91) drift = -3;
    else drift = -4;
    int change = sum;
    if(mMood < 41) {
        if(sum < 0) change = sum + drift > 0 ? 0 : sum + drift;
    } else if(mMood >= 61) {
        if(sum >= 0) change = sum + drift < 0 ? 0 : sum + drift;
    }
    mMood = std::clamp(mMood + change, 0, 100);
    // a working hippodrome changes the mood by its length (exe
    // 0x0056FDC0, table 0x006429F0): 4-98 +1, 100-198 +2, 200+ -5
    for(const auto& h : mHippodromes) {
        if(!h || !h->working()) continue;
        const int len = h->length();
        if(len <= 3) continue;
        if(len >= 200) mMood -= 5;
        else if(len >= 100) mMood += 2;
        else mMood += 1;
        break;
    }
    mMood = std::clamp(mMood, 0, 100);
}

// Monthly health target (original exe 0x00566C00): the population-weighted
// average of 50 for a recent healer visit plus 50 for water, plus a bonus
// for small cities (table 0x00641ED0), within 0-100.
void eBoardCity::updateHealthTarget() {
    long sum = 0;
    long people = 0;
    for(const auto h : mHouses) {
        const auto sh = dynamic_cast<eSmallHouse*>(h);
        if(!sh) continue;
        const int p = sh->people();
        if(p <= 0) continue;
        int v = 0;
        if(sh->hygiene() > 0) v += 50;
        if(sh->water() > 0) v += 50;
        sum += long(p)*v;
        people += p;
    }
    const int avg = people > 0 ? int(sum/people) : 0;
    const int pop = mPopData.population();
    static const int sPop[10][2] = {{100, 80}, {200, 80}, {300, 70},
                                    {400, 60}, {500, 50}, {600, 40},
                                    {700, 30}, {800, 20}, {900, 10},
                                    {1000, 0}};
    int bonus = 0;
    for(const auto& r : sPop) {
        if(pop < r[0]) {
            bonus = r[1];
            break;
        }
    }
    mHealthTarget = std::clamp(avg + bonus, 0, 100);
}

// Every cycle (16 a month): health moves one point toward its target, and
// an outbreak may start (original exe 0x00566B90, 0x00447FF0). The chance
// per 10,000 by health: <10 300, <20 250, <30 200, <45 100, <60 50,
// <75 25, else none (table 0x005F84A8); at most one a month and about
// chance*384/20000 a year.
void eBoardCity::healthCycle() {
    emigrationCycle();
    if(mCityHealth < mHealthTarget) mCityHealth++;
    else if(mCityHealth > mHealthTarget) mCityHealth--;
    static const int sBand[7][2] = {{10, 300}, {20, 250}, {30, 200},
                                    {45, 100}, {60, 50}, {75, 25},
                                    {100, 0}};
    int chance = 0;
    bool found = false;
    for(const auto& r : sBand) {
        if(mCityHealth < r[0]) {
            chance = r[1];
            found = true;
            break;
        }
    }
    if(!found || chance <= 0) return;
    if(mOutbreakThisMonth) return;
    int cap = (chance*384 + 9999)/10000;
    cap = (cap + 1)/2;
    if(mOutbreaksThisYear >= cap) return;
    if(eRand::rand() % 10000 >= chance) return;
    plagueOutbreak();
}

// An outbreak (original exe 0x004481B0, 0x00448490, 0x00448220): a fifth
// of the lived-in houses fall sick, those with the least health care first
// (a healer's recent visit, +20 for water), each with a coin toss until
// enough are chosen. A sick house loses a share of its people by
// difficulty: 25/33/50/67/100% (table 0x005F8480).
void eBoardCity::plagueOutbreak() {
    std::vector<eSmallHouse*> houses;
    for(const auto h : mHouses) {
        const auto sh = dynamic_cast<eSmallHouse*>(h);
        if(!sh || sh->people() <= 0 || sh->plague()) continue;
        houses.push_back(sh);
    }
    int need = int(houses.size())/5;
    if(need <= 0) return;
    mOutbreakThisMonth = true;
    mOutbreaksThisYear++;
    std::stable_sort(houses.begin(), houses.end(),
                     [](eSmallHouse* const a, eSmallHouse* const b) {
        const int sa = a->hygiene() + (a->water() > 0 ? 20 : 0);
        const int sb = b->hygiene() + (b->water() > 0 ? 20 : 0);
        return sa < sb;
    });
    const auto p = owningPlayer();
    const auto diff = p ? mBoard.difficulty(owningPlayerId()) : eDifficulty::hero;
    static const double sDie[5] = {0.25, 1./3, 0.5, 2./3, 1.};
    const double die = sDie[std::clamp(static_cast<int>(diff), 0, 4)];
    const auto plague = std::make_shared<ePlague>(mId, mBoard);
    int left = int(houses.size());
    bool first = true;
    for(const auto h : houses) {
        if(need <= 0) break;
        const bool take = left <= need || eRand::rand() % 2 == 0;
        left--;
        if(!take) continue;
        need--;
        h->kill(int(h->people()*die));
        plague->addHouse(h);
        if(first) {
            first = false;
            eEventData ed(mId);
            ed.fTile = h->centerTile();
            mBoard.event(eEvent::plague, ed);
        }
    }
    if(plague->houseCount() > 0) mPlagues.push_back(plague);
}

// Once a year the population grows or shrinks by a share set by city
// health (original exe 0x00566D40): 91+ +6%, 81-90 +4%, 71-80 +3%,
// 61-70 +2%, 51-60 +1%, 41-50 0; 40 or less: -1% while the health target
// is 31 or more, otherwise -2% (21-40), -3% (11-20), -5% (1-10).
// Hera's finished sanctuary adds 3. Newborns fill houses with room;
// deaths come out of the houses.
void eBoardCity::yearlyBirthsDeaths() {
    const int h = mCityHealth;
    int pct = 0;
    if(h >= 91) pct = 6;
    else if(h >= 81) pct = 4;
    else if(h >= 71) pct = 3;
    else if(h >= 61) pct = 2;
    else if(h >= 51) pct = 1;
    else if(h >= 41) pct = 0;
    else if(mHealthTarget >= 31) pct = -1;
    else if(h >= 21) pct = -2;
    else if(h >= 11) pct = -3;
    else if(h > 0) pct = -5;
    const auto hs = sanctuary(eGodType::hera);
    if(hs && hs->active()) pct += 3;
    const int pop = mPopData.population();
    int n = (pop*std::abs(pct))/100;
    if(n <= 0 || pct == 0) return;
    for(const auto house : mHouses) {
        if(n <= 0) break;
        if(house->people() <= 0) continue;
        if(pct > 0) n -= house->moveIn(n);
        else n -= house->kill(n);
    }
}

// Emigration, every cycle (original exe 0x00567450, 0x00486AB0): with
// 101+ people and the mood below 36, people leave: 1 a cycle at 26-35,
// 3 below 26, in groups of 4 from the lowest houses first (at most 4 from
// one house). Aphrodite's finished sanctuary stops it.
void eBoardCity::emigrationCycle() {
    const int pop = mPopData.population();
    int n = 0;
    if(mMood < 26) n = 3;
    else if(mMood < 36) n = 1;
    if(pop < 101) n = 0;
    const auto as = sanctuary(eGodType::aphrodite);
    if(as && as->active()) n = 0;
    if(n <= 0) {
        mEmigrantsPending = 0;
        return;
    }
    mEmigrantsPending += n;
    if(mEmigrantsPending < 4) return;
    int left = mEmigrantsPending;
    mEmigrantsPending = 0;
    std::vector<eHouseBase*> hs(mHouses.begin(), mHouses.end());
    std::stable_sort(hs.begin(), hs.end(),
                     [](eHouseBase* const a, eHouseBase* const b) {
        const bool ea = a->type() == eBuildingType::eliteHousing;
        const bool eb = b->type() == eBuildingType::eliteHousing;
        if(ea != eb) return !ea;
        return a->level() < b->level();
    });
    for(const auto h : hs) {
        if(left <= 0) break;
        if(h->people() <= 0) continue;
        left -= h->emigrate(std::min(4, left));
    }
}
