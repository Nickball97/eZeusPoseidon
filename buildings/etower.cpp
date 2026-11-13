#include "etower.h"
#include "missiles/eatlanteanfiremissile.h"
#include "fileIO/efileformat.h"
#include "engine/egameboard.h"
#include "characters/efigurestats.h"

#include "textures/egametextures.h"

#include "characters/actions/earcheraction.h"
#include "vec2.h"
#include "missiles/earrowmissile.h"
#include "characters/efightingcharacter.h"
#include "characters/actions/efightingaction.h"
#include "audio/esounds.h"
#include "enumbers.h"

eTower::eTower(eGameBoard& board, const eCityId cid) :
    eEmployingBuilding(board, eBuildingType::tower, 2, 2, 15, cid) {
    eGameTextures::loadGatehouseAndTower();
    setHP(eNumbers::sTowerHP);
    if(atlantean()) {
        eGameTextures::loadPoseidonTowerArcher();
    } else {
        eGameTextures::loadArcher();
    }
}

eTower::~eTower() {
    if(mArcher) mArcher->kill();
    if(!mDeleteArchers) return;
    const auto& tiles = tilesUnder();
    for(const auto tile : tiles) {
        if(!tile) continue;
        const auto chars = tile->characters();
        for(const auto& c : chars) {
            const auto type = c->type();
            if(type == eCharacterType::archer) c->kill();
        }
    }
}

std::shared_ptr<eTexture>
eTower::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings();
    return texs[sizeId].fTower;
}

std::vector<eOverlay>
eTower::getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::characters()[sizeId];
    const eArcherTextures* aTexs;
    if(atlantean()) {
        aTexs = &texs.fPoseidonTowerArcher;
    } else {
        aTexs = &texs.fArcher;
    }
    if(mAttack) {
        const int oid = static_cast<int>(mAttackOrientation);
        const auto& coll = &aTexs->fFight[oid];
        const int t = time()/20;
        const int texId = t % coll->size();
        eOverlay o;
        o.fTex = coll->getTexture(texId);
        o.fX = -3.5;
        o.fY = -5.5;
        return {o};
    } else {
        const auto& colls = aTexs->fPatrol;
        const int tt = textureTime();
        const int idid = tt/50 % 8;
        const int ids[8] = {5, 0, 3, 7, 4, 2, 6, 1};
        const int id = ids[idid];
        const auto& coll = colls[id];
        eOverlay o;
        o.fTex = coll.getTexture(tt % coll.size());
        o.fX = -3.5;
        o.fY = -5.5;
        return {o};
    }
}

