#include "eresourcecollectbuilding.h"

#include "characters/eresourcecollector.h"
#include "characters/actions/ecollectresourceaction.h"
#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "enumbers.h"
#include "fileIO/efileformat.h"

#include <algorithm>

eResourceCollectBuilding::eResourceCollectBuilding(
        eGameBoard& board,
        const eBaseTex baseTex,
        const double overlayX,
        const double overlayY,
        const eOverlays overlays,
        const int waitingOO,
        const double waitingOX,
        const double waitingOY,
        const eCharGenerator& charGen,
        const eBuildingType type,
        const stdsptr<eHasResourceObject>& hr,
        const int sw, const int sh,
        const int maxEmployees,
        const eResourceType resType,
        const eCityId cid) :
    eResourceCollectBuildingBase(board, type, sw, sh,
                                 maxEmployees, resType, cid),
    mCharGenerator(charGen),
    mTextures(eGameTextures::buildings()),
    mBaseTex(baseTex), mOverlays(overlays),
    mOverlayX(overlayX), mOverlayY(overlayY),
    mWaitingOO(waitingOO),
    mWaitingOX(waitingOX), mWaitingOY(waitingOY),
    mHasRes(hr) {
    eGameTextures::loadWaitingOverlay();
}

eResourceCollectBuilding::~eResourceCollectBuilding() {
    for(const auto& c : mCollectors) {
        if(c) c->kill();
    }
}

std::shared_ptr<eTexture> eResourceCollectBuilding::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return mTextures[sizeId].*mBaseTex;
}

std::vector<eOverlay> eResourceCollectBuilding::
    getOverlays(const eTileSize size) const {
    std::vector<eOverlay> result;
    const int sizeId = static_cast<int>(size);
    const auto& btexs = eGameTextures::buildings()[sizeId];
    if(mRawCount <= 0) {
        eOverlay& o = result.emplace_back();
        const int wo = seed() % 2;
        const std::vector<eTextureCollection>* coll;
        if(wo) {
            coll = &btexs.fWaitingOverlay0;
        } else {
            coll = &btexs.fWaitingOverlay1;
        }
        const auto& ov = (*coll)[mWaitingOO];
        const int t = textureTime();
        const int id = t % ov.size();
        o.fTex = ov.getTexture(id);
        o.fX = mWaitingOX;
        o.fY = mWaitingOY;
        result.push_back(o);
    } else if(mOverlays) {
        const auto& coll = btexs.*mOverlays;
        const int texId = textureTime() % coll.size();
        eOverlay& o = result.emplace_back();
        o.fTex = coll.getTexture(texId);
        o.fX = mOverlayX;
        o.fY = mOverlayY;
    }
    if(resource() > 0) {
        const auto& texs = mTextures[sizeId];
        const eTextureCollection* coll = nullptr;
        double x = -0.5;
        double y = -2;
        switch(resourceType()) {
        case eResourceType::meat:
            coll = &texs.fWaitingMeat;
            break;
        case eResourceType::cheese:
            coll = &texs.fWaitingCheese;
            break;
        case eResourceType::wheat:
            coll = &texs.fWaitingWheat;
            break;
        case eResourceType::wood:
            coll = &texs.fWaitingWood;
            x += 0.25;
            y -= 0.50;
            break;
        case eResourceType::bronze:
            coll = &texs.fWaitingBronze;
            break;
        case eResourceType::orichalc:
            x += 0.25;
            y -= 0.50;
            coll = &texs.fWaitingOrichalc;
            break;
        case eResourceType::grapes:
            coll = &texs.fWaitingGrapes;
            break;
        case eResourceType::olives:
            coll = &texs.fWaitingOlives;
            break;
        case eResourceType::armor:
            coll = &texs.fWaitingArmor;
            break;
        default: break;
        }

        if(coll) {
            const int resMax = coll->size() - 1;
            const int res = std::clamp(resource() - 1, 0, resMax);
            eOverlay& resO = result.emplace_back();
            resO.fTex = coll->getTexture(res);
            resO.fX = x;
            resO.fY = y;
        }
    }
    return result;
}

