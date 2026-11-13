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

eBoardCity::eBoardCity(const eCityId cid, eGameBoard& board) :
    mBoard(board),
    mId(cid),
    mCityEvents(cid, board),
    mCityPlan(cid),
    mHusbData(mPopData, *this, board),
    mEmplData(mPopData, *this, board),
    mEmplDistributor(mEmplData) {}

eBoardCity::~eBoardCity() {
    mCityEvents.clear();
}

void eBoardCity::updateTiles() {
    mTiles.clear();
    bool first = true;
    mBoard.iterateOverAllTiles([&](eTile* const tile) {
        const auto cid = tile->cityId();
        if(cid != mId) return;
        mTiles.push_back(tile);
        SDL_Rect tRect;
        tRect.x = tile->dx();
        tRect.y = tile->dy();
        tRect.w = 1;
        tRect.h = 1;
        if(first) {
            first = false;
            mTileBRect = tRect;
        } else {
            SDL_Rect result;
            SDL_UnionRect(&mTileBRect, &tRect, &result);
            mTileBRect = result;
        }
    });
    if(mTiles.empty()) clearTiles();
}

void eBoardCity::clearTiles() {
    mTiles.clear();
    mTileBRect = SDL_Rect{0, 0, 0, 0};
}

void eBoardCity::incTime(const int by) {
    mCoverageUpdate += by;
    const int cup = 2000;
    if(mCoverageUpdate > cup) {
        mCoverageUpdate -= cup;
        updateCoverage();
    }

    const auto& msgs = &eMessages::instance;
    eEventData ed(mId);
    const int pop = mPopData.population();
    if(pop >= 100 && !mPop100) {
        mBoard.showMessage(ed, msgs->fPop100);
        mPop100 = true;
    } else if(pop >= 500 && !mPop500) {
        mBoard.showMessage(ed, msgs->fPop500);
        mPop500 = true;
    } else if(pop >= 1000 && !mPop1000) {
        mBoard.showMessage(ed, msgs->fPop1000);
        allow(eBuildingType::commemorative, 0);
        mPop1000 = true;
    } else if(pop >= 2000 && !mPop2000) {
        mBoard.showMessage(ed, msgs->fPop2000);
        allow(eBuildingType::commemorative, 0);
        mPop2000 = true;
    } else if(pop >= 3000 && !mPop3000) {
        mBoard.showMessage(ed, msgs->fPop3000);
        mPop3000 = true;
    } else if(pop >= 5000 && !mPop5000) {
        mBoard.showMessage(ed, msgs->fPop5000);
        allow(eBuildingType::commemorative, 0);
        mPop5000 = true;
    } else if(pop >= 10000 && !mPop10000) {
        mBoard.showMessage(ed, msgs->fPop10000);
        allow(eBuildingType::commemorative, 0);
        mPop10000 = true;
    } else if(pop >= 15000 && !mPop15000) {
        mBoard.showMessage(ed, msgs->fPop15000);
        allow(eBuildingType::commemorative, 0);
        mPop15000 = true;
    } else if(pop >= 20000 && !mPop20000) {
        mBoard.showMessage(ed, msgs->fPop20000);
        allow(eBuildingType::commemorative, 0);
        mPop20000 = true;
    } else if(pop >= 25000 && !mPop25000) {
        mBoard.showMessage(ed, msgs->fPop25000);
        allow(eBuildingType::commemorative, 0);
        mPop25000 = true;
    }

    // city health moves and outbreaks are checked every cycle
    mHealthCycle += by;
    const int healthPeriod = (365*eNumbers::sDayLength)/(12*16);
    while(mHealthCycle >= healthPeriod) {
        mHealthCycle -= healthPeriod;
        healthCycle();
    }

    // the original updates the city mood twice a month
    mMoodTimer += by;
    const int moodPeriod = (365*eNumbers::sDayLength)/24;
    if(mMoodTimer > moodPeriod) {
        mMoodTimer -= moodPeriod;
        updateMood();
    }

    const int updateResWait = 1000;
    mUpdateResources += by;
    if(mUpdateResources > updateResWait) {
        mUpdateResources -= updateResWait;
        updateResources();
    }

    const int food = resourceCount(eResourceType::food);
    bool prolongedNoFood = false;
    const auto date = mBoard.date();
    if(mNoFood && food < 1 && pop > 10) {
        prolongedNoFood = date.year() - mNoFoodSince.year() > 1;
    } else if(food < 1 && pop > 10) {
        mNoFood = true;
        mNoFoodSince = date;
    } else {
        mNoFood = false;
    }
    const int u = mEmplData.unemployed();
    const int e = mEmplData.employable();
    const int v = mPopData.vacancies();
    const auto oldLimit = mImmigrationLimit;
    const auto p = owningPlayer();
    int drachmas = 0;
    eDate inDebtSince = date;
    if(p) {
        drachmas = p->drachmas();
        inDebtSince = p->inDebtSince();
    }
    if(prolongedNoFood) {
        mImmigrationLimit = eILB::lackOfFood;
    } else if(drachmas < 0 && date.year() - inDebtSince.year() > 1) {
        mImmigrationLimit = eILB::prolongedDebt;
    } else if(static_cast<int>(mWageRate) < static_cast<int>(eWageRate::low)) {
        mImmigrationLimit = eILB::lowWages;
    } else if(e < 10 ? false : (1.*u/e > 0.25 && u > 50)) {
        mImmigrationLimit = eILB::unemployment;
    } else if(static_cast<int>(mTaxRate) > static_cast<int>(eTaxRate::high)) {
        mImmigrationLimit = eILB::highTaxes;
    } else if(mExcessiveMilitaryServiceCount > 0) {
        mImmigrationLimit = eILB::excessiveMilitaryService;
    } else if(v <= 0) {
        mImmigrationLimit = eILB::lackOfVacancies;
    } else {
        mImmigrationLimit = eILB::none;
    }
    if(oldLimit != mImmigrationLimit) {
        switch(mImmigrationLimit) {
        case eILB::lackOfFood:
            mBoard.showTip(mId, eLanguage::zeusText(19, 112));
            break;
        case eILB::prolongedDebt:
            mBoard.showTip(mId, eLanguage::zeusText(19, 116));
            break;
        case eILB::lowWages:
            mBoard.showTip(mId, eLanguage::zeusText(19, 115));
            break;
        case eILB::unemployment:
            mBoard.showTip(mId, eLanguage::zeusText(19, 113));
            break;
        case eILB::highTaxes:
            mBoard.showTip(mId, eLanguage::zeusText(19, 114));
            break;
        case eILB::excessiveMilitaryService:
            mBoard.showTip(mId, eLanguage::zeusText(19, 117));
            break;
        case eILB::lackOfVacancies:
            mBoard.showTip(mId, eLanguage::zeusText(19, 111));
            break;
        case eILB::none:
            mBoard.showTip(mId, eLanguage::zeusText(19, 124));
        }
    }

    for(const auto i : mInvasionHandlers) {
        i->incTime(by);
    }

    for(const auto& b : mPalaceSoldierBanners) {
        b->incTime(by); // morale
    }

    mCityEvents.handleNewDate(date);

    if(p && !p->isPerson()) {
        const int lookForCityDefenseCheck = 5000;
        mLookForCityDefense += by;
        if(mLookForCityDefense > lookForCityDefenseCheck) {
            mLookForCityDefense -= lookForCityDefenseCheck;
            updateCityDefense();
        }
    }

    if(mCutOffCheckBuildingsState != mAllBuildingsState) {
        mCutOffCheckBuildingsState = mAllBuildingsState;
        const auto entry = entryPoint();
        const auto exit = exitPoint();
        if(entry && exit) {
            const int eX = exit->x();
            const int eY = exit->y();
            ePathFinder p([](eTileBase* const t) {
                return t->walkableTerrain();
            }, [&](eTileBase* const t) {
                return t->x() == eX && t->y() == eY;
            });
            const auto distance = [](eTileBase* const t) {
                const auto type = t->underBuildingType();
                // palace grounds: not walkable since #53, but never knocked down
                const bool w = eBuilding::sWalkableBuilding(type) ||
                               type == eBuildingType::palaceTile;
                if(!w) return 200;
                return 1;
            };
            const int w = mBoard.width();
            const int h = mBoard.height();
            const bool r = p.findPath({0, 0, w, h}, entry, 1000, true, w, h, distance);
            if(r) {
                std::vector<eTile*> path;
                p.extractPath(path, mBoard);
                for(const auto t : path) {
                    const auto type = t->underBuildingType();
                    const bool w = eBuilding::sWalkableBuilding(type) ||
                                   type == eBuildingType::palaceTile;
                    if(w) continue;
                    const auto ub = t->underBuilding();
                    ub->collapse();

                    eEventData ed;
                    ed.fTile = t;
                    mBoard.event(eEvent::areaCutOff, ed);
                }
            }
        }
    }
}

