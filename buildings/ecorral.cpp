#include "ecorral.h"

#include "textures/egametextures.h"
#include "characters/ebutcher.h"
#include "characters/ecattle.h"
#include "characters/actions/efollowaction.h"
#include "characters/actions/etakecattleaction.h"
#include "characters/actions/ereplacecattleaction.h"
#include "enumbers.h"

#include <algorithm>

eCorral::eCorral(eGameBoard& board,
                 const eCityId cid) :
    eResourceBuildingBase(board, eBuildingType::corral,
                          4, 4, 25, eResourceType::meat,
                          cid) {
    eGameTextures::loadCorral();
}

std::shared_ptr<eTexture> eCorral::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    return blds[sizeId].fCorral;
}

std::vector<eOverlay> eCorral::getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    const auto& texs = blds[sizeId];
    std::vector<eOverlay> result;
    if(resource() > 0) {
        const int res = std::clamp(resource() - 1, 0, 4);
        auto& meat = result.emplace_back();
        meat.fTex = texs.fWaitingMeat.getTexture(res);
        meat.fX = -1.5;
        meat.fY = -3;
    }
    if(mNCattle == 0) {
        const auto& coll = texs.fCorralOverlay;
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fTex = coll.getTexture(texId);
        o.fX = -1.00;
        o.fY = -5.20;
    } else if(mNCattle == 1) {
        const auto& coll = texs.fCorralCattleOverlay[0];
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fAlignTop = true;
        o.fTex = coll.getTexture(texId);
        o.fX = 1.38;
        o.fY = -2.95;
    } else if(mNCattle == 2) {
        const auto& coll = texs.fCorralCattleOverlay[1];
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fAlignTop = true;
        o.fTex = coll.getTexture(texId);
        o.fX = 1.38;
        o.fY = -2.98;
    } else { // mNCattle == 3
        const auto& coll = texs.fCorralCattleOverlay[2];
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fTex = coll.getTexture(texId);
        o.fX = -0.08;
        o.fY = -4.44;
    }
    if(mNCattle > 0) {
        eGameTextures::loadCattle();
        const auto& chars = eGameTextures::characters();
        const auto& texs = chars[sizeId];
        const auto& coll = texs.fCattle1.fStand;
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fTex = coll[2].getTexture(texId);
        o.fX = 1.33;
        o.fY = -2.47;
    }
    if(mProcessing > 0) {
        const auto& coll = texs.fCorralProcessingOverlay;
        const int texId = textureTime() % coll.size();
        auto& o = result.emplace_back();
        o.fTex = coll.getTexture(texId);
        o.fX = -3.33;
        o.fY = -4.97;
    }
    return result;
}

void eCorral::read(eReadStream& src) {
    eResourceBuildingBase::read(src);
    src >> mNoCattle;
    src >> mProcessing;
    src >> mNCattle;
    src >> mTakeWait;
    src >> mKillWait;
    src >> mReplaceWait;
}

void eCorral::write(eWriteStream& dst) const {
    eResourceBuildingBase::write(dst);
    dst << mNoCattle;
    dst << mProcessing;
    dst << mNCattle;
    dst << mTakeWait;
    dst << mKillWait;
    dst << mReplaceWait;
}

void eCorral::timeChanged(const int by) {
    eResourceBuildingBase::timeChanged(by);

    const double eff = effectiveness();
    const double newP = std::max(0., mProcessing - by*eff);
    if(newP == 0. && mProcessing > 0) {
        // each cow gives 2 loads of meat (Citadel of Poseidon)
        addProduced(eResourceType::meat, useBlessing(2));
    }

    mProcessing = newP;

    // original exe 0x004F9960: a fetcher leaves every 1/4/8/16/32 cycles
    // by staffing (full, 75%+, 50%+, 25%+, any), at most 2 out at once and
    // never more cattle held and on the way than 3
    mFetchers.erase(std::remove_if(mFetchers.begin(), mFetchers.end(),
                                   [](const stdptr<eCharacter>& c) {
        return !c || c->dead();
    }), mFetchers.end());
    const int out = static_cast<int>(mFetchers.size());
    const double filled = vacanciesFilledFraction();
    if(eff > 0. && filled > 0. && out < 2 && mNCattle + out < 3) {
        const int cycles = filled >= 1. ? 1 :
                           filled >= 0.75 ? 4 :
                           filled >= 0.5 ? 8 :
                           filled >= 0.25 ? 16 : 32;
        const int cycle = (365*eNumbers::sDayLength)/(12*16);
        mTakeWait += by;
        if(mTakeWait > cycles*cycle) {
            mTakeWait = 0;
            takeCattle();
        }
    }
    if(mNCattle > 0) {
        mKillWait += by*eff;
        // waits until the slaughter in progress is done (original)
        if(mKillWait > eNumbers::sCorralKillPeriod && killCattle()) {
            mKillWait = 0;
        }
    }
    mReplaceWait += by*eff;
    if(mReplaceWait > eNumbers::sCorralReplacePeriod) {
        mReplaceWait = 0;
        replaceCattle();
    }
}

void eCorral::addCattle() {
    mNCattle = std::clamp(mNCattle + 1, 0, 3);
    if(mNCattle == 1) mKillWait = 0;
}

void eCorral::setNoCattle(const bool c) {
    mNoCattle = c;
}

bool eCorral::killCattle() {
    if(mProcessing > 0) return false;
    if(mNCattle < 1) return false;
    const int space = spaceLeft(eResourceType::meat);
    if(space < 3) return false;
    mNCattle--;
    mProcessing = eNumbers::sCorralProcessingPeriod;
    return true;
}

bool eCorral::takeCattle() {
    if(mNCattle >= 3) return false;
    auto& board = getBoard();
    const auto c = e::make_shared<eButcher>(board);
    mFetchers.push_back(c.get());
    const auto cid = cityId();
    c->setBothCityIds(cid);
    const auto tile = centerTile();
    c->changeTile(tile);
    const auto a = e::make_shared<eTakeCattleAction>(c.get(), this);
    c->setAction(a);
    return true;
}

void eCorral::replaceCattle() {
    auto& board = getBoard();
    const auto c = e::make_shared<eButcher>(board);
    const auto cid = cityId();
    c->setBothCityIds(cid);
    const auto tile = centerTile();
    c->changeTile(tile);
    const auto cc = e::make_shared<eCattle>(board, eCharacterType::cattle1);
    cc->setBothCityIds(cid);
    cc->changeTile(tile);
    const auto fa = e::make_shared<eFollowAction>(c.get(), cc.get());
    cc->setAction(fa);
    const auto a = e::make_shared<eReplaceCattleAction>(c.get(), cc.get());
    c->setAction(a);
}
