#include "ehorseranch.h"

#include "textures/egametextures.h"

#include "ehorseranchenclosure.h"
#include "engine/egameboard.h"
#include "enumbers.h"

#include <algorithm>

eHorseRanch::eHorseRanch(eGameBoard& board,
                         const eCityId cid) :
    eEmployingBuilding(board, eBuildingType::horseRanch, 3, 3, 15, cid) {
    eGameTextures::loadHorseRanch();
}

eHorseRanch::~eHorseRanch() {
    if(mTakeCart) mTakeCart->kill();
}

void eHorseRanch::erase() {
    mEnclosure->eBuilding::erase();
    eBuilding::erase();
}

std::shared_ptr<eTexture> eHorseRanch::getTexture(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    return blds[sizeId].fHorseRanch;
}

std::vector<eOverlay> eHorseRanch::getOverlays(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    const auto& texs = blds[sizeId];
    const auto& coll = texs.fHorseRanchOverlay;
    const int texId = textureTime() % coll.size();

    std::vector<eOverlay> os;
    eOverlay& o = os.emplace_back();
    o.fTex = coll.getTexture(texId);
    o.fX = -2.1;
    o.fY = -2.85;

    if(mWheat > 0) {
        const int nw = std::clamp((mWheat - 50)/100, 0, 7);
        eOverlay& wo = os.emplace_back();
        wo.fTex = texs.fWaitingWheat.getTexture(nw);
        wo.fX = -0.4;
        wo.fY = -2.7;
    }

    return os;
}

void eHorseRanch::timeChanged(const int by) {
    if(enabled()) {
        if(!mTakeCart) {
            mTakeCart = spawnCart(eCartActionTypeSupport::take);
        }
        if(blessed()) {
            // Poseidon's blessing: "the horse ranch will breed 4 extra
            // horses in less than a month" (Citadel of Poseidon)
            spendBlessing();
            int n = 0;
            for(int i = 0; i < 4; i++) {
                if(mEnclosure->spawnHorse()) n++;
            }
            if(n > 0 && isPersonPlayer()) {
                getBoard().incProduced(eResourceType::horse, n);
            }
        }
        if(mWheat > 0) {
            mWheatTime += by;
            if(mWheatTime > eNumbers::sHorseRanchWheatUsePeriod) {
                mWheatTime = 0;
                // Each horse in the ranch eats 10 units of wheat (Citadel
                // of Poseidon) a month (Nicolás); 100 units = 1 load.
                mWheat = std::max(0, mWheat - 10*horseCount());
            }
            // Horses are born in even months (February, April ...) and a
            // fully staffed ranch breeds one in each (Citadel of Poseidon);
            // fewer workers stretch the gap to later even months.
            mHorseTime += std::round(by*effectiveness());
            const auto month = getBoard().date().month();
            const bool newMonth = month != mLastMonth;
            mLastMonth = month;
            const int im = static_cast<int>(month);
            const bool evenMonth = im % 2 == 1; // february = 1
            const int need = std::round(0.9*eNumbers::sHorseRanchHorseSpawnPeriod);
            if(newMonth && evenMonth && mHorseTime >= need) {
                mHorseTime = 0;
                // Breeding one horse costs 50 units of wheat
                // (Citadel of Poseidon).
                const bool bred = mWheat >= 50 && mEnclosure->spawnHorse();
                if(bred) mWheat -= 50;

                const bool isPp = isPersonPlayer();
                if(bred && isPp) {
                    auto& board = getBoard();
                    board.incProduced(eResourceType::horse, 1);
                }
            }
        }
    }
    eEmployingBuilding::timeChanged(by);
}

int eHorseRanch::count(const eResourceType type) const {
    if(type == eResourceType::wheat) return mWheat/100;
    if(type == eResourceType::horse) return horseCount();
    return eEmployingBuilding::count(type);
}

int eHorseRanch::add(const eResourceType type, const int count) {
    if(type == eResourceType::wheat) {
        mWheat += count*100;
        return count;
    }
    return eEmployingBuilding::add(type, count);
}

int eHorseRanch::take(const eResourceType type, const int count) {
    if(type == eResourceType::horse) {
        return mEnclosure->take(type, count);
    }
    return eEmployingBuilding::take(type, count);
}

int eHorseRanch::spaceLeft(const eResourceType type) const {
    if(type == eResourceType::wheat) return 8 - mWheat/100;
    return eEmployingBuilding::spaceLeft(type);
}

std::vector<eCartTask> eHorseRanch::cartTasks() const {
    auto tasks = eEmployingBuilding::cartTasks();

    if(800 > mWheat) {
        eCartTask task;
        task.fType = eCartActionType::take;
        task.fResource = eResourceType::wheat;
        task.fMaxCount = 8 - mWheat/100;
        tasks.push_back(task);
    }

    return tasks;
}

int eHorseRanch::provide(const eProvide p, const int n) {
    mEnclosure->eBuildingWithResource::provide(p, n);
    return eEmployingBuilding::provide(p, n);
}

void eHorseRanch::setEnclosure(eHorseRanchEnclosure* const e) {
    mEnclosure = e;
}

int eHorseRanch::horseCount() const {
    return mEnclosure->horseCount();
}

bool eHorseRanch::takeHorse() {
    return mEnclosure->takeHorse();
}

void eHorseRanch::read(eReadStream& src) {
    eEmployingBuilding::read(src);

    src >> mWheat;
    src >> mWheatTime;
    src >> mHorseTime;
    auto& board = getBoard();
    src.readCharacter(&board, [this](eCharacter* const c) {
        mTakeCart = static_cast<eCartTransporter*>(c);
    });
}

void eHorseRanch::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);

    dst << mWheat;
    dst << mWheatTime;
    dst << mHorseTime;
    dst.writeCharacter(mTakeCart);
}