void eBoardCity::acquired() {
    mCityEvents.setupStartDate(mBoard.date());
}

void eBoardCity::updateCoverage() {
    int totalPeople = 0;
    int totalBuildings = 0;
    int commonPeople = 0;
    int sport = 0;
    int phil = 0;
    int drama = 0;
    int taxes = 0;
    double totalUnrest = 0;
    int totalSatisfaction = 0;
    int totalHygiene = 0;
    int totalMaintanance = 0;
    for(const auto b : mTimedBuildings) {
        if(const auto h = dynamic_cast<eHouseBase*>(b)) {
            const int p = h->people();
            if(h->athletesScholars() > 0) {
                sport += p;
            }
            if(h->philosophersInventors() > 0) {
                phil += p;
            }
            if(h->actorsAstronomers() > 0) {
                drama += p;
            }
            if(h->paidTaxes()) {
                taxes += p;
            }
            if(const auto ch = dynamic_cast<eSmallHouse*>(b)) {
                if(ch->disgruntled()) totalUnrest += p;
                totalSatisfaction += p*ch->satisfaction();
                totalHygiene += p*ch->hygiene();
                commonPeople += p;
            }
            totalPeople += p;
        }
        totalBuildings++;
        totalMaintanance += b->maintenance();
    }
    if(totalPeople <= 0) {
        mAthleticsCoverage = 0;
        mPhilosophyCoverage = 0;
        mDramaCoverage = 0;
        mTaxesCoverage = 0;
    } else {
        mAthleticsCoverage = std::round(100.*sport/totalPeople);
        mPhilosophyCoverage = std::round(100.*phil/totalPeople);
        mDramaCoverage = std::round(100.*drama/totalPeople);
        mTaxesCoverage = std::round(100.*taxes/totalPeople);
    }
    mAllDiscCoverage = (mAthleticsCoverage + mPhilosophyCoverage + mDramaCoverage)/3;

    if(commonPeople == 0) {
        mUnrest = 0;
        mPopularity = 100;
        mHealth = 100;
        mMaintanance = 100;
    } else {
        mUnrest = std::round(100.*totalUnrest/commonPeople);
        // popularity is the city mood (the original's sentiment)
        mPopularity = mMood;
        // the health rating is the original's city health
        mHealth = mCityHealth;
        mMaintanance = std::round(1.*totalMaintanance/totalBuildings);
    }
}

