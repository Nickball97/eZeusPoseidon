#include "eprocessingbuilding.h"

#include "characters/actions/ecarttransporteraction.h"
#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "enumbers.h"

#include <algorithm>

eProcessingBuilding::eProcessingBuilding(
        eGameBoard& board,
        const eBaseTex baseTex,
        const double overlayX,
        const double overlayY,
        const eOverlays overlays,
        const eBuildingType type,
        const int sw, const int sh,
        const int maxEmployees,
        const eResourceType rawMaterial,
        const eResourceType product,
        const int rawUse,
        const int time,
        const eCityId cid) :
    eResourceBuildingBase(board, type, sw, sh,
                          maxEmployees, product, cid),
    mTextures(eGameTextures::buildings()),
    mBaseTex(baseTex), mOverlays(overlays),
    mOverlayX(overlayX), mOverlayY(overlayY),
    mRawMaterial(rawMaterial), mRawUse(rawUse),
    mProcessWaitTime(time) {
    setOverlayEnabledFunc([this]() {
        return enabled() && mRawCount > 0;
    });
    // zeus.exe (0x0048CFB0, workshop types 0x35-0x37 and 0x87) keeps no
    // finished goods: each batch leaves at once on a cart (100 units, a
    // statue 400), and nothing more leaves until that cart is back. One
    // load here plays the same part; a blessed batch may still go over.
    setMaxResource(1);
}

eProcessingBuilding::~eProcessingBuilding() {
    if(mTakeCart) mTakeCart->kill();
}

std::shared_ptr<eTexture> eProcessingBuilding::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return mTextures[sizeId].*mBaseTex;
}

std::vector<eOverlay> eProcessingBuilding::getOverlays(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& coll = mTextures[sizeId].*mOverlays;
    const int texId = textureTime() % coll.size();
    eOverlay o;
    o.fTex = coll.getTexture(texId);
    o.fX = mOverlayX;
    o.fY = mOverlayY;
    return std::vector<eOverlay>({o});
}

void eProcessingBuilding::timeChanged(const int by) {
    if(enabled()) {
        if(!mTakeCart) {
            mTakeCart = spawnCart(eCartActionTypeSupport::take);
            mTakeCart->setMaxDistance(eNumbers::sProcessingBuildingMaxResourceTakeDistance);
        }
        // the original works in staffing steps (exe 0x0048CFB0): full staff
        // 10 a cycle, 75%+ 8, 50%+ 6, 25%+ 4, any 2
        double eff = effectiveness();
        const double filled = vacanciesFilledFraction();
        if(filled > 0.) {
            const double step = filled >= 1. ? 1. :
                                filled >= 0.75 ? 0.8 :
                                filled >= 0.5 ? 0.6 :
                                filled >= 0.25 ? 0.4 : 0.2;
            eff *= step/filled;
        }
        mProcessTime += by*eff;
        const auto product = resourceType();
        if(mProcessTime > mProcessWaitTime && mRawCount >= mRawUse &&
           eResourceBuildingBase::spaceLeft(product) <= 0) {
            // a finished batch waits for the cart, as in the original,
            // where the progress stays until the batch can leave
            mProcessTime = mProcessWaitTime;
        } else if(mProcessTime > mProcessWaitTime) {
            mProcessTime -= mProcessWaitTime;
            if(mRawCount >= mRawUse) {
                const auto type = resourceType();
                // a blessed workshop turns this batch into several
                const int c = addProduced(type, useBlessing(1));
                if(c > 0) mRawCount -= mRawUse;
            }
        }
    }
    eResourceBuildingBase::timeChanged(by);
}

int eProcessingBuilding::count(const eResourceType type) const {
    if(type == mRawMaterial) return mRawCount;
    return eResourceBuildingBase::count(type);
}

int eProcessingBuilding::add(const eResourceType type, const int count) {
    if(type == mRawMaterial) {
        const int r = std::clamp(mRawCount + count, 0, mMaxRaw);
        const int result = r - mRawCount;
        mRawCount = r;
        return result;
    }
    return eResourceBuildingBase::add(type, count);
}

int eProcessingBuilding::spaceLeft(const eResourceType type) const {
    if(type == mRawMaterial) return mMaxRaw - mRawCount;
    return eResourceBuildingBase::spaceLeft(type);
}

std::vector<eCartTask> eProcessingBuilding::cartTasks() const {
    auto tasks = eResourceBuildingBase::cartTasks();

    if(mMaxRaw > mRawCount) {
        eCartTask task;
        task.fType = eCartActionType::take;
        task.fResource = mRawMaterial;
        task.fMaxCount = mMaxRaw - mRawCount;
        tasks.push_back(task);
    }

    return tasks;
}

void eProcessingBuilding::read(eReadStream& src) {
    eResourceBuildingBase::read(src);

    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mTakeCart = static_cast<eCartTransporter*>(c);
    });

    src >> mRawCount;
    src >> mProcessTime;
}

void eProcessingBuilding::write(eWriteStream& dst) const {
    eResourceBuildingBase::write(dst);

    dst.writeCharacter(mTakeCart);

    dst << mRawCount;
    dst << mProcessTime;
}
