#include "ehuntinglodge.h"

#include "characters/ehunter.h"
#include "textures/egametextures.h"
#include "characters/actions/ehuntaction.h"
#include "enumbers.h"
#include "fileIO/efileformat.h"

#include <algorithm>

eHuntingLodge::eHuntingLodge(eGameBoard& board,
                             const eCityId cid) :
    eResourceCollectBuildingBase(board, eBuildingType::huntingLodge,
                          2, 2, 8, eResourceType::meat, cid) {
    eGameTextures::loadHuntingLodge();
}

eHuntingLodge::~eHuntingLodge() {
    for(const auto& h : mHunters) {
        if(h) h->kill();
    }
}

std::shared_ptr<eTexture> eHuntingLodge::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return eGameTextures::buildings()[sizeId].fHuntingLodge;
}

std::vector<eOverlay> eHuntingLodge::getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];
    const auto& coll = texs.fHuntingLodgeOverlay;
    const int texId = textureTime() % coll.size();
    eOverlay o;
    o.fTex = coll.getTexture(texId);
    o.fX = -1.95;
    o.fY = -2.4;
    if(resource() > 0) {
        eOverlay meat;
        const int res = std::clamp(resource() - 1, 0, 4);
        meat.fTex = texs.fWaitingMeat.getTexture(res);
        meat.fX = -0.5;
        meat.fY = -2;

        return std::vector<eOverlay>({o, meat});
    }
    return std::vector<eOverlay>({o});
}

void eHuntingLodge::timeChanged(const int by) {
    eResourceCollectBuildingBase::timeChanged(by);
    if(hunterCount() < 3) {
        const double eff = effectiveness();
        mSpawnTime += by*eff;
        const int wait = eNumbers::sHuntingLodgeWaitPeriod;
        if(mSpawnTime > wait) {
            mSpawnTime -= wait;
            spawn();
        }
    }
}

void eHuntingLodge::read(eReadStream& src) {
    eResourceCollectBuildingBase::read(src);
    src >> mSpawnTime;
    int n = 1;
    if(src.formatVersion() >= eFileFormat::huntingLodgeHunters) src >> n;
    mHunters.clear();
    for(int i = 0; i < n; i++) {
        src.readCharacter(&getBoard(), [this](eCharacter* const c) {
            if(c) mHunters.push_back(static_cast<eHunter*>(c));
        });
    }
}

void eHuntingLodge::write(eWriteStream& dst) const {
    eResourceCollectBuildingBase::write(dst);
    dst << mSpawnTime;
    std::vector<eHunter*> hs;
    for(const auto& h : mHunters) {
        if(h) hs.push_back(h.get());
    }
    dst << static_cast<int>(hs.size());
    for(const auto h : hs) dst.writeCharacter(h);
}

int eHuntingLodge::hunterCount() {
    mHunters.erase(std::remove_if(mHunters.begin(), mHunters.end(),
                                  [](const stdptr<eHunter>& h) {
        return !h;
    }), mHunters.end());
    return mHunters.size();
}

bool eHuntingLodge::spawn() {
    const auto t = centerTile();
    const auto h = e::make_shared<eHunter>(getBoard());
    mHunters.push_back(h.get());
    h->setBothCityIds(cityId());
    h->setAtlantean(atlantean());
    h->changeTile(t);
    const auto a = e::make_shared<eHuntAction>(this, h.get());
    h->setAction(a);
    return true;
}