void eBoardCity::payTaxes(const int d, const int people) {
    mTaxesPaidThisYear += d;
    mPeoplePaidTaxesThisYear += people;
}

double eBoardCity::taxCoverage() const {
    const auto date = mBoard.date();
    const int m = 12*date.year() + static_cast<int>(date.month());
    if(m == mTaxCoverageMonth) return mTaxCoverage;
    mTaxCoverageMonth = m;
    int total = 0;
    int paid = 0;
    for(const auto h : mHouses) {
        const int p = h->people();
        total += p;
        if(h->paidTaxesLastMonth() > 0) paid += p;
    }
    mTaxCoverage = total == 0 ? 0. : double(paid)/total;
    return mTaxCoverage;
}

void eBoardCity::nextYear() {
    mTaxesPaidLastYear = mTaxesPaidThisYear;
    mTaxesPaidThisYear = 0;
    mPeoplePaidTaxesLastYear = mPeoplePaidTaxesThisYear;
    mPeoplePaidTaxesThisYear = 0;
}

void eBoardCity::payPensions() {
    const auto p = owningPlayer();
    const int d = std::ceil(mEmplData.pensions()/12.);
    if(p) p->incDrachmas(-d, eFinanceTarget::wages);
}

bool eBoardCity::replace3By3AestheticByCommemorative() {
    int id = -1;
    for(int i = 0; i <= 8; i++) {
        const bool a = mAvailableBuildings.available(
                           eBuildingType::commemorative, i);
        if(a) {
            id = i;
            break;
        }
    }
    if(id == -1) return false;
    double mostMissingAppeal = 0.;
    eBuilding* toReplace = nullptr;
    for(const auto b : m3x3AestheticBuildings) {
        const auto type = b->type();
        switch(type) {
        case eBuildingType::hedgeMaze:
        case eBuildingType::dolphinSculpture:
        case eBuildingType::orrery:
        case eBuildingType::spring:
        case eBuildingType::topiary: {
            const int did = b->districtId();
            if(did == -1) continue;
            const auto rect = b->tileRect();
            const int range = 3;
            for(int x = rect.x - range; x < rect.x + rect.w + range; x++) {
                for(int y = rect.y - range; y < rect.y + rect.h + range; y++) {
                    const SDL_Point p{x, y};
                    const bool in = SDL_PointInRect(&p, &rect);
                    if(in) continue;
                    int dx;
                    int dy;
                    eTileHelper::tileIdToDTileId(x, y, dx, dy);
                    const auto tile = mBoard.dtile(dx, dy);
                    if(!tile) continue;
                    const auto ub = tile->underBuilding();
                    if(!ub) continue;
                    if(const auto h = dynamic_cast<eHouseBase*>(ub)) {
                        const double goodAppeal =
                                type == eBuildingType::eliteHousing ?
                                    80. : 30.;
                        const double a = h->appeal();
                        const double missing = goodAppeal - a;
                        if(missing > mostMissingAppeal) {
                            mostMissingAppeal = missing;
                            toReplace = b;
                        }
                    }
                }
            }
        } break;
        default:
            break;
        }
    }
    if(!toReplace) return false;
    const auto b = toReplace;
    const auto rect = b->tileRect();

    const int did = b->districtId();
    b->setDistrictId(-1);
    b->erase();
    const auto bc = [&]() {
        const auto c = e::make_shared<eCommemorative>(id, mBoard, mId);
        c->setDistrictId(did);
        return c;
    };

    const auto pid = mBoard.cityIdToPlayerId(mId);
    const bool r = mBoard.buildBase(rect.x, rect.y,
                                    rect.x + rect.w - 1,
                                    rect.y + rect.h - 1,
                                    bc, pid, mId, true);
    if(r) mAvailableBuildings.built(eBuildingType::commemorative, id);
    return r;
}

void eBoardCity::updateResourceTiles() {
    mResourceTiles.clear();
    for(const auto tile : mTiles) {
        if(tile->hasFish() || tile->hasUrchin()) {
            mResourceTiles.push_back(tile);
            continue;
        }
        const auto terr = tile->terrain();
        switch(terr) {
        case eTerrain::marble:
        case eTerrain::blackMarble:
        case eTerrain::copper:
        case eTerrain::silver:
        case eTerrain::orichalc:
        case eTerrain::forest:
        case eTerrain::choppedForest:
            mResourceTiles.push_back(tile);
            break;
        default:
            break;
        }
    }
}

void eBoardCity::updateForestTiles() {
    mForestTiles.clear();
    for(const auto tile : mTiles) {
        const auto terr = tile->terrain();
        switch(terr) {
        case eTerrain::forest:
        case eTerrain::choppedForest:
            mForestTiles.push_back(tile);
            break;
        default:
            break;
        }
    }
}

