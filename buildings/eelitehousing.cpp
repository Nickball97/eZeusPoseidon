#include "eelitehousing.h"
#include "ehousemodel.h"

#include "engine/egameboard.h"

#include "textures/egametextures.h"
#include "elanguage.h"
#include "enumbers.h"
#include "buildings/epalace.h"
#include "characters/eelitecitizen.h"
#include "characters/actions/esickdisgruntledaction.h"
#include "fileIO/efileformat.h"
#include "erand.h"

#include <algorithm>

eEliteHousing::eEliteHousing(eGameBoard& board,
                             const eCityId cid) :
    eHouseBase(board, eBuildingType::eliteHousing,
               4, 4, {6, 6, 10, 16, 20}, cid) {}

eTextureSpace eEliteHousing::getTextureSpace(
        const int tx, const int ty,
        const eTileSize size) const {
    const SDL_Point p{tx, ty};
    const auto& r = tileRect();
    if(!SDL_PointInRect(&p, &r)) return {nullptr};
    auto& board = getBoard();
    const auto dir = board.direction();
    const auto& coll = getTextureCollection(size);
    const int rx = r.x;
    const int ry = r.y;
    const int dx = tx - rx;
    const int dy = ty - ry;
    if(dx < 2 && dy < 2) { // top
        if(dir == eWorldDirection::N) {
            const auto& tex = coll.getTexture(3);
            return {tex, true, {r.x, r.y, 2, 2}};
        } else if(dir == eWorldDirection::E) {
            const int id = seed() % 2;
            const auto& tex = coll.getTexture(id);
            return {tex, true, {r.x, r.y, 2, 2}};
        } else if(dir == eWorldDirection::S) {
            const auto& tex = coll.getTexture(2);
            return {tex, true, {r.x, r.y, 2, 2}};
        } else { // if(dir == eWorldDirection::W) {
            const auto& tex = coll.getTexture(4);
            return {tex, true, {r.x, r.y, 2, 2}};
        }
    } else if(dx < 2 && dy >= 2) { // left
        if(dir == eWorldDirection::N) {
            const int id = seed() % 2;
            const auto& tex = coll.getTexture(id);
            return {tex, false, {r.x, r.y + 2, 2, 2}};
        } else if(dir == eWorldDirection::E) {
            const auto& tex = coll.getTexture(2);
            return {tex, false, {r.x, r.y + 2, 2, 2}};
        } else if(dir == eWorldDirection::S) {
            const auto& tex = coll.getTexture(4);
            return {tex, false, {r.x, r.y + 2, 2, 2}};
        } else { // if(dir == eWorldDirection::W) {
            const auto& tex = coll.getTexture(3);
            return {tex, false, {r.x, r.y + 2, 2, 2}};
        }
    } else if(dx >= 2 && dy < 2) { // right
        if(dir == eWorldDirection::N) {
            const auto& tex = coll.getTexture(4);
            return {tex, false, {r.x + 2, r.y, 2, 2}};
        } else if(dir == eWorldDirection::E) {
            const auto& tex = coll.getTexture(3);
            return {tex, false, {r.x + 2, r.y, 2, 2}};
        } else if(dir == eWorldDirection::S) {
            const int id = seed() % 2;
            const auto& tex = coll.getTexture(id);
            return {tex, false, {r.x + 2, r.y, 2, 2}};
        } else { // if(dir == eWorldDirection::W) {
            const auto& tex = coll.getTexture(2);
            return {tex, false, {r.x + 2, r.y, 2, 2}};
        }
    } else { // bottom
        if(dir == eWorldDirection::N) {
            const auto& tex = coll.getTexture(2);
            return {tex, false, {r.x + 2, r.y + 2, 2, 2}};
        } else if(dir == eWorldDirection::E) {
            const auto& tex = coll.getTexture(4);
            return {tex, false, {r.x + 2, r.y + 2, 2, 2}};
        } else if(dir == eWorldDirection::S) {
            const auto& tex = coll.getTexture(3);
            return {tex, false, {r.x + 2, r.y + 2, 2, 2}};
        } else { // if(dir == eWorldDirection::W) {
            const int id = seed() % 2;
            const auto& tex = coll.getTexture(id);
            return {tex, false, {r.x + 2, r.y + 2, 2, 2}};
        }
    }
    return {};
}

