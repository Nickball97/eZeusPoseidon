#include "eresourcebuilding.h"

#include "textures/egametextures.h"
#include "textures/ebuildingtextures.h"
#include "enumbers.h"
#include "engine/egameboard.h"

#include <algorithm>

eBuildingType resourceTypeToBuildingType(const eResourceBuildingType r) {
    switch(r) {
    case eResourceBuildingType::oliveTree:
        return eBuildingType::oliveTree;
    case eResourceBuildingType::vine:
        return eBuildingType::vine;
    case eResourceBuildingType::orangeTree:
        return eBuildingType::orangeTree;
    }
    return eBuildingType::oliveTree;
}

int resourceTypeToRipePeriod(const eResourceBuildingType r) {
    switch(r) {
    case eResourceBuildingType::oliveTree:
        return eNumbers::sOliveTreeRipePeriod;
    case eResourceBuildingType::vine:
        return eNumbers::sVineRipePeriod;
    case eResourceBuildingType::orangeTree:
        return eNumbers::sOrangeTreeRipePeriod;
    }
    return eNumbers::sOliveTreeRipePeriod;
}

eResourceBuilding::eResourceBuilding(
        eGameBoard& board, const eResourceBuildingType type,
        const eCityId cid) :
    eBuilding(board, resourceTypeToBuildingType(type), 1, 1, cid),
    mType(type), mRipePeriod(resourceTypeToRipePeriod(type)) {
    switch(type) {
    case eResourceBuildingType::oliveTree:
        eGameTextures::loadOliveTree();
        break;
    case eResourceBuildingType::vine:
        eGameTextures::loadVine();
        break;
    case eResourceBuildingType::orangeTree:
        //eGameTextures::loadOrangeTree();
        break;
    }
}

std::shared_ptr<eTexture> eResourceBuilding::getTexture(const eTileSize size) const {
    const auto& texs = eGameTextures::buildings();
    const int sizeId = static_cast<int>(size);
    const auto& colls = texs[sizeId];
    switch(mType) {
    case eResourceBuildingType::oliveTree:
        return colls.fOliveTree.getTexture(mRipe);
    case eResourceBuildingType::vine:
        return colls.fVine.getTexture(mRipe);
    case eResourceBuildingType::orangeTree:
        return colls.fOrangeTree.getTexture(mRipe);
    }
    return std::shared_ptr<eTexture>();
}

void eResourceBuilding::erase() {
    if(mSanctuary) return;
    eBuilding::erase();
}

void eResourceBuilding::sanctuaryErase() {
    eBuilding::erase();
}

bool eResourceBuilding::sIsResourceBuilding(const eBuildingType type) {
    return type == eBuildingType::oliveTree ||
           type == eBuildingType::orangeTree ||
           type == eBuildingType::vine;
}

int eResourceBuilding::takeResource(const int by) {
    if(mResource == 0) return 0;
    const int take = std::clamp(by, 0, mResource);
    mResource -= take;
    if(mResource == 0) {
        mRipe = 0;
        const auto tile = centerTile();
        tile->scheduleTerrainUpdate();
    }
    return take;
}

void eResourceBuilding::workOn() {
    mWorkedOn = true;
    mNextRipe = 0;
}

void eResourceBuilding::setSanctuary(const bool s) {
    mSanctuary = s;
}

bool eResourceBuilding::sHarvestSeason(const eBuildingType type,
                                       const int month) {
    if(type == eBuildingType::vine) return month >= 9; // Oct-Dec
    if(type == eBuildingType::oliveTree) return month <= 2; // Jan-Mar
    return true;
}

void eResourceBuilding::timeChanged(const int by) {
    const auto bt = eBuilding::type();
    if(bt == eBuildingType::vine || bt == eBuildingType::oliveTree) {
        // ripe fruit waits for its harvest season and is lost if the
        // season ends before it is picked; then the tree grows again
        const int m = static_cast<int>(getBoard().date().month());
        const bool season = sHarvestSeason(bt, m);
        const bool seasonEnded = mWasInSeason && !season;
        mWasInSeason = season;
        if(mRipe >= 5) {
            if(seasonEnded) {
                mRipe = 0;
                mResource = 0;
                mNextRipe = 0;
                centerTile()->scheduleTerrainUpdate();
            } else {
                mResource = season ? 1 : 0;
            }
            return;
        }
        mNextRipe += by;
        int wait = mRipePeriod;
        if(mSanctuary || blessed()) {
            wait *= eNumbers::sTreeVineBlessedRipePeriodMultiplier;
        } else if(cursed()) {
            wait *= eNumbers::sTreeVineCursedRipePeriodMultiplier;
        }
        if(mWorkedOn && mNextRipe > wait) {
            mNextRipe -= wait;
            mWorkedOn = false;
            ++mRipe;
            centerTile()->scheduleTerrainUpdate();
        }
        return;
    }
    mNextRipe += by;
    const double mult = eNumbers::sTreeVineFullyRipePeriodMultiplier;
    int wait = mRipe >= 5 ? mult*mRipePeriod : mRipePeriod;
    if(mSanctuary || blessed()) {
        if(mRipe >= 5) {
            wait *= eNumbers::sTreeVineBlessedFullyRipePeriodMultiplier;
        } else {
            wait *= eNumbers::sTreeVineBlessedRipePeriodMultiplier;
        }
    } else if(cursed()) {
        if(mRipe >= 5) {
            wait *= eNumbers::sTreeVineCursedFullyRipePeriodMultiplier;
        } else {
            wait *= eNumbers::sTreeVineCursedRipePeriodMultiplier;
        }
    }
    if((mWorkedOn || mRipe >= 5) && mNextRipe > wait) {
        mNextRipe -= wait;
        mWorkedOn = false;
        if(mRipe >= 5) {
            mRipe = 0;
            mResource = 0;
            return;
        }
        if(++mRipe == 5) {
            mResource = 1;
        }
        const auto tile = centerTile();
        tile->scheduleTerrainUpdate();
    }
}

void eResourceBuilding::read(eReadStream& src) {
    eBuilding::read(src);
    src >> mSanctuary;
    src >> mWorkedOn;
    src >> mNextRipe;
    src >> mRipe;
    src >> mResource;
}

void eResourceBuilding::write(eWriteStream& dst) const {
    eBuilding::write(dst);
    dst << mSanctuary;
    dst << mWorkedOn;
    dst << mNextRipe;
    dst << mRipe;
    dst << mResource;
}