void eBoardCity::nextMonth() {
    payPensions();

    // the original also counts enemy ships still in the city's waters
    // (figures 63-64, 0x00483C10)
    bool enemyShips = false;
    for(const auto c : mBoard.characters()) {
        if(c->type() != eCharacterType::enemyWarship) continue;
        if(c->onCityId() != mId || c->dead()) continue;
        enemyShips = true;
        break;
    }
    if(hasActiveInvasions() || enemyShips) mPeaceMonths = 0;
    else mPeaceMonths = std::min(mPeaceMonths + 1, 1000);

    updateHealthTarget();
    mOutbreakThisMonth = false;
    if(mBoard.date().month() == eMonth::january) {
        mOutbreaksThisYear = 0;
        yearlyBirthsDeaths();
    }

    {
        int nTot = 0;
        int nCalled = 0;
        for(const auto& s : mPalaceSoldierBanners) {
            const bool a = s->isAbroad();
            if(a) continue;
            const int c = s->count();
            nTot += c;
            const bool h = s->isHome();
            if(h) continue;
            nCalled += c;
        }
        if(nTot == 0 || static_cast<double>(nCalled)/nTot < 0.25) {
            mExcessiveMilitaryServiceCount--;
            mMonthsOfMilitaryService = 0;
        } else {
            mMonthsOfMilitaryService++;
            if(mMonthsOfMilitaryService > 6) {
                mExcessiveMilitaryServiceCount = 2;
            }
        }
    }

    mPopData.nextMonth();
    for(const auto& h : mHippodromes) {
        h->nextMonth();
    }
    hippodromeStanding();

    replace3By3AestheticByCommemorative();

    const auto date = mBoard.date();
    const bool pp = personPlayerOwner();
    if(!pp && !mDefending) {
        const int space = maxPalaceBannerCount();
        if(space == sPalaceTiles) {
            if(mNextAttackPlanned) {
                if(date < mNextAttackDate) {
                    return;
                }
            } else {
                mNextAttackPlanned = true;
                mNextAttackDate = date;
                mNextAttackDate.nextMonths(eNumbers::sAIInvasionMonthsBreak);
                return;
            }
            const int soldiers = mMaxHoplites + mMaxHorsemen;
            const int missingArmor = missingArmorFromEliteHouses();
            const auto pid = owningPlayerId();
            const auto tid = mBoard.playerIdToTeamId(pid);
            if(soldiers > 20 && missingArmor == 0) {
                auto enemyCids = mBoard.enemyCidsOnBoard(tid);
                if(!enemyCids.empty()) {
                    eRand::randomShuffle(enemyCids);
                    const auto targetCid = enemyCids[0];
                    const auto& wboard = mBoard.world();
                    const auto wc = wboard.cityWithId(targetCid);
                    eEnlistedForces forces;
                    for(const auto& b : mSoldierBanners) {
                        const auto type = b->type();
                        if(type == eBannerType::rockThrower) continue;
                        forces.fSoldiers.push_back(b);
                    }
                    for(const auto hh : mHeroHalls) {
                        const auto stage = hh->stage();
                        if(stage != eHeroSummoningStage::arrived) continue;
                        const bool abroad = hh->heroOnQuest();
                        if(abroad) continue;
                        const auto type = hh->heroType();
                        forces.fHeroes.push_back({mId, type});
                    }
                    mBoard.enlistForces(forces);
                    const auto e = e::make_shared<ePlayerConquestEvent>(
                                       mId, eGameEventBranch::root, mBoard);
                    const auto boardDate = mBoard.date();
                    const int period = eNumbers::sArmyTravelTime;
                    const auto date = boardDate + period;
                    e->setWarningMonths(period/31);
                    e->initializeDate(date, period, 1);
                    e->initialize(date, forces, wc);
                    mBoard.addRootGameEvent(e);
                    mNextAttackPlanned = false;
                }
            }
        }
    }
}

double eBoardCity::taxRateF() const {
    return eTaxRateHelpers::getRate(mTaxRate);
}

void eBoardCity::setTaxRate(const eTaxRate tr) {
    mTaxRate = tr;
}

void eBoardCity::setWageRate(const eWageRate wr) {
    mWageRate = wr;
    distributeEmployees();
}

bool eBoardCity::supportsBuilding(const eBuildingMode mode) const {
    // the scenario's roadblock switch (zeus.exe 0x00402B80, switch 26)
    if(mode == eBuildingMode::roadblock) {
        return !mAvailableBuildings.fRoadblockOff;
    }
    const auto type = eBuildingModeHelpers::toBuildingType(mode);
    int id = -1;
    switch(mode) {
    case eBuildingMode::populationMonument:
    case eBuildingMode::victoryMonument:
    case eBuildingMode::colonyMonument:
    case eBuildingMode::athleteMonument:
    case eBuildingMode::conquestMonument:
    case eBuildingMode::happinessMonument:
    case eBuildingMode::heroicFigureMonument:
    case eBuildingMode::diplomacyMonument:
    case eBuildingMode::scholarMonument:
        id = eBuildingModeHelpers::toCommemorativeId(mode);
        break;
    case eBuildingMode::aphroditeMonument:
    case eBuildingMode::apolloMonument:
    case eBuildingMode::aresMonument:
    case eBuildingMode::artemisMonument:
    case eBuildingMode::athenaMonument:
    case eBuildingMode::atlasMonument:
    case eBuildingMode::demeterMonument:
    case eBuildingMode::dionysusMonument:
    case eBuildingMode::hadesMonument:
    case eBuildingMode::hephaestusMonument:
    case eBuildingMode::heraMonument:
    case eBuildingMode::hermesMonument:
    case eBuildingMode::poseidonMonument:
    case eBuildingMode::zeusMonument: {
        const auto gt = eBuildingModeHelpers::toGodType(mode);
        id = static_cast<int>(gt);
    } break;
    default:
        break;
    }

    return availableBuilding(type, id);
}