std::vector<eOverlay> eEliteHousing::getOverlays(const eTileSize size) const {
//    auto& board = getBoard();
//    if(board.atlantean()) return {};
//    return getHorseOverlays(size);
    (void)size;
    return {};
}

std::shared_ptr<eTexture>
eEliteHousing::getLeftTexture(const eTileSize size) const {
    const auto& coll = getTextureCollection(size);
    const int id = seed() % 2;
    return coll.getTexture(id);
}

std::shared_ptr<eTexture>
eEliteHousing::getBottomTexture(const eTileSize size) const {
    const auto& coll = getTextureCollection(size);
    return coll.getTexture(2);
}

std::shared_ptr<eTexture>
eEliteHousing::getTopTexture(const eTileSize size) const {
    const auto& coll = getTextureCollection(size);
    return coll.getTexture(3);
}

std::shared_ptr<eTexture>
eEliteHousing::getRightTexture(const eTileSize size) const {
    const auto& coll = getTextureCollection(size);
    return coll.getTexture(4);
}

std::vector<eOverlay>
eEliteHousing::getHorseOverlays(const eTileSize size) const {
    if(mLevel < 3 || mHorses < 1 || mPeople <= 0) {
        return {};
    }
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];

    eGameTextures::loadEliteHouse();
    const auto& coll = texs.fEliteHouseHorses;
    eOverlay h;
    h.fX = -2.0;
    h.fY = -2.5;
    h.fTex = coll.getTexture(mHorses - 1);
    return {h};
}

