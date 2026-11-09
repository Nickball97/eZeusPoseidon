#include "eartisansguild.h"

#include "textures/egametextures.h"
#include "characters/actions/eartisanaction.h"
#include "engine/egameboard.h"
#include "buildings/sanctuaries/esanctuary.h"
#include "fileIO/efileformat.h"

eArtisansGuild::eArtisansGuild(eGameBoard& board, const eCityId cid) :
    eEmployingBuilding(board, eBuildingType::artisansGuild, 2, 2, 25, cid) {
    eGameTextures::loadArtisansGuild();
}

eArtisansGuild::~eArtisansGuild() {
    if(mArtisan) mArtisan->kill();
    if(mArtisan2) mArtisan2->kill();
}

std::shared_ptr<eTexture> eArtisansGuild::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];
    return texs.fArtisansGuild;
}

std::vector<eOverlay> eArtisansGuild::
    getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];

    const auto& coll = texs.fArtisansGuildOverlay;
    const int texId = textureTime() % coll.size();
    eOverlay o;
    o.fTex = coll.getTexture(texId);
    o.fX = -1.45;
    o.fY = -3.0;

    return {o};
}

void eArtisansGuild::timeChanged(const int by) {
    if(enabled()) {
        if(!mArtisan) {
            mSpawnTime += by*effectiveness();
            if(mSpawnTime > mWaitTime) {
                mSpawnTime = 0;
                spawnArtisan(&eArtisansGuild::mArtisan);
            }
        } else if(!mArtisan2) {
            const auto s = getBoard().sanctuary(cityId(), eGodType::atlas);
            if(s && s->finished()) {
                mSpawnTime += by*effectiveness();
                if(mSpawnTime > mWaitTime) {
                    mSpawnTime = 0;
                    spawnArtisan(&eArtisansGuild::mArtisan2);
                }
            }
        }
    }
    eEmployingBuilding::timeChanged(by);
}

bool eArtisansGuild::spawnArtisan(const eArtisanPtr artisan) {
    const auto t = centerTile();
    const auto ar = e::make_shared<eArtisan>(getBoard());
    this->*artisan = ar.get();
    ar->setBothCityIds(cityId());
    ar->changeTile(t);
    const auto a = e::make_shared<eArtisanAction>(ar.get(), this);
    ar->setAction(a);
    return true;
}

void eArtisansGuild::read(eReadStream& src) {
    eEmployingBuilding::read(src);
    src >> mSpawnTime;
    auto& board = getBoard();
    src.readCharacter(&board, [this](eCharacter* const c) {
        mArtisan = static_cast<eArtisan*>(c);
    });
    if(src.formatVersion() >= eFileFormat::artisansSecond) {
        src.readCharacter(&board, [this](eCharacter* const c) {
            mArtisan2 = static_cast<eArtisan*>(c);
        });
    }
}

void eArtisansGuild::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);
    dst << mSpawnTime;
    dst.writeCharacter(mArtisan);
    dst.writeCharacter(mArtisan2);
}

void eArtisansGuild::killArtisan() {
    if(mArtisan) mArtisan->killWithCorpse();
    if(mArtisan2) mArtisan2->killWithCorpse();
}