bool eBoardCity::availableBuilding(const eBuildingType type,
                                   const int id) const {
    switch(type) {
    case eBuildingType::chariotVendor:
    case eBuildingType::chariotFactory: {
        if(!mAtlantean) return false;
        // zeus.exe also needs the horse ranch switch (0x00402B80)
        if(type == eBuildingType::chariotFactory &&
           !mAvailableBuildings.available(eBuildingType::horseRanch)) {
            return false;
        }
    } break;
    case eBuildingType::horseTrainer: {
        if(mAtlantean) return false;
    } break;
    default:
        break;
    }

    return mAvailableBuildings.available(type, id);
}

void eBoardCity::built(const eBuildingType type,
                       const int id) {
    mAvailableBuildings.built(type, id);
    mBoard.updateButtonsVisibility();
}

void eBoardCity::destroyed(const eBuildingType type,
                           const int id) {
    if(!mBoard.registerBuildingsEnabled()) return;
    mAvailableBuildings.destroyed(type, id);
    mBoard.updateButtonsVisibility();
}

void eBoardCity::allow(const eBuildingType type,
                       const int id) {
    mAvailableBuildings.allow(type, id);
    mBoard.updateButtonsVisibility();
}

void eBoardCity::allowPyramid(const eBuildingType type,
                              const std::vector<bool>& levels) {
    mAvailableBuildings.allowPyramid(type, levels);
    mBoard.updateButtonsVisibility();
}

void eBoardCity::disallow(const eBuildingType type,
                          const int id) {
    mAvailableBuildings.disallow(type, id);
    mBoard.updateButtonsVisibility();
}

void eBoardCity::incPopulation(const int by) {
    mPopData.incPopulation(by);
    mEmploymentUpdateScheduled = true;
}

std::vector<eBuilding*> eBoardCity::buildings(const eBuildingValidator& v) const {
    std::vector<eBuilding*> result;
    for(const auto b : mTimedBuildings) {
        const bool r = v(b);
        if(r) result.push_back(b);
    }
    return result;
}

std::vector<eBuilding*> eBoardCity::buildings(const eBuildingType type) const {
    return buildings([type](eBuilding* const b) {
        const auto bt = b->type();
        return type == bt;
    });
}

int eBoardCity::countBuildings(const eBuildingValidator& v) const {
    const auto vv = buildings(v);
    return vv.size();
}

int eBoardCity::countBuildings(const eBuildingType t) const {
    return countBuildings([t](eBuilding* const b) {
        const auto bt = b->type();
        return bt == t;
    });
}

bool eBoardCity::hasBuilding(const eBuildingType t) const {
    for(const auto b : mTimedBuildings) {
        const bool r = t == b->type();
        if(r) return true;
    }
    return false;
}

int eBoardCity::countAllowed(const eBuildingType t) const {
    eBuildingType parent;

    if(t == eBuildingType::sheep) {
        parent = eBuildingType::cardingShed;
    } else if(t == eBuildingType::goat) {
        parent = eBuildingType::dairy;
    } else if(t == eBuildingType::cattle) {
        parent = eBuildingType::corral;
    } else {
        return 0;
    }
    const int already = countBuildings(t);

    return 8*countBuildings(parent) - already;
}

eBuilding* eBoardCity::randomBuilding(const eBuildingValidator& v) const {
    auto blds = mTimedBuildings;
    eRand::randomShuffle(blds);
    for(const auto b : blds) {
        const bool r = v(b);
        if(r) return b;
    }
    return nullptr;
}

eTile* eBoardCity::randomTile() const {
    if(mTiles.empty()) return nullptr;
    const int id = eRand::rand() % mTiles.size();
    return mTiles[id];
}

std::vector<bool> eBoardCity::pyramidLevels(const eBuildingType type) const {
    return mAvailableBuildings.pyramidLevels(type);
}

namespace {
    // zeus.exe counts buildings of each type that have workers
    // (0x00782888, filled each cycle)
    int countWorking(const eBoardCity& c, const eBuildingType t) {
        return c.countBuildings([t](eBuilding* const b) {
            if(b->type() != t) return false;
            const auto eb = dynamic_cast<eEmployingBuilding*>(b);
            return eb && eb->employed() > 0;
        });
    }
}

bool eBoardCity::canTakePartInGames(const eGames game) const {
    if(mAtlantean) return false;
    const int colleges = countWorking(*this, eBuildingType::college);
    const int dramaSchools = countWorking(*this, eBuildingType::dramaSchool);
    const int gyms = countWorking(*this, eBuildingType::gymnasium);
    const int stadiums = countWorking(*this, eBuildingType::stadium);
    switch(game) {
    case eGames::isthmian:
        return colleges > 0;
    case eGames::nemean:
        return gyms + stadiums > 0;
    case eGames::pythian:
        return dramaSchools > 0;
    case eGames::olympian:
        return colleges + dramaSchools + gyms + stadiums > 0;
    }
    return false;
}

