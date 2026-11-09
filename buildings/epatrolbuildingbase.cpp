#include "epatrolbuildingbase.h"

#include "engine/egameboard.h"
#include "engine/eguidedmovepathtask.h"
#include "characters/etaxcollector.h"

#include <algorithm>

// How many tiles a roaming walker walks before turning back, from
// HeavenGames' "Walker Distances" table (zeus.heavengames.com/units/).
// Other patrol buildings keep the general patroler_max_distance_i setting.
int ePatrolBuildingBase::sMaxDistance(const eBuildingType type) {
    switch(type) {
    case eBuildingType::fountain: // water carrier
    case eBuildingType::hospital: // healer
        return 27;
    case eBuildingType::taxOffice: // tax collector
    case eBuildingType::gymnasium:
    case eBuildingType::podium:
    case eBuildingType::theater:
    case eBuildingType::stadium:
    case eBuildingType::bibliotheke:
    case eBuildingType::laboratory:
    case eBuildingType::observatory:
    case eBuildingType::museum:
        return 35;
    case eBuildingType::commonAgora: // peddler
    case eBuildingType::grandAgora:
    case eBuildingType::maintenanceOffice: // superintendent
    case eBuildingType::watchPost: // watchman
        return 44;
    default:
        return eNumbers::sPatrolerMaxDistance;
    }
}

ePatrolBuildingBase::ePatrolBuildingBase(
        eGameBoard& board,
        const eCharGenerator& charGen,
        const eActGenerator& actGen,
        const eBuildingType type,
        const int sw, const int sh,
        const int maxEmployees,
        const eCityId cid) :
    eEmployingBuilding(board, type, sw, sh, maxEmployees, cid),
    mCharGenerator(charGen),
    mActGenerator(actGen),
    mMaxDistance(sMaxDistance(type)) {

}

stdsptr<eCharacterAction>
ePatrolBuildingBase::sDefaultActGenerator(
           eCharacter* const c,
           ePatrolBuildingBase* const b,
           const std::vector<eOrientation>& path,
           const stdsptr<eDirectionTimes>& dirTimes) {
    return e::make_shared<ePatrolAction>(c, b, path, dirTimes);
}

ePatrolBuildingBase::ePatrolBuildingBase(eGameBoard& board,
        const eCharGenerator& charGen,
        const eBuildingType type,
        const int sw, const int sh,
        const int maxEmployees,
        const eCityId cid) :
    ePatrolBuildingBase(board, charGen, sDefaultActGenerator,
                        type, sw, sh, maxEmployees, cid) {}

ePatrolBuildingBase::~ePatrolBuildingBase() {
    if(mChar) mChar->kill();
}

// Cycles (16 a month) a building waits, once its walker is back, before
// sending the next, by staffing: full, 75%+, 50%+, 25%+, any (original exe
// step 31, 0x0048CFB0). 0 where the original's rule isn't known.
static int sSpawnWaitCycles(const eBuildingType type, const int step) {
    static const int sService[5] = {1, 2, 4, 8, 16};
    static const int sFountain[5] = {2, 4, 8, 16, 30};
    static const int sVenue[5] = {4, 8, 16, 30, 45};
    static const int sPodium[5] = {9, 16, 31, 51, 101};
    static const int sGym[5] = {5, 6, 11, 18, 31};
    switch(type) {
    case eBuildingType::taxOffice:
    case eBuildingType::maintenanceOffice:
    case eBuildingType::watchPost:
        return sService[step];
    case eBuildingType::fountain:
        return sFountain[step];
    case eBuildingType::hospital:
    case eBuildingType::theater:
    case eBuildingType::stadium:
        return sVenue[step];
    case eBuildingType::podium:
        return sPodium[step];
    case eBuildingType::gymnasium:
        return sGym[step];
    default:
        return 0;
    }
}

void ePatrolBuildingBase::timeChanged(const int by) {
    if(mSpawnPatrolers && enabled()) {
        if(!mChar) {
            double waitTime = mWaitTime;
            double rate = effectiveness();
            const double filled = vacanciesFilledFraction();
            if(filled > 0.) {
                const int step = filled >= 1. ? 0 :
                                 filled >= 0.75 ? 1 :
                                 filled >= 0.5 ? 2 :
                                 filled >= 0.25 ? 3 : 4;
                const int cycles = sSpawnWaitCycles(type(), step);
                if(cycles > 0) {
                    const int cycle = (365*eNumbers::sDayLength)/(12*16);
                    waitTime = cycles*cycle;
                    // staffing is in the wait itself; a blessing still
                    // speeds it up
                    rate /= filled;
                }
            }
            mSpawnTime += by*rate;
            if(mSpawnTime > waitTime) {
                mSpawnTime = 0;
                // blessed: walkers come twice as often (sBlessMultiplier);
                // two such walkers make one extra, which uses it up
                // (a tax office instead spends it on one collector, in spawn)
                if(blessed() && type() != eBuildingType::taxOffice) {
                    spendBlessing(0.5);
                }
                const bool r = updatePathIfNeeded();
                if(r) mSpawnOnPathSet = true;
                else spawn();
            }
        }
    }
    eEmployingBuilding::timeChanged(by);
}

