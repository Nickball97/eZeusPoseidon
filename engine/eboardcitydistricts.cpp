// Part of eBoardCity: the districts of a computer-run city and the editor's city plan.
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


void eBoardCity::saveEditorCityPlan() {
    mCityPlan = eAICityPlan(mId);
    for(const auto b : mAllBuildings) {
        const int did = b->districtId();
        while(did >= mCityPlan.districtCount()) {
            const int id = mCityPlan.districtCount();
            eAIDistrict d;
            d.fReadyConditions = mEditorDistrictConditions[id];
            mCityPlan.addDistrict(d);
        }
        auto& d = mCityPlan.district(did);
        eAIBuilding ab;
        ab.fType = b->type();
        ab.fRect = b->tileRect();
        if(const auto sb = dynamic_cast<eStorageBuilding*>(b)) {
            ab.fGet = sb->get();
            ab.fEmpty = sb->empties();
            ab.fAccept = sb->accepts();
            ab.fSpace = sb->maxCount();
            if(const auto p = dynamic_cast<eTradePost*>(b)) {
                ab.fO = p->orientation();
                ab.fTradePostType = p->tpType();
                const auto city = p->city();
                ab.fTradingPartner = city.cityId();
                if(ab.fTradePostType == eTradePostType::pier) {
                    const auto pier = p->unpackBuilding();
                    ab.fOtherRect = pier->tileRect();
                }
            }
        } else if(ab.fType == eBuildingType::pier) {
            continue;
        } else if(const auto hr = dynamic_cast<eHorseRanch*>(b)) {
            const auto e = hr->enclosure();
            ab.fOtherRect = e->tileRect();
        } else if(ab.fType == eBuildingType::horseRanchEnclosure) {
            continue;
        } else if(ab.fType == eBuildingType::agoraSpace) {
            continue;
        } else if(ab.fType == eBuildingType::foodVendor) {
            continue;
        } else if(ab.fType == eBuildingType::fleeceVendor) {
            continue;
        } else if(ab.fType == eBuildingType::oilVendor) {
            continue;
        } else if(ab.fType == eBuildingType::wineVendor) {
            continue;
        } else if(ab.fType == eBuildingType::armsVendor) {
            continue;
        } else if(ab.fType == eBuildingType::chariotVendor) {
            continue;
        } else if(ab.fType == eBuildingType::horseTrainer) {
            continue;
        } else if(ab.fType == eBuildingType::palaceTile) {
            continue;
        } else if(ab.fType == eBuildingType::templeTile) {
            continue;
        } else if(ab.fType == eBuildingType::templeAltar) {
            continue;
        } else if(ab.fType == eBuildingType::templeMonument) {
            continue;
        } else if(ab.fType == eBuildingType::templeGrandStatue) {
            continue;
        } else if(ab.fType == eBuildingType::templeObelisk) {
            continue;
        } else if(ab.fType == eBuildingType::templeStatue) {
            continue;
        } else if(ab.fType == eBuildingType::temple) {
            continue;
        } else if(ab.fType == eBuildingType::placeholder) {
            continue;
        } else if(const auto pb = dynamic_cast<ePatrolBuildingBase*>(b)) {
            ab.fGuides = pb->patrolGuides();
            ab.fGuidesBothDirections = pb->bothDirections();
            if(const auto f = dynamic_cast<eAgoraBase*>(b)) {
                ab.fO = f->diagonalOrientation();
            }
        } else if(const auto s = dynamic_cast<eSanctuary*>(b)) {
            const bool r = s->rotated();
            ab.fO = r ? eDiagonalOrientation::topLeft :
                        eDiagonalOrientation::topRight;
        } else if(const auto uq = dynamic_cast<eUrchinQuay*>(b)) {
            ab.fO = uq->orientation();
        } else if(const auto f = dynamic_cast<eFishery*>(b)) {
            ab.fO = f->orientation();
        }
        d.addBuilding(ab);
    }
}

void eBoardCity::editorClearBuildings() {
    for(const auto b : mAllBuildings) {
        const auto type = b->type();
        switch(type) {
        case eBuildingType::agoraSpace:
        case eBuildingType::foodVendor:
        case eBuildingType::fleeceVendor:
        case eBuildingType::oilVendor:
        case eBuildingType::wineVendor:
        case eBuildingType::armsVendor:
        case eBuildingType::horseTrainer:
        case eBuildingType::chariotVendor:
            continue;
        default:
            break;
        }

        b->erase();
    }
}