int eBoardCity::gamesScore(const eGames game, const bool roll) const {
    const int colleges = countWorking(*this, eBuildingType::college);
    const int dramaSchools = countWorking(*this, eBuildingType::dramaSchool);
    const int gyms = countWorking(*this, eBuildingType::gymnasium);
    const int stadiums = countWorking(*this, eBuildingType::stadium);
    const int podiums = countWorking(*this, eBuildingType::podium);
    const int theaters = countWorking(*this, eBuildingType::theater);

    // coverage is the share of people whose house had a visit, 0-100
    const int phil = mPhilosophyCoverage/4 + colleges + podiums;
    const int ath = mAthleticsCoverage/4 + 5*stadiums + gyms;
    const int drama = mDramaCoverage/4 + dramaSchools + theaters;
    const auto as = sanctuary(eGodType::apollo);
    const int apollo = as && as->active() ? 20 : 0;

    int score = 0;
    switch(game) {
    case eGames::isthmian:
        score = phil + apollo;
        break;
    case eGames::nemean:
        score = ath + apollo;
        break;
    case eGames::pythian:
        score = drama + apollo;
        break;
    case eGames::olympian:
        score = apollo + (phil + ath + drama)/3 + 4*stadiums;
        break;
    }
    if(!roll || score == 0) return score;
    // while Hercules' Hall stands and the city hasn't won the games he
    // asks for, the score is squared and multiplied by 0.16
    if(heroHall(eHeroType::hercules) && mWonGames == 0) {
        score = static_cast<int>(score*score*0.16);
    }
    return score;
}

int eBoardCity::sGamesScoreText(const int score) {
    if(score < 5) return 14; // terrible
    if(score < 10) return 13; // poor
    if(score < 20) return 12; // ok
    if(score < 30) return 11; // not bad
    return 10; // good
}

bool eBoardCity::personPlayerOwner() const {
    const auto ppid = mBoard.personPlayer();
    const auto pid = owningPlayerId();
    return ppid == pid;
}

ePlayerId eBoardCity::owningPlayerId() const {
    return mBoard.cityIdToPlayerId(mId);
}

eBoardPlayer* eBoardCity::owningPlayer() const {
    const auto pid = owningPlayerId();
    const auto p = mBoard.boardPlayerWithId(pid);
    return p;
}

std::vector<eAgoraBase*> eBoardCity::agoras() const {
    std::vector<eAgoraBase*> r;
    for(const auto b : mAllBuildings) {
        const auto a = dynamic_cast<eAgoraBase*>(b);
        if(!a) continue;
        r.push_back(a);
    }
    return r;
}

int eBoardCity::eliteHouses() const {
    int r = 0;
    for(const auto b : mTimedBuildings) {
        const auto bt = b->type();
        if(bt != eBuildingType::eliteHousing) continue;
        // only lived-in houses count (Odysseus needs 8 with residents,
        // Citadel of Poseidon, Trojan War ep. 4)
        const auto h = static_cast<eEliteHousing*>(b);
        if(h->people() > 0) r++;
    }
    return r;
}

eSanctuary* eBoardCity::sanctuary(const eGodType god) const {
    for(const auto s : mSanctuaries) {
        if(s->godType() == god) return s;
    }
    return nullptr;
}

eHerosHall* eBoardCity::heroHall(const eHeroType hero) const {
    for(const auto h : mHeroHalls) {
        if(h->heroType() == hero) return h;
    }
    return nullptr;
}

ePyramid* eBoardCity::pyramid(const eBuildingType type) const {
    for(const auto b : mTimedBuildings) {
        const auto t = b->type();
        if(t == type) return static_cast<ePyramid*>(b);
    }
    return nullptr;
}

std::vector<ePyramid*> eBoardCity::pyramids() const {
    std::vector<ePyramid*> result;
    for(const auto b : mTimedBuildings) {
        if(const auto p = dynamic_cast<ePyramid*>(b)) {
            result.push_back(p);
        }
    }
    return result;
}

eBanner* eBoardCity::banner(const eBannerTypeS type, const int id) const {
    for(const auto& b : mBanners) {
        const int bid = b->id();
        if(bid != id) continue;
        const auto btype = b->type();
        if(btype != type) continue;
        return b;
    }
    return nullptr;
}

eTile* eBoardCity::entryPoint() const {
    for(const auto b : mBanners) {
        const auto type = b->type();
        if(type == eBannerTypeS::entryPoint) {
            return b->tile();
        }
    }
    return nullptr;
}

eTile* eBoardCity::exitPoint() const {
    for(const auto b : mBanners) {
        const auto type = b->type();
        if(type == eBannerTypeS::exitPoint) {
            return b->tile();
        }
    }
    return nullptr;
}

eTile* eBoardCity::riverEntryPoint() const {
    for(const auto b : mBanners) {
        const auto type = b->type();
        if(type == eBannerTypeS::riverEntryPoint) {
            return b->tile();
        }
    }
    return nullptr;
}

eTile* eBoardCity::riverExitPoint() const {
    for(const auto b : mBanners) {
        const auto type = b->type();
        if(type == eBannerTypeS::riverExitPoint) {
            return b->tile();
        }
    }
    return nullptr;
}

eTile* eBoardCity::monsterTile(const int id) const {
    const auto b = banner(eBannerTypeS::monsterPoint, id);
    return b ? b->tile() : nullptr;
}

eTile* eBoardCity::landInvasionTile(const int id) const {
    const auto b = banner(eBannerTypeS::landInvasion, id);
    return b ? b->tile() : nullptr;
}

eTile* eBoardCity::seaInvasionTile(const int id) const {
    const auto b = banner(eBannerTypeS::seaInvasion, id);
    return b ? b->tile() : nullptr;
}