void eTower::timeChanged(const int by) {
    // Athena's curse leaves towers unmanned (Zeus manual)
    if(cursed()) {
        if(mArcher) mArcher->kill();
    } else if(enabled()) {
        // Atlantean Towers (Poseidon manual): orichalc is delivered
        // automatically if the metal is available; one load fits the
        // Atlantean Fire launcher
        if(!mOrichalc) {
            mOrichalcCheck += by;
            if(mOrichalcCheck > eNumbers::sDayLength) {
                mOrichalcCheck = 0;
                auto& board = getBoard();
                const auto cid = cityId();
                if(board.atlantean(cid) &&
                   board.takeResource(cid, eResourceType::orichalc, 1) > 0) {
                    mOrichalc = true;
                }
            }
        }
        if(!mArcher) {
            mSpawnTime += by;
            if(mSpawnTime > eNumbers::sTowerSpawnPeriod) {
                spawn();
                mSpawnTime = 0;
            }
        }

        const int rangeAttackCheck = 500;
        const int missileCheck = 200;
        // the tower's shooter from the figure file ("Tower Sentry")
        const int range = eFigureStats::sTowerSentry(
                              eFigureStats::sDifficulty(getBoard())).fMissileRange;

        const auto ct = centerTile();
        const int tx = ct->x();
        const int ty = ct->y();
        const vec2d cpos{1.*tx, 1.*ty};
        const auto tid = teamId();
        auto& brd = getBoard();

        if(mAttack) {
            if(range > 0 && mAttackTarget) {
                mMissile += by;
                if(mMissile > missileCheck) {
                    mMissile = mMissile - missileCheck;
                    const auto tt = mAttackTarget->tile();
                    const int ttx = tt->x();
                    const int tty = tt->y();
                    if(mOrichalc) {
                        eMissile::sCreate<eAtlanteanFireMissile>(
                            brd, tx, ty, 3.5, ttx, tty, 0.5, 2);
                    } else {
                        eMissile::sCreate<eArrowMissile>(brd, tx, ty, 3.5,
                                                         ttx, tty, 0.5, 2);
                    }
                    auto& board = getBoard();
                    board.ifVisible(centerTile(), [&]() {
                        eSounds::playAttackSound(eCharacterType::archer);
                    });
                }
            }
            mAttackTime += by;
            bool finishAttack = !mAttackTarget ||
                                mAttackTarget->dead() ||
                                mAttackTime > 1000;
            if(mAttackTarget && !mAttackTarget->dead()) {
                double att = eFigureStats::sTowerDamage(
                                 getBoard(), mAttackTarget.get(), by);
                if(mOrichalc && att > 0) {
                    // Atlantean Fire: same strength as on frigates (per
                    // Nicolas): 1.5x damage, target burns two game days
                    // at half the tower's damage rate
                    mAttackTarget->setBurning(by > 0 ? 0.5*att/by : 0.,
                                              2*eNumbers::sDayLength);
                    att *= 1.5;
                }
                const bool d = mAttackTarget->defend(att);
                if(d) finishAttack = true;
            }
            if(finishAttack) {
                mAttack = false;
                mAttackTarget = nullptr;
                mAttackTime = 0;
                mRangeAttack = rangeAttackCheck;
            } else {
                return;
            }
        }

        mRangeAttack += by;
        if(mRangeAttack > rangeAttackCheck) {
            mRangeAttack = mRangeAttack - rangeAttackCheck;
            for(int i = -range; i <= range; i++) {
                for(int j = -range; j <= range; j++) {
                    const auto t = brd.tile(tx + i, ty + j);
                    if(!t) continue;
                    const auto& chars = t->characters();
                    for(const auto& cc : chars) {
                        // towers shoot wolves too (upstream #20, #54)
                        if(!cc->isFighter() &&
                           cc->type() != eCharacterType::wolf) continue;
                        const auto cctid = cc->teamId();
                        if(!eTeamIdHelpers::isEnemy(cctid, tid)) continue;
                        if(cc->hiddenUnderwater()) continue;
                        if(cc->dead()) continue;
                        const vec2d ccpos{cc->absX(), cc->absY()};
                        const vec2d posdif = ccpos - cpos;
                        mAttackTarget = cc;
                        mAttack = true;
                        mAttackTime = 0;
                        const double angle = posdif.angle();
                        const auto o = sAngleOrientation(angle);
                        mAttackOrientation = o;

                        const auto tt = cc->tile();
                        const int ttx = tt->x();
                        const int tty = tt->y();
                        eFightingAction::sSignalBeingAttack(cc.get(), ttx, tty, brd);

                        return;
                    }
                }
            }
        }
    } else {
        if(mArcher) mArcher->kill();
    }
    eEmployingBuilding::timeChanged(by);
}

void eTower::read(eReadStream& src) {
    eEmployingBuilding::read(src);
    src >> mMissile;
    src >> mRangeAttack;
    src >> mAttackTime;
    src >> mAttack;
    src >> mAttackOrientation;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mAttackTarget = c;
    });
    src >> mSpawnTime;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mArcher = static_cast<eArcher*>(c);
    });
    if(src.formatVersion() >= eFileFormat::towerOrichalc) {
        src >> mOrichalc;
    }
}

void eTower::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);
    dst << mMissile;
    dst << mRangeAttack;
    dst << mAttackTime;
    dst << mAttack;
    dst << mAttackOrientation;
    dst.writeCharacter(mAttackTarget);
    dst << mSpawnTime;
    dst.writeCharacter(mArcher);
    dst << mOrichalc;
}

bool eTower::spawn() {
    const auto archer = e::make_shared<eArcher>(getBoard());
    archer->setBothCityIds(cityId());
    archer->setAtlantean(atlantean());
    archer->changeTile(centerTile());
    const auto a = e::make_shared<eArcherAction>(archer.get());
    archer->setAction(a);
    mArcher = archer;
    return true;
}