using ePatrolGuides = std::vector<ePatrolGuide>;
ePatrolGuides ePatrolBuildingBase::reversePatrolGuides() const {
    auto guides = mPatrolGuides;
    std::reverse(guides.begin(), guides.end());
    return guides;
}

void ePatrolBuildingBase::setPatrolGuides(const ePatrolGuides &g) {
    mPatrolGuides = g;
}

void ePatrolBuildingBase::setBothDirections(const bool both) {
    mBothDirections = both;
}

bool ePatrolBuildingBase::spawn() {
    if(!mCharGenerator || !mActGenerator) return false;
    const auto chr = mCharGenerator();
    mChar = chr.get();
    if(!mChar) return false;
    if(type() == eBuildingType::taxOffice && blessed()) {
        if(const auto tc = dynamic_cast<eTaxCollector*>(chr.get())) {
            tc->setBlessed(true);
            spendBlessing(1.);
        }
    }
    chr->setBothCityIds(cityId());
    if(mPatrolGuides.empty()) {
        eTile* t = nullptr;
        const auto bt = type();
        if(bt == eBuildingType::commonAgora ||
           bt == eBuildingType::grandAgora) {
            t = centerTile();
        } else {
            const auto ts = surroundingRoad(false, true);
            if(!ts.empty()) {
                const int tss = ts.size();
                if(++mSpawnRoadId >= tss) mSpawnRoadId = 0;
                t = ts[mSpawnRoadId];
            }
        }
        if(!t) return false;
        chr->changeTile(t);
    } else {
        chr->changeTile(centerTile());
    }
    bool reverse = false;
    if(mBothDirections) {
        mLastDirection = !mLastDirection;
        if(mLastDirection) reverse = true;
    }
    const auto a = mActGenerator(chr.get(), this,
                                 reverse ? mReversePath : mPath,
                                 mDirTimes);
    chr->setAction(a);
    return true;
}

void ePatrolBuildingBase::setSpawnPatrolers(const bool s) {
    mSpawnPatrolers = s;
}

bool ePatrolBuildingBase::spawnsPatrolers() const {
    return mCharGenerator && mActGenerator;
}

void ePatrolBuildingBase::read(eReadStream& src) {
    eEmployingBuilding::read(src);

    src >> mBothDirections;
    src >> mLastDirection;

    src >> mSpawnPatrolers;
    src >> mSpawnTime;
    src >> mSpawnRoadId;

    mDirTimes = src.readDirectionTimes(getBoard());

    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mChar = c;
    });

    int n;
    src >> n;
    for(int i = 0; i < n; i++) {
        ePatrolGuide pg;
        src >> pg.fX;
        src >> pg.fY;
        mPatrolGuides.push_back(pg);
    }
}

void ePatrolBuildingBase::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);

    dst << mBothDirections;
    dst << mLastDirection;

    dst << mSpawnPatrolers;
    dst << mSpawnTime;
    dst << mSpawnRoadId;

    dst.writeDirectionTimes(mDirTimes.get());

    dst.writeCharacter(mChar);

    dst << mPatrolGuides.size();
    for(const auto& pg : mPatrolGuides) {
        dst << pg.fX;
        dst << pg.fY;
    }
}

bool ePatrolBuildingBase::updatePathIfNeeded() {
    if(mPatrolGuides.empty()) {
        mPath.clear();
        mReversePath.clear();
        return false;
    }
    auto& board = getBoard();
    const auto cid = cityId();
    const auto c = board.boardCityWithId(cid);
    const int s = c->roadState();
    if(s == mPathState) return false;
    mPathState = s;
    return updatePath();
}

bool ePatrolBuildingBase::updatePath(const eAction& finish) {
    if(mPatrolGuides.empty()) {
        mPath.clear();
        mReversePath.clear();
        return false;
    }
    auto& board = getBoard();
    auto& tp = board.threadPool();
    const auto task = new eGuidedMovePathTask(this, finish);
    tp.queueTask(task);
    return true;
}

void ePatrolBuildingBase::setPath(const std::vector<eOrientation> &path,
                                  const std::vector<eOrientation> &reversePath) {
    mPath = path;
    mReversePath = reversePath;
    if(mSpawnOnPathSet) {
        mSpawnOnPathSet = false;
        spawn();
    }
}