eTile* eBoardCity::disasterTile(const int id) const {
    const auto b = banner(eBannerTypeS::disasterPoint, id);
    return b ? b->tile() : nullptr;
}

eTile* eBoardCity::landSlideTile(const int id) const {
    const auto b = banner(eBannerTypeS::landSlidePoint, id);
    return b ? b->tile() : nullptr;
}

void eBoardCity::addMonsterEvent(const eMonsterType type, eMonsterInvasionEventBase * const e) {
    mMonsterEvents[type] = e;
}

void eBoardCity::removeMonsterEvent(eMonsterInvasionEventBase * const e) {
    for(const auto& m : mMonsterEvents) {
        if(m.second != e) continue;
        mMonsterEvents.erase(m.first);
        break;
    }
}

void eBoardCity::addInvasionHandler(eInvasionHandler* const i) {
    mInvasionHandlers.push_back(i);
}

void eBoardCity::removeInvasionHandler(eInvasionHandler* const i) {
    eVectorHelpers::remove(mInvasionHandlers, i);
}

bool eBoardCity::hasActiveInvasions() const {
    return !mInvasionHandlers.empty();
}

const std::vector<eMonster*>& eBoardCity::monsters() const {
    return mMonsters;
}

void eBoardCity::monsterSlayed(const eMonsterType m) {
    const auto it = mMonsterEvents.find(m);
    if(it == mMonsterEvents.end()) return;
    const auto e = it->second;
    e->killed(m);
    mMonsterEvents.erase(m);
}

const eBoardCity::eChars& eBoardCity::attackingGods() const {
    return mAttackingGods;
}

bool eBoardCity::wasHeroSummoned(const eHeroType hero) const {
    return eVectorHelpers::contains(mSummonedHeroes, hero);
}

void eBoardCity::heroSummoned(const eHeroType hero) {
    mSummonedHeroes.push_back(hero);
}

void eBoardCity::addRootGameEvent(const stdsptr<eGameEvent>& e) {
    mCityEvents.addEvent(e);
}

void eBoardCity::removeRootGameEvent(const stdsptr<eGameEvent>& e) {
    mCityEvents.removeEvent(e);
}

void eBoardCity::incTerrainState() {
    mTerrainState++;
    mResourceTilesUpdate = true;
}

void eBoardCity::incForestsState() {
    mForestsState++;
    mForestTilesUpdate = true;
}

const std::vector<eTile*>& eBoardCity::animalBuildingsTiles() {
    if(mAnimalBuildingsSurroundingUpdate) {
        mAnimalBuildingsSurrounding.clear();
        const int range = eNumbers::sAnimalMoveRange + 2;
        for(const auto b : mAnimalBuildings) {
            const auto center = b->centerTile();
            const int x = center->x();
            const int y = center->y();
            for(int i = x - range; i <= x + range; i++) {
                for(int j = y - range; j <= y + range; j++) {
                    const auto tile = mBoard.tile(i, j);
                    if(!tile) continue;
                    const auto cid = tile->cityId();
                    if(cid != mId) continue;
                    const bool r = eVectorHelpers::contains(
                                       mAnimalBuildingsSurrounding, tile);
                    if(r) continue;
                    mAnimalBuildingsSurrounding.push_back(tile);
                }
            }
        }
        mAnimalBuildingsSurroundingUpdate = false;
    }
    return mAnimalBuildingsSurrounding;
}

const std::vector<eTile*>& eBoardCity::huntingTiles() {
    if(mHuntingTilesUpdate) {
        mHuntingTiles.clear();
        const int range = eNumbers::sAnimalMoveRange + 2;
        for(const auto s : mSpawners) {
            const auto type = s->type();
            if(type != eBannerTypeS::boar &&
               type != eBannerTypeS::deer) continue;
            const auto tile = s->tile();
            const int x = tile->x();
            const int y = tile->y();
            for(int i = x - range; i <= x + range; i++) {
                for(int j = y - range; j <= y + range; j++) {
                    const auto tile = mBoard.tile(i, j);
                    if(!tile) continue;
                    const auto cid = tile->cityId();
                    if(cid != mId) continue;
                    const bool r = eVectorHelpers::contains(
                                       mHuntingTiles, tile);
                    if(r) continue;
                    mHuntingTiles.push_back(tile);
                }
            }
        }
        mHuntingTilesUpdate = false;
    }
    return mHuntingTiles;
}

const std::vector<eTile*>& eBoardCity::resourceTiles() {
    if(mResourceTilesUpdate) {
        updateResourceTiles();
        mResourceTilesUpdate = false;
    }
    return mResourceTiles;
}

const std::vector<eTile*>& eBoardCity::forestTiles() {
    if(mForestTilesUpdate) {
        updateForestTiles();
        mForestTilesUpdate = false;
    }
    return mForestTiles;
}

int eBoardCity::workingHippodrome() const {
    int result = 0;
    for(const auto& h : mHippodromes) {
        const bool w = h->working();
        if(!w) continue;
        const int r = h->length();
        if(r > result) result = r;
    }
    return result;
}

void eBoardCity::setInvasionHandlersIOIDs(int& id) {
    for(const auto i : mInvasionHandlers) {
        i->setIOID(id++);
    }
}

eInvasionHandler* eBoardCity::invasionHandlerWithIOID(const int id) const {
    if(id == -1) return nullptr;
    for(const auto i : mInvasionHandlers) {
        const int iio = i->ioID();
        if(iio == id) return i;
    }
    return nullptr;
}