void eBoardCity::editorDisplayBuildings() {
    mCityPlan.editorDisplayBuildings(mBoard);
}

void eBoardCity::buildScheduled() {
    mCityPlan.buildScheduled(mBoard);
}

bool eBoardCity::previousDistrictFulfilled() {
    const int id = mCityPlan.lastBuiltDistrictId();
    if(id == -1) return true;
    const auto& d = mCityPlan.district(id);
    const auto& cs = d.fReadyConditions;
    if(cs.empty()) return true;
    for(const auto& c : cs) {
        using eType = eDistrictReadyCondition::eType;
        if(c.fType == eType::districtResourceCount ||
           c.fType == eType::totalResourceCount) {
            int total = 0;
            if(c.fResource == eResourceType::drachmas) {
                const auto pid = mBoard.cityIdToPlayerId(mId);
                total = mBoard.drachmas(pid);
            } else {
                const bool totalType = c.fType == eType::totalResourceCount;
                for(const auto b : mStorBuildings) {
                    if(!totalType) {
                        const int did = b->districtId();
                        if(did != id) continue;
                    }
                    const int cc = b->count(c.fResource);
                    total += cc;
                    if(total >= c.fValue) break;
                }
            }
            if(total < c.fValue) return false;
        } else if(c.fType == eType::districtPopulation) {
            int total = 0;
            for(const auto& b : mTimedBuildings) {
                if(const auto hb = dynamic_cast<eHouseBase*>(b)) {
                    const int did = b->districtId();
                    if(did != id) continue;
                    const int cc = hb->people();
                    total += cc;
                    if(total >= c.fValue) break;
                }
            }
            if(total < c.fValue) return false;
        } else if(c.fType == eType::totalPopulation) {
            const int p = population();
            if(p < c.fValue) return false;
        } else if(c.fType == eType::sanctuaryReady) {
            const auto s = sanctuary(c.fSanctuary);
            if(!s) return false;
            if(!s->finished()) return false;
        }
    }
    return true;
}

int eBoardCity::nextDistrictCost(int* const marble) {
    const int id = mCityPlan.nextDistrictId();
    if(id == -1) return 0;
    auto& c = mNextDistrictCost;
    if(c.fDrachmas == -1) {
        c.fDrachmas = mCityPlan.districtCost(mBoard, id, &c.fMarble);
    }
    if(marble) *marble = c.fMarble;
    return c.fDrachmas;
}

bool eBoardCity::buildNextDistrict(const int drachmas) {
    const bool pf = previousDistrictFulfilled();
    if(!pf) return false;
    const int id = mCityPlan.nextDistrictId();
    if(id == -1) return false;
    int marbleCost = 0;
    const int cost = nextDistrictCost(&marbleCost);
    if(cost > drachmas) return false;
    const int marble = resourceCount(eResourceType::marble);
    if(marbleCost > marble) return false;
    mCityPlan.buildNextDistrict(mBoard);
    mNextDistrictCost.fDrachmas = -1;
    return true;
}

std::vector<eBoardCity::eCondition> eBoardCity::getDistrictReadyConditions() {
    const auto& v = mEditorDistrictConditions[mCurrentDistrictId];
    return v;
    if(mCurrentDistrictId >= mCityPlan.districtCount()) return {};
    const auto& d = mCityPlan.district(mCurrentDistrictId);
    return d.fReadyConditions;
}

void eBoardCity::addDistrictReadyCondition(const eCondition& c) {
    auto& v = mEditorDistrictConditions[mCurrentDistrictId];
    v.push_back(c);
    if(mCurrentDistrictId >= mCityPlan.districtCount()) return;
    auto& d = mCityPlan.district(mCurrentDistrictId);
    d.fReadyConditions.push_back(c);
}

void eBoardCity::removeDistrictReadyCondition(const int id) {
    auto& v = mEditorDistrictConditions[mCurrentDistrictId];
    v.erase(v.begin() + id);
    if(mCurrentDistrictId >= mCityPlan.districtCount()) return;
    auto& d = mCityPlan.district(mCurrentDistrictId);
    d.fReadyConditions.erase(d.fReadyConditions.begin() + id);
}

void eBoardCity::setDistrictReadyCondition(const int id, const eCondition& c) {
    auto& v = mEditorDistrictConditions[mCurrentDistrictId];
    v[id] = c;
    if(mCurrentDistrictId >= mCityPlan.districtCount()) return;
    auto& d = mCityPlan.district(mCurrentDistrictId);
    d.fReadyConditions[id] = c;
}