int eEliteHousing::provide(const eProvide p, const int n) {
    if(mPeople <= 0) return 0;
    int max = 8;
    int* value = nullptr;
    switch(p) {
    case eProvide::food:
        value = &mFood;
        max = eNumbers::sHouseFoodStorePerPerson*
              (eNumbers::sEliteHouseFoodStoreExtra + mPeople + mHorses);
        break;
    case eProvide::fleece:
        value = &mFleece;
        max = eNumbers::sEliteHouseFleeceStoreMax;
        break;
    case eProvide::oil:
        value = &mOil;
        max = eNumbers::sEliteHouseOilStoreMax;
        break;
    case eProvide::wine:
        value = &mWine;
        max = eNumbers::sEliteHouseWineStoreMax;
        break;
    case eProvide::arms: {
        // armor storage by level (Model "ALL HOUSES" rows)
        static const int sMax[] = {0, 2, 2, 4, 4};
        max = sMax[std::clamp(mLevel, 0, 4)];
        value = &mArms;
    } break;
    case eProvide::horses: {
        // horse storage by level (Model "ALL HOUSES" rows)
        static const int sMax[] = {0, 1, 2, 4, 4};
        max = sMax[std::clamp(mLevel, 0, 4)];
        value = &mHorses;
    } break;

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

    case eProvide::taxes:
        return collectTaxes(eNumbers::sEliteHousingTaxMultiplier);
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

eEliteHousing::~eEliteHousing() {
    if(mNoble) mNoble->kill();
}

void eEliteHousing::spawnNoble() {
    auto ts = surroundingRoad(false, true);
    if(ts.empty()) return;
    const auto tile = ts[eRand::rand() % ts.size()];
    const auto c = e::make_shared<eEliteCitizen>(getBoard());
    c->setBothCityIds(cityId());
    c->changeTile(tile);
    const auto a = e::make_shared<eSickDisgruntledAction>(c.get(), this);
    c->setAction(a);
    mNoble = c.get();
}

void eEliteHousing::timeChanged(const int by) {
    // how often and how far nobles walk isn't in any source: eZeus's
    // usual roaming-walker wait is used
    if(!mNoble && mPeople > 0) {
        mNobleWait += by;
        if(mNobleWait > eNumbers::sPatrolerWaitTime) {
            mNobleWait = 0;
            spawnNoble();
        }
    }
    // checked every cycle (16 a month), as houses_evolve does
    mUpdateLevel += by;
    const int lupdate = (365*eNumbers::sDayLength)/(12*16);
    if(mUpdateLevel > lupdate) {
        mUpdateLevel -= lupdate;
        updateLevel(true);
    }
    eHouseBase::timeChanged(by);
}

void eEliteHousing::nextMonth() {
    mPaidTaxesLastMonth = mPaidTaxes;
    mPaidTaxes = 0;
    // a devolved home needs and uses nothing (Model columns i-l are 0)
    if(mLevel <= 0 || mPeople <= 0) return;
    // the residents eat a quarter of their number, rounded down, and the
    // horses 1 unit between them; with no food left for them a horse is
    // lost (original exe 0x0048B8D0)
    const int cfood = mPeople*eNumbers::sHouseFoodUsePercent/100;
    mFood = std::max(0, mFood - cfood);
    if(mHorses > 0) {
        if(mFood > 0) {
            mFood = std::max(0, mFood - eNumbers::sEliteHouseHorsesFoodUse);
        } else {
            removeHorse();
        }
    }
    // fleece, oil and wine: 1 each twice a month (original 0x00484C50)
    const int cfleece = eNumbers::sEliteHouseFleeceUse;
    const int coil = eNumbers::sEliteHouseOilUse;
    const int cwine = mLevel > 2 ? eNumbers::sEliteHouseWineUse : 0;
    mFleece = std::max(0, mFleece - cfleece);
    mOil = std::max(0, mOil - coil);
    mWine = std::max(0, mWine - cwine);
}

bool eEliteHousing::lowFood() const {
    if(!mFood) return true;
    const int cfood = mPeople*eNumbers::sHouseFoodUsePercent/100 +
                      (mHorses > 0 ? eNumbers::sEliteHouseHorsesFoodUse : 0);
    return mFood < cfood;
}

void eEliteHousing::removeArmor() {
    mArms = std::max(0, mArms - 1);
    updateLevel();
}

void eEliteHousing::removeHorse() {
    mHorses = std::max(0, mHorses - 1);
    updateLevel();
}

// Original Zeus elite house desirability (Model data, ALL HOUSES rows a
// and b): reaching level L needs eliteEvolve(L); a house already at level
// L or above keeps it while appeal stays at or above eliteDevolve(L).
// Levels: 0 abandoned, 1 residence, 2 mansion, 3 manor, 4 estate.
// Values by difficulty in eHouseModel.
bool eEliteHousing::appealFor(const int l) const {
    const double appeal = eHouseBase::appeal();
    const auto d = getBoard().difficulty(playerId());
    const int need = level() >= l ? eHouseModel::eliteDevolve(d, l) :
                                    eHouseModel::eliteEvolve(d, l);
    return appeal >= need - aphroditeAppealBonus();
}

// culture/science points for level L. Nicolás's call (9 Nov 2026): an
// estate needs only its points, no stadium or competitor visit, although
// the original exe (0x004843D0, Model column e) also asks for a
// competitor (Atlantean: curator) visit
bool eEliteHousing::cultureFor(const int l) const {
    const auto d = getBoard().difficulty(playerId());
    return culturePoints() >= eHouseModel::eliteCulture(d, l);
}

eHouseMissing eEliteHousing::missing() const {
    if(mFood <= 0) return eHouseMissing::food;
    if(mFleece <= 0) return eHouseMissing::fleece;
    if(mOil <= 0) return eHouseMissing::oil;
    if(!cultureFor(1)) return eHouseMissing::venues;
    if(!appealFor(1)) return eHouseMissing::appeal;
    if(mArms <= 0) return eHouseMissing::arms;
    if(!cultureFor(2)) return eHouseMissing::venues;
    if(!appealFor(2)) return eHouseMissing::appeal;
    if(mWine <= 0) return eHouseMissing::wine;
    if(!cultureFor(3)) return eHouseMissing::venues;
    if(!appealFor(3)) return eHouseMissing::appeal;
    if(mHorses <= 0) return eHouseMissing::horse;
    if(!cultureFor(4)) return eHouseMissing::venues;
    if(!appealFor(4)) return eHouseMissing::appeal;
    return eHouseMissing::nothing;
}

void eEliteHousing::read(eReadStream& src) {
    eHouseBase::read(src);
    src >> mUpdateLevel;
    src >> mWine;
    src >> mArms;
    src >> mHorses;
    if(src.formatVersion() >= eFileFormat::eliteNoble) {
        src.readCharacter(&getBoard(), [this](eCharacter* const c) {
            mNoble = c;
        });
        src >> mNobleWait;
    }
    if(src.formatVersion() >= eFileFormat::eliteFailedChecks) {
        src >> mFailedChecks;
    }
}

void eEliteHousing::write(eWriteStream& dst) const {
    eHouseBase::write(dst);
    dst << mUpdateLevel;
    dst << mWine;
    dst << mArms;
    dst << mHorses;
    dst.writeCharacter(mNoble);
    dst << mNobleWait;
    dst << mFailedChecks;
}

std::string eEliteHousing::sName(const int level) {
    return eLanguage::zeusText(28, 10 + level);
}

const eTextureCollection& eEliteHousing::getTextureCollection(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings()[sizeId];
    if(atlantean()) {
        eGameTextures::loadPoseidonEliteHouse();
        if(mPeople <= 0) return blds.fPoseidonEliteHouse[0];
        return blds.fPoseidonEliteHouse[mLevel + 1];
    } else {
        eGameTextures::loadEliteHouse();
        if(mPeople <= 0) return blds.fEliteHouse[0];
        return blds.fEliteHouse[mLevel + 1];
    }
}

// Original elite houses (exe 0x004FE9C0 ... 0x004FEC30): one level at a
// time, and a level is lost only after 10 failed checks in a row.
void eEliteHousing::stepToward(const int target, const bool timed) {
    if(target > mLevel) {
        mFailedChecks = 0;
        setLevel(mLevel + 1);
    } else if(target < mLevel) {
        if(!timed) return;
        if(++mFailedChecks >= 10) {
            mFailedChecks = 0;
            setLevel(mLevel - 1);
        }
    } else if(timed) {
        mFailedChecks = 0;
    }
}

void eEliteHousing::updateLevel(const bool timed) {
    if(mFood > 0 && mFleece > 0 && mOil > 0 && cultureFor(1) && appealFor(1)) {
        if(mArms > 0 && cultureFor(2) && appealFor(2)) {
            if(mWine > 0 && cultureFor(3) && appealFor(3)) {
                if(mHorses > 0 && cultureFor(4) && appealFor(4)) {
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

// Original thief (exe 0x005C00C0): from each good he takes a quarter of
// what the house holds, or half the house level's storage number from the
// Model file if that is more (food: column q, horses: g, armor: n; the
// other goods' numbers are 1 and so add nothing).
void eEliteHousing::robbed() {
    static const int sFood[5] = {0, 75, 100, 150, 200};
    static const int sHorses[5] = {0, 1, 2, 4, 4};
    static const int sArms[5] = {0, 2, 2, 4, 4};
    const int l = std::clamp(mLevel, 0, 4);
    const auto steal = [](int& stock, const int floor) {
        const int t = std::max(stock/4, floor);
        stock = std::max(0, stock - t);
    };
    steal(mFood, sFood[l]/2);
    steal(mFleece, 0);
    steal(mOil, 0);
    steal(mWine, 0);
    steal(mArms, sArms[l]/2);
    steal(mHorses, sHorses[l]/2);
    updateLevel();
}

void eEliteHousing::soldiers(int& hoplites, int& horsemen) const {
    hoplites = 0;
    horsemen = 0;
    const int l = level();
    if(l < 2) return;
    const int a = mArms;
    if(l == 2) {
        hoplites = std::min(2, a);
    } else if(l == 3) {
        hoplites = std::min(4, a);
    } else if(l == 4) {
        horsemen = std::min(std::min(a, 4), mHorses);
        hoplites = std::max(0, std::min(4 - horsemen, a - horsemen));
    }
}