bool eBoardCity::handleEpisodeCompleteEvents() {
    return mCityEvents.handleEpisodeCompleteEvents();
}

void eBoardCity::hippodromeStanding() {
    // zeus.exe 0x0056FC60, run at the start of months 0 and 6 (January and
    // July) while the hippodrome races: the first two columns of table
    // 0x006429E8 by track length are added to the attitude of allies,
    // vassals and colonies (first column) and of rivals (second column).
    // (The exe doesn't reset the value between cities, so a parent city or
    // distant city after one of those gets the same amount; not copied.)
    const auto m = mBoard.date().month();
    if(m != eMonth::january && m != eMonth::july) return;
    int len = 0;
    for(const auto& h : mHippodromes) {
        if(!h || !h->working()) continue;
        len = h->length();
        break;
    }
    if(len < 4) return;
    const int band = len < 12 ? 0 : len < 24 ? 1 : len < 36 ? 2 :
                     len < 60 ? 3 : len < 100 ? 4 : len < 200 ? 5 : 6;
    const int friendly[7] = {0, 0, 1, 1, 1, 2, 3};
    const int rivals[7] = {0, 0, 0, 0, 1, 1, 3};
    const auto pid = mBoard.cityIdToPlayerId(mId);
    for(const auto& c : mBoard.world().cities()) {
        if(!c || c->isCurrentCity()) continue;
        int a = 0;
        if(c->isAlly() || c->isVassal() || c->isColony()) a = friendly[band];
        else if(c->isRival()) a = rivals[band];
        if(a != 0) c->incAttitude(a, pid);
    }
}

void eBoardCity::updateHippodromes() {
    for(int i = 0; i < (int)mHippodromes.size(); i++) {
        const auto& h = mHippodromes[i];
        if(!h->closed()) {
            h->clear();
            mHippodromes.erase(mHippodromes.begin() + i);
            i--;
        }
    }

    const auto hps = buildings(eBuildingType::hippodromePiece);
    for(const auto b : hps) {
        const auto hp = static_cast<eHippodromePiece*>(b);
        {
            const auto h = hp->hippodrome();
            if(h) continue;
        }
        const auto h = std::make_shared<eHippodrome>(mId, mBoard);
        h->addPieces(hp);
        mHippodromes.push_back(h);
    }
}

void eBoardCity::clearHippodromes() {
    for(const auto& h : mHippodromes) {
        h->clear();
    }
    mHippodromes.clear();
}

void eBoardCity::clearAfterLastEpisode() {
    mCityEvents.clearAfterLastEpisode();
}

void eBoardCity::startEpisode(eEpisode* const e) {
    const auto& es = e->fEvents[mId];
    const auto date = mBoard.date();
    for(const auto& ee : es) {
        const auto eee = ee->makeCopy();
        eee->setupStartDate(date);
        mCityEvents.addEvent(eee);
        eee->loadResources();
    }

    const auto& ab = e->fAvailableBuildings[mId];
    mAvailableBuildings.startEpisode(ab);

    const auto it = e->fMaxSanctuaries.find(mId);
    if(it != e->fMaxSanctuaries.end()) {
        mMaxSanctuaries = it->second;
    }

    mNextAttackDate = date;
    mNextAttackDate.nextYears(5);
    mNextAttackPlanned = true;

    setFriendlyGods(e->fFriendlyGods[mId]);
}

void eBoardCity::loadResources() {
    mCityEvents.loadResources();
}

void eBoardCity::setFriendlyGods(const std::vector<eGodType>& gods) {
    for(const auto g : gods) {
        eBuildingType bt;
        switch(g) {
        case eGodType::aphrodite:
            bt = eBuildingType::templeAphrodite;
            break;
        case eGodType::apollo:
            bt = eBuildingType::templeApollo;
            break;
        case eGodType::ares:
            bt = eBuildingType::templeAres;
            break;
        case eGodType::artemis:
            bt = eBuildingType::templeArtemis;
            break;
        case eGodType::athena:
            bt = eBuildingType::templeAthena;
            break;
        case eGodType::atlas:
            bt = eBuildingType::templeAtlas;
            break;
        case eGodType::demeter:
            bt = eBuildingType::templeDemeter;
            break;
        case eGodType::dionysus:
            bt = eBuildingType::templeDionysus;
            break;
        case eGodType::hades:
            bt = eBuildingType::templeHades;
            break;
        case eGodType::hephaestus:
            bt = eBuildingType::templeHephaestus;
            break;
        case eGodType::hera:
            bt = eBuildingType::templeHera;
            break;
        case eGodType::hermes:
            bt = eBuildingType::templeHermes;
            break;
        case eGodType::poseidon:
            bt = eBuildingType::templePoseidon;
            break;
        case eGodType::zeus:
            bt = eBuildingType::templeZeus;
            break;
        default:
            bt = eBuildingType::none;
            break;
        }

        allow(bt);
    }

    const auto e = e::make_shared<eGodVisitEvent>(
        mId, eGameEventBranch::root, mBoard);
    e->setIsEpisodeEvent(true);
    eDate date = mBoard.date();
    const int period = eNumbers::sFriendlyGodVisitPeriod;
    date += period;
    date += eRand::rand() % 60;
    e->initializeDate(date, period, 10000);
    e->setTypes(gods);
    addRootGameEvent(e);
    e->loadResources();
}