void eResourceCollectBuilding::timeChanged(const int by) {
    eResourceBuildingBase::timeChanged(by);
    if(enabled()) {
        if(mRawCount >= mRawPerLoad) {
            mProcessTime += by*effectiveness();
            if(mProcessTime > mProcessDuration) {
                mProcessTime -= mProcessDuration;
                const auto type = resourceType();
                if(type == eResourceType::silver) {
                    auto& brd = getBoard();
                    const auto pid = playerId();
                    brd.incDrachmas(pid, useBlessing(1)*eNumbers::sMintDrachmasPerSilver,
                                    eFinanceTarget::minedSilver);
                    mRawCount -= mRawPerLoad;
                } else {
                    // a blessed building turns this load into several
                    const int c = addProduced(type, useBlessing(1));
                    if(c > 0) mRawCount -= mRawPerLoad;
                }
                if(mRawCount <= mRawCountCollect) enableSpawn();
            }
        }

        if(collectorCount() < mMaxCollectors && mSpawnEnabled) {
            mSpawnTime += by*effectiveness();
            if(mSpawnTime > mWaitTime) {
                mSpawnTime -= mWaitTime;
                spawn();
            }
        }
    }
}

void eResourceCollectBuilding::read(eReadStream& src) {
    eResourceCollectBuildingBase::read(src);
    src >> mCollectedAction;
    const bool multi = src.formatVersion() >= eFileFormat::multiCollectors;
    int nc = 1;
    if(multi) src >> nc;
    mCollectors.clear();
    for(int i = 0; i < nc; i++) {
        src.readCharacter(&getBoard(), [this](eCharacter* const c) {
            if(!c) return;
            mCollectors.push_back(static_cast<eResourceCollectorBase*>(c));
        });
    }
    const int rawCountCollect = mRawCountCollect;
    const int waitTime = mWaitTime;
    src >> mSpawnEnabled;
    src >> mAddResource;
    src >> mRawCount;
    src >> mRawCountCollect;
    src >> mRawInc;
    src >> mProcessDuration;
    src >> mProcessTime;
    src >> mWaitTime;
    src >> mSpawnTime;
    if(!multi) {
        // Older saves counted whole loads and used the old timings.
        mRawCount *= mRawPerLoad;
        mRawCountCollect = rawCountCollect;
        mWaitTime = waitTime;
        mSpawnTime = std::min(mSpawnTime, double(mWaitTime));
    }
}

void eResourceCollectBuilding::write(eWriteStream& dst) const {
    eResourceCollectBuildingBase::write(dst);
    dst << mCollectedAction;
    {
        std::vector<eResourceCollectorBase*> cs;
        for(const auto& c : mCollectors) {
            if(c) cs.push_back(c.get());
        }
        dst << static_cast<int>(cs.size());
        for(const auto c : cs) dst.writeCharacter(c);
    }
    dst << mSpawnEnabled;
    dst << mAddResource;
    dst << mRawCount;
    dst << mRawCountCollect;
    dst << mRawInc;
    dst << mProcessDuration;
    dst << mProcessTime;
    dst << mWaitTime;
    dst << mSpawnTime;
}

bool eResourceCollectBuilding::spawn() {
    if(resource() >= maxResource()) return false;
    if(!mSpawnEnabled) return false;
    const auto t = centerTile();
    const auto c = mCharGenerator();
    c->setBothCityIds(cityId());
    mCollectors.push_back(c.get());
    c->changeTile(t);

    const auto a = e::make_shared<eCollectResourceAction>(
                       this, c.get(), mHasRes);
    a->setAddResource(mAddResource);
    a->setCollectedAction(mCollectedAction);
    switch(resourceType()) {
    case eResourceType::silver:
    case eResourceType::bronze:
        a->setGetAtTile(false);
    default: break;
    }
    c->setAction(a);
    return true;
}

int eResourceCollectBuilding::collectorCount() {
    mCollectors.erase(std::remove_if(mCollectors.begin(), mCollectors.end(),
                                     [](const stdptr<eResourceCollectorBase>& c) {
        return !c;
    }), mCollectors.end());
    return mCollectors.size();
}

void eResourceCollectBuilding::addRaw() {
    mRawCount += mRawInc;
    if(mRawCount > mRawCountCollect) disableSpawn();
}

void eResourceCollectBuilding::setCollectedAction(const eTileActionType a) {
    mCollectedAction = a;
}
