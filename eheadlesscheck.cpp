#define _CRT_SECURE_NO_WARNINGS // plain fopen is fine here

#include "eheadlesscheck.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "egamedir.h"
#include "elanguage.h"
#include "emessages.h"
#include "enumbers.h"
#include "erand.h"
#include "esettings.h"

#include "audio/emusic.h"
#include "audio/esounds.h"

#include "buildings/ebuilding.h"
#include "buildings/ebuildingregistry.h"
#include "buildings/estoragebuilding.h"
#include "buildings/sanctuaries/esanctuaryblueprint.h"
#include "characters/echaracter.h"

#include "engine/ai/eaidistrict.h"
#include "engine/ecampaign.h"
#include "engine/egameboard.h"
#include "engine/ethreadpool.h"

#include "fileIO/ereadstream.h"
#include "fileIO/ewritestream.h"
#include "fileIO/efileformat.h"

#include "textures/egametextures.h"
#include "widgets/egamewidget.h" // eGameWidgetSettings, part of a save

// ---- hunting for values that are saved or used before they are set ----
//
// With --fill N every freshly allocated piece of memory is first filled
// with the byte N. A value the code forgets to set then shows up as that
// byte: two runs with different N differ exactly where such a value is
// saved (or, worse, used). Off unless asked for; the game never turns it
// on.
static bool gFillMemory = false;
static unsigned char gFillByte = 0;

static void* filledAlloc(const std::size_t size) {
    const auto p = std::malloc(size ? size : 1);
    if(!p) throw std::bad_alloc();
    if(gFillMemory) std::memset(p, gFillByte, size);
    return p;
}

void* operator new(std::size_t size) { return filledAlloc(size); }
void* operator new[](std::size_t size) { return filledAlloc(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {

const int gTicksPerStep = 100; // what the game does at 100% speed
const int gTicksPerDay = 350;
const int gDaysPerYear = 365;
// the test city is looked after every 2 days (7 steps of 100 ticks)
const int gChunkDays = 2;

struct eArgs {
    std::string fAdventure;
    int fYears = 1;
    unsigned int fSeed = 1;
    std::string fOut;
    bool fCity = true;
    int fReloadDays = 60;
    bool fThreads = false;
    bool fKeepSaves = false;
    int fTrace = 0; // days between trace saves; 0 for none
    int fTraceKeepFrom = 1;
    int fTraceKeepTo = 0;
    int fTraceStepsFrom = 1; // days between which every step is traced
    int fTraceStepsTo = 0;
    std::string fRandLog; // file for eRand::setLog
    std::string fRandDiffA;
    std::string fRandDiffB;
    std::vector<long long> fSaveWho; // bytes of the first save to explain
    std::vector<long long> fSave2Who; // and of the save made after reloading
    bool fBuildingTable = false; // only check the table of building kinds
};

bool parseArgs(const int argc, char** const argv, eArgs& args) {
    std::vector<std::string> pos;
    for(int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        const auto next = [&]() -> std::string {
            if(i + 1 >= argc) return "0";
            return argv[++i];
        };
        if(a == "--headless-check") {
        } else if(a == "--seed") {
            args.fSeed = static_cast<unsigned int>(std::stoul(next()));
        } else if(a == "--out") {
            args.fOut = next();
        } else if(a == "--no-city") {
            args.fCity = false;
        } else if(a == "--reload-days") {
            args.fReloadDays = std::stoi(next());
        } else if(a == "--save-who") {
            args.fSaveWho.push_back(std::stoll(next()));
        } else if(a == "--save2-who") {
            args.fSave2Who.push_back(std::stoll(next()));
        } else if(a == "--building-table") {
            args.fBuildingTable = true;
        } else if(a == "--rand-log") {
            args.fRandLog = next();
        } else if(a == "--rand-diff") {
            args.fRandDiffA = next();
            args.fRandDiffB = next();
        } else if(a == "--fill") {
            gFillByte = static_cast<unsigned char>(std::stoi(next()));
            gFillMemory = true;
        } else if(a == "--trace") {
            args.fTrace = std::stoi(next());
        } else if(a == "--trace-steps") {
            args.fTraceStepsFrom = std::stoi(next());
            args.fTraceStepsTo = std::stoi(next());
        } else if(a == "--trace-keep") {
            args.fTraceKeepFrom = std::stoi(next());
            args.fTraceKeepTo = std::stoi(next());
        } else if(a == "--keep-saves") {
            args.fKeepSaves = true;
        } else if(a == "--threads") {
            args.fThreads = true;
        } else {
            pos.push_back(a);
        }
    }
    if(!args.fRandDiffA.empty()) return true;
    if(pos.size() != 2) return false;
    args.fAdventure = pos[0];
    args.fYears = std::stoi(pos[1]);
    return args.fYears >= 0;
}

struct eCounters {
    std::map<int, int> fEvents; // by eEvent number
    int fMessages = 0; // messages shown without an event
    int fGoalsFulfilled = 0; // times the episode's goals were all met
    std::string fLog; // every event, with its date and place
};

void connect(eGameBoard& board, eCounters& counters) {
    // the game window normally answers these; here they are only counted
    const auto boardPtr = &board;
    board.setEventHandler([&counters, boardPtr](const eEvent e, eEventData& ed) {
        const auto& date = boardPtr->date();
        std::ostringstream o;
        o << "  " << date.day() << "/" << (static_cast<int>(date.month()) + 1)
          << "/" << date.year() << ": event " << static_cast<int>(e);
        if(ed.fTile) {
            o << " at " << ed.fTile->x() << "," << ed.fTile->y();
            const auto ub = ed.fTile->underBuilding();
            if(ub) o << " (" << eBuilding::sNameForBuilding(ub->type()) << ")";
        }
        o << "\n";
        counters.fLog += o.str();
        counters.fEvents[static_cast<int>(e)]++;
    });
    board.setMessageShower([&counters](eEventData&, const eMessageType&) {
        counters.fMessages++;
    });
    board.setEpisodeFinishedHandler([&counters]() {
        counters.fGoalsFulfilled++;
    });
}

// One step of the game, the same one the game window runs at 100% speed.
void step(eGameBoard& board) {
    eRand::logMark(static_cast<unsigned int>(board.totalTime()/gTicksPerStep));
    board.step(gTicksPerStep, true);
}

// ---- the test city ----
//
// A small city laid out the same way on every map, built in three goes the
// way a player would: homes, water, safety and food first; then taxes,
// culture and storage; then the big buildings. Where a building goes only
// depends on the map, never on chance.

const int gBlockW = 24;
const int gBlockH4 = 33; // room for four streets
const int gBlockH3 = 24;
const int gBlockH2 = 15;
const int gStageDays[] = {0, 180, 540};
// a walker in the way, or the ruin still burning: try again later
const int gBuildTries = 60;

class eTestCity {
public:
    eTestCity(eGameBoard& board) : mBoard(board) {}

    // call every now and then with the days run so far
    void update(const int day);
    std::string report() const;
private:
    struct eItem {
        int fStage;
        eBuildingType fType;
        int fLX;
        int fLY;
        int fW;
        int fH;
        int fTries = 0;
        bool fBuilt = false;
    };

    void start();
    void plan();
    bool place(const eBuildingType type, const int lx, const int ly,
               const int w, const int h);
    void add(const int stage, const eBuildingType type,
             const int lx, const int ly, const int w, const int h);
    void buildFarms();
    void buildPalace();
    bool roadTo(const int x, const int y, const int w, const int h);
    void buildHerd(const eBuildingType type, int& placed);
    bool free(const int x, const int y, const int w, const int h,
              const bool fertile = false) const;
    bool inBlock(const int x, const int y, const int w, const int h) const;
    void sortTiles(const int x, const int y);

    eGameBoard& mBoard;
    ePlayerId mPid{};
    eCityId mCid{};
    bool mStarted = false;
    bool mSite = false;
    int mX0 = 0;
    int mY0 = 0;
    int mBlockH = 0;
    int mStreets = 0;
    std::vector<eTile*> mTiles;
    std::vector<eItem> mItems;
    int mFarms = 0;
    int mFarmRoads = 0;
    bool mPalace = false;
    int mSheep = 0;
    int mGoats = 0;
    int mRebuilt = 0;
};

bool eTestCity::free(const int x, const int y, const int w, const int h,
                     const bool fertile) const {
    return mBoard.canBuildBase(x, x + w, y, y + h, !fertile,
                               mCid, mPid, fertile);
}

bool eTestCity::inBlock(const int x, const int y,
                        const int w, const int h) const {
    return x + w > mX0 - 1 && x < mX0 + gBlockW + 1 &&
           y + h > mY0 - 1 && y < mY0 + mBlockH + 1;
}

void eTestCity::sortTiles(const int x, const int y) {
    const auto dist2 = [x, y](const eTile* const t) {
        const long long dx = t->x() - x;
        const long long dy = t->y() - y;
        return dx*dx + dy*dy;
    };
    std::stable_sort(mTiles.begin(), mTiles.end(),
                     [&](const eTile* const a, const eTile* const b) {
        const auto da = dist2(a);
        const auto db = dist2(b);
        if(da != db) return da < db;
        if(a->x() != b->x()) return a->x() < b->x();
        return a->y() < b->y();
    });
}

bool eTestCity::place(const eBuildingType type, const int lx, const int ly,
                      const int w, const int h) {
    eAIBuilding b{};
    b.fType = type;
    b.fRect = SDL_Rect{mX0 + lx, mY0 + ly, w, h};
    b.fGuidesBothDirections = false;
    b.fO = eDiagonalOrientation::bottomLeft;
    if(type == eBuildingType::granary) {
        b.fAccept = eResourceType::food;
    } else if(type == eBuildingType::warehouse) {
        b.fAccept = eResourceType::warehouse;
    }
    return eAIDistrict::sBuild(mBoard, mPid, mCid, false, b);
}

void eTestCity::add(const int stage, const eBuildingType type,
                    const int lx, const int ly, const int w, const int h) {
    if(ly + h > mBlockH) return; // the block is too small for this row
    eItem i{};
    i.fStage = stage;
    i.fType = type;
    i.fLX = lx;
    i.fLY = ly;
    i.fW = w;
    i.fH = h;
    mItems.push_back(i);
}

void eTestCity::plan() {
    using eBT = eBuildingType;
    // above the first street
    add(0, eBT::maintenanceOffice, 1, 0, 2, 2);
    for(int i = 1; i < 11; i++) add(0, eBT::commonHouse, 1 + 2*i, 0, 2, 2);
    // below the first street; the agora takes its stretch of the street
    add(0, eBT::commonAgora, 1, 2, 6, 3);
    for(int i = 0; i < 3; i++) add(0, eBT::commonHouse, 7 + 2*i, 3, 2, 2);
    add(1, eBT::taxOffice, 13, 3, 2, 2);
    add(0, eBT::fountain, 15, 3, 2, 2);
    add(0, eBT::maintenanceOffice, 17, 3, 2, 2);
    add(0, eBT::watchPost, 19, 3, 2, 2);
    add(1, eBT::podium, 21, 3, 2, 2);
    // above the second street
    add(2, eBT::theater, 1, 6, 5, 5);
    add(2, eBT::dramaSchool, 6, 8, 3, 3);
    add(2, eBT::hospital, 9, 7, 4, 4);
    add(0, eBT::granary, 13, 7, 4, 4);
    add(1, eBT::warehouse, 17, 8, 3, 3);
    add(1, eBT::college, 20, 8, 3, 3);
    // below the second street
    // (a maintenance office in the middle: walkers from the other streets
    // do not reach this far)
    for(int i = 0; i < 11; i++) {
        const auto type = i == 5 ? eBT::maintenanceOffice : eBT::commonHouse;
        add(0, type, 1 + 2*i, 12, 2, 2);
    }
    // above the third street
    add(1, eBT::gymnasium, 1, 17, 3, 3);
    for(int i = 0; i < 9; i++) add(0, eBT::commonHouse, 4 + 2*i, 18, 2, 2);
    // below the third street
    for(int i = 0; i < 11; i++) {
        const auto type = i == 5 ? eBT::maintenanceOffice : eBT::commonHouse;
        add(0, type, 1 + 2*i, 21, 2, 2);
    }
    // above the fourth street
    for(int i = 0; i < 11; i++) add(0, eBT::commonHouse, 1 + 2*i, 27, 2, 2);
    // below the fourth street
    add(0, eBT::huntingLodge, 1, 30, 2, 2);
    add(0, eBT::dairy, 3, 30, 2, 2);
    add(0, eBT::cardingShed, 5, 30, 2, 2);
    add(1, eBT::timberMill, 7, 30, 2, 2);
    add(2, eBT::masonryShop, 9, 30, 2, 2);
    add(2, eBT::mint, 11, 30, 2, 2);
    add(1, eBT::warehouse, 13, 30, 3, 3);
    add(0, eBT::fountain, 16, 30, 2, 2);
    add(0, eBT::maintenanceOffice, 18, 30, 2, 2);
    add(0, eBT::watchPost, 20, 30, 2, 2);
}

// A road from the edge of a building to the nearest road there is, over
// free land (carts only travel on roads). False if there is no way.
bool eTestCity::roadTo(const int x, const int y, const int w, const int h) {
    using ePt = std::pair<int, int>;
    std::map<ePt, ePt> from;
    std::vector<ePt> queue;
    const ePt none{-100000, -100000};
    const auto visit = [&](const ePt& p, const ePt& parent) {
        if(from.count(p)) return;
        from[p] = parent;
        queue.push_back(p);
    };
    // the tiles along the four sides of the building
    for(int i = 0; i < w; i++) {
        visit({x + i, y - 1}, none);
        visit({x + i, y + h}, none);
    }
    for(int i = 0; i < h; i++) {
        visit({x - 1, y + i}, none);
        visit({x + w, y + i}, none);
    }
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};
    for(size_t i = 0; i < queue.size() && i < 20000; i++) {
        const auto p = queue[i];
        const auto t = mBoard.tile(p.first, p.second);
        if(!t) continue;
        if(t->hasRoad()) {
            // found: lay the road back to the building
            auto q = from[p];
            while(q != none) {
                place(eBuildingType::road, q.first - mX0, q.second - mY0, 1, 1);
                q = from[q];
                mFarmRoads++;
            }
            return true;
        }
        if(!free(p.first, p.second, 1, 1)) continue;
        for(int k = 0; k < 4; k++) {
            visit({p.first + dx[k], p.second + dy[k]}, p);
        }
    }
    return false;
}

void eTestCity::buildFarms() {
    using eBT = eBuildingType;
    // on the nearest meadow, wherever the map has it
    for(const auto farm : {eBT::wheatFarm, eBT::carrotsFarm, eBT::onionsFarm,
                           eBT::wheatFarm, eBT::carrotsFarm, eBT::onionsFarm}) {
        for(const auto t : mTiles) {
            const int x = t->x();
            const int y = t->y();
            if(inBlock(x, y, 3, 3)) continue;
            if(!free(x, y, 3, 3, true)) continue;
            if(!place(farm, x - mX0, y - mY0, 3, 3)) continue;
            mFarms++;
            roadTo(x, y, 3, 3);
            // on the list like the rest, so it is rebuilt if lost
            eItem i{};
            i.fStage = 0;
            i.fType = farm;
            i.fLX = x - mX0;
            i.fLY = y - mY0;
            i.fW = 3;
            i.fH = 3;
            i.fBuilt = true;
            mItems.push_back(i);
            break;
        }
    }
}

// A palace on the nearest free land outside the block (houses pay no tax
// without one), with a road to it. It takes a ring of tiles around it.
void eTestCity::buildPalace() {
    for(const auto t : mTiles) {
        const int x = t->x();
        const int y = t->y();
        if(inBlock(x, y, 10, 6)) continue;
        if(!free(x, y, 10, 6)) continue;
        if(!place(eBuildingType::palace, x + 1 - mX0, y + 1 - mY0, 8, 4)) {
            continue;
        }
        mPalace = true;
        // on the list like the rest, so it is rebuilt if lost
        eItem i{};
        i.fStage = 0;
        i.fType = eBuildingType::palace;
        i.fLX = x + 1 - mX0;
        i.fLY = y + 1 - mY0;
        i.fW = 8;
        i.fH = 4;
        i.fBuilt = true;
        mItems.push_back(i);
        roadTo(x, y, 10, 6);
        return;
    }
}

void eTestCity::buildHerd(const eBuildingType type, int& placed) {
    for(const auto t : mTiles) {
        if(placed >= 6) break;
        const int x = t->x();
        const int y = t->y();
        if(inBlock(x, y, 1, 1)) continue;
        if(t->terrain() != eTerrain::fertile) continue;
        if(t->underBuilding()) continue;
        if(!t->characters().empty()) continue;
        if(place(type, x - mX0, y - mY0, 1, 1)) placed++;
    }
}

void eTestCity::start() {
    mStarted = true;
    mPid = mBoard.personPlayer();
    // the city of the player that is on this map (in a colony episode it
    // is not the capital)
    const auto mine = mBoard.personPlayerCitiesOnBoard();
    mCid = mine.empty() ? mBoard.personPlayerCapital() : mine[0];
    const auto city = mBoard.boardCityWithId(mCid);
    if(!city) return;

    // the land of the city, nearest to where settlers arrive first
    mTiles = city->tiles();
    const auto entry = mBoard.entryPoint(mCid);
    sortTiles(entry ? entry->x() : 0, entry ? entry->y() : 0);

    // A free, flat block of land: room for four streets if the map has
    // it, otherwise for three or two.
    for(const int h : {gBlockH4, gBlockH3, gBlockH2}) {
        for(const auto t : mTiles) {
            if(!free(t->x(), t->y(), gBlockW, h)) continue;
            mSite = true;
            mX0 = t->x();
            mY0 = t->y();
            break;
        }
        if(!mSite) continue;
        mBlockH = h;
        mStreets = h == gBlockH4 ? 4 : (h == gBlockH3 ? 3 : 2);
        break;
    }
    if(!mSite) return;

    // the streets (y = 2, 11, 20 and 29), joined at both ends
    const int lastStreet = 2 + 9*(mStreets - 1);
    for(int y = 2; y <= lastStreet; y++) {
        const bool street = (y - 2) % 9 == 0;
        for(int x = 0; x < gBlockW; x++) {
            if(!street && x != 0 && x != gBlockW - 1) continue;
            place(eBuildingType::road, x, y, 1, 1);
        }
    }
    plan();

    sortTiles(mX0 + gBlockW/2, mY0 + mBlockH/2);
    buildFarms();
    buildPalace();
    buildHerd(eBuildingType::sheep, mSheep);
    buildHerd(eBuildingType::goat, mGoats);

    // enough money that the city is never lost to debt during the run
    mBoard.setDrachmas(mPid, 1000000);
}

void eTestCity::update(const int day) {
    if(!mStarted) start();
    if(!mSite) return;
    for(auto& i : mItems) {
        if(i.fBuilt) {
            // burned down or collapsed since? build it again, as a
            // player would
            const bool agora = i.fType == eBuildingType::commonAgora;
            const auto t = mBoard.tile(mX0 + i.fLX,
                                       mY0 + i.fLY + (agora ? 1 : 0));
            const auto ub = t ? t->underBuilding() : nullptr;
            if(ub && ub->type() != eBuildingType::ruins) continue;
            i.fBuilt = false;
            i.fTries = 0;
            mRebuilt++;
        }
        if(i.fTries >= gBuildTries) continue;
        if(day < gStageDays[i.fStage]) continue;
        i.fTries++;
        i.fBuilt = place(i.fType, i.fLX, i.fLY, i.fW, i.fH);
    }
}

std::string eTestCity::report() const {
    std::ostringstream o;
    if(!mSite) {
        int freeTiles = 0;
        for(const auto t : mTiles) {
            if(free(t->x(), t->y(), 1, 1)) freeTiles++;
        }
        o << "test city: no free block of land found (city "
          << static_cast<int>(mCid) << " has " << mTiles.size()
          << " tiles, " << freeTiles << " of them free)\n";
        return o.str();
    }
    int built = 0;
    for(const auto& i : mItems) {
        if(i.fBuilt) built++;
    }
    o << "test city: at " << mX0 << "," << mY0
      << ", streets " << mStreets
      << ", built " << built << " of " << mItems.size()
      << ", farms " << mFarms
      << " (road tiles to them and the palace: " << mFarmRoads << ")"
      << ", palace " << (mPalace ? "yes" : "no")
      << ", sheep " << mSheep
      << ", goats " << mGoats
      << ", lost and built again " << mRebuilt << "\n";
    for(const auto& i : mItems) {
        if(i.fBuilt) continue;
        o << "  not built: " << eBuilding::sNameForBuilding(i.fType)
          << " at " << i.fLX << "," << i.fLY
          << (i.fTries == 0 ? " (its turn never came)" : "") << "\n";
    }
    return o.str();
}

// ---- what the city looks like ----

// What is really in the storage buildings. (eBoardCity::resourceCount is
// a figure the city only refreshes now and then.)
int stored(const eBoardCity& c, const eResourceType type) {
    int count = 0;
    for(const auto b : c.allBuildings()) {
        const auto s = dynamic_cast<const eStorageBuilding*>(b);
        if(s) count += s->count(type);
    }
    return count;
}

int foodStored(const eBoardCity& c) {
    int food = 0;
    for(int i = 0; i < 8; i++) {
        food += stored(c, static_cast<eResourceType>(1 << i));
    }
    return food;
}

// one line for the year-by-year list
std::string yearLine(eGameBoard& board) {
    std::ostringstream o;
    const auto mine = board.personPlayerCitiesOnBoard();
    const auto c = mine.empty() ? nullptr : board.boardCityWithId(mine[0]);
    o << "year " << board.date().year() << ":";
    if(c) {
        auto& empl = c->employmentData();
        o << " people " << c->population()
          << ", workers " << empl.employable()
          << ", jobs " << empl.totalJobVacancies()
          << ", food " << foodStored(*c)
          << ", buildings " << c->allBuildings().size() << ",";
    }
    o << " walkers " << board.characters().size()
      << ", drachmas " << board.drachmas(board.personPlayer()) << "\n";
    return o.str();
}

std::string summary(eGameBoard& board) {
    std::ostringstream o;
    const auto& date = board.date();
    o << "date: " << date.day() << "/"
      << (static_cast<int>(date.month()) + 1) << "/" << date.year() << "\n";
    o << "total ticks: " << board.totalTime() << "\n";
    o << "episode lost: " << (board.episodeLost() ? "yes" : "no") << "\n";
    const auto ppid = board.personPlayer();
    o << "drachmas: " << board.drachmas(ppid) << "\n";

    const auto cids = board.citiesOnBoard();
    for(const auto cid : cids) {
        const auto c = board.boardCityWithId(cid);
        if(!c) continue;
        const bool mine = board.cityIdToPlayerId(cid) == ppid;
        o << "\n[city " << static_cast<int>(cid)
          << (mine ? " (the player's)" : "") << "]\n";
        o << "population: " << c->population() << "\n";
        o << "popularity: " << c->popularity() << "\n";
        o << "hygiene: " << c->health() << "\n";
        o << "unrest: " << c->unrest() << "\n";
        o << "mood: " << c->mood() << "\n";
        o << "maintenance: " << c->maintanance() << "\n";
        o << "taxes paid this year: " << c->taxesPaidThisYear() << "\n";
        o << "taxes paid last year: " << c->taxesPaidLastYear() << "\n";
        o << "culture coverage: "
          << c->philosophyResearchCoverage() << " "
          << c->athleticsLearningCoverage() << " "
          << c->dramaAstronomyCoverage() << " "
          << c->allCultureScienceCoverage() << "\n";
        auto& empl = c->employmentData();
        o << "workers: " << empl.employable()
          << ", jobs: " << empl.totalJobVacancies()
          << ", unemployed: " << empl.unemployed() << "\n";

        std::map<int, int> buildings;
        for(const auto b : c->allBuildings()) {
            buildings[static_cast<int>(b->type())]++;
        }
        o << "buildings:\n";
        for(const auto& b : buildings) {
            const auto type = static_cast<eBuildingType>(b.first);
            o << "  " << b.first << " "
              << eBuilding::sNameForBuilding(type) << ": " << b.second << "\n";
        }

        o << "stored goods:\n";
        for(int i = 0; i < 24; i++) {
            const auto type = static_cast<eResourceType>(1 << i);
            if(type == eResourceType::drachmas) continue;
            const int count = stored(*c, type);
            if(count == 0) continue;
            o << "  " << eResourceTypeHelpers::typeName(type)
              << ": " << count << "\n";
        }
    }

    std::map<int, int> chars;
    for(const auto c : board.characters()) {
        chars[static_cast<int>(c->type())]++;
    }
    o << "\n[walkers by type number]\n";
    int total = 0;
    for(const auto& c : chars) {
        o << "  " << c.first << ": " << c.second << "\n";
        total += c.second;
    }
    o << "walkers in all: " << total << "\n";

    // which city they belong to, and the ones that have been out for more
    // than a year (a walker that never gets where it is going shows here)
    std::map<int, int> byCity;
    std::map<int, int> old;
    for(const auto c : board.characters()) {
        byCity[static_cast<int>(c->cityId())]++;
        if(c->time() > gDaysPerYear*gTicksPerDay) {
            old[static_cast<int>(c->type())]++;
        }
    }
    o << "[walkers by city]\n";
    for(const auto& c : byCity) {
        o << "  " << c.first << ": " << c.second << "\n";
    }
    o << "[walkers out for more than a year, by type number]\n";
    for(const auto& c : old) {
        o << "  " << c.first << ": " << c.second << "\n";
    }
    return o.str();
}

std::string eventsSummary(const eCounters& counters) {
    std::ostringstream o;
    int total = 0;
    for(const auto& e : counters.fEvents) {
        o << "  " << e.first << ": " << e.second << "\n";
        total += e.second;
    }
    o << "events in all: " << total << "\n";
    o << "other messages: " << counters.fMessages << "\n";
    o << "times all goals were met: " << counters.fGoalsFulfilled << "\n";
    o << "[events as they happened]\n" << counters.fLog;
    return o.str();
}

// Runs the city for a number of days. The test city (if any) is looked
// after as it goes, and a line is noted down each time a year ends.
void run(eGameBoard& board, const int days, eTestCity* const city,
         std::string* const years,
         const std::function<void(int)>& everyChunk = nullptr,
         const std::function<void(int, int)>& everyStep = nullptr) {
    const int stepsPerChunk = gChunkDays*gTicksPerDay/gTicksPerStep;
    int lastYear = board.date().year();
    long long steps = 0;
    for(int day = 0; day < days; day += gChunkDays) {
        if(everyChunk) everyChunk(day);
        if(city) city->update(day);
        for(int i = 0; i < stepsPerChunk; i++, steps++) {
            // at 100% speed the game does 5 steps for each frame it draws
            if(steps % 5 == 0) board.incFrame();
            step(board);
            if(everyStep) everyStep(day, i);
        }
        const int year = board.date().year();
        if(year == lastYear) continue;
        lastYear = year;
        if(!years) continue;
        *years += yearLine(board);
        std::printf("  ... year %d\n", year);
        std::fflush(stdout);
    }
}

// ---- saving and loading, as eMainWindow does it ----

bool saveCampaign(eCampaign& c, const std::string& path) {
    c.waitUntilFinished();
    std::ofstream file(std::filesystem::u8path(path),
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if(!file) return false;
    eWriteTarget target(&file);
    eWriteStream dst(target);
    dst.writeFormat("eZeus.ez");
    eGameWidgetSettings s;
    s.fPaused = true;
    s.write(dst);
    c.write(dst);
    file.flush();
    return static_cast<bool>(file);
}

stdsptr<eCampaign> loadCampaign(const std::string& path) {
    std::ifstream file(std::filesystem::u8path(path),
                       std::ios::in | std::ios::binary);
    if(!file) return nullptr;
    eReadSource source(&file);
    eReadStream src(source);
    src.readFormat();
    if(src.format() != "eZeus.ez") return nullptr;
    eGameWidgetSettings s;
    s.read(src);
    const auto c = std::make_shared<eCampaign>();
    c->read(src);
    c->loadStrings();
    c->loadNumbers();
    src.handlePostFuncs();
    return c;
}

// fingerprint of a file (FNV-1a, 64 bit)
bool fileFingerprint(const std::string& path,
                     std::string& fingerprint, long long& size) {
    std::ifstream file(std::filesystem::u8path(path),
                       std::ios::in | std::ios::binary);
    if(!file) return false;
    uint64_t h = 14695981039346656037ull;
    size = 0;
    char buf[65536];
    while(file) {
        file.read(buf, sizeof(buf));
        const auto n = file.gcount();
        for(std::streamsize i = 0; i < n; i++) {
            h ^= static_cast<unsigned char>(buf[i]);
            h *= 1099511628211ull;
        }
        size += n;
    }
    char out[32];
    std::snprintf(out, sizeof(out), "%016llx",
                  static_cast<unsigned long long>(h));
    fingerprint = out;
    return true;
}

stdsptr<eCampaign> openAdventure(const std::string& name) {
    namespace fs = std::filesystem;
    const auto c = std::make_shared<eCampaign>();
    const auto own = eGameDir::adventuresDir() + name + "/" + name + ".epak";
    if(fs::exists(fs::u8path(own))) {
        if(!c->load(name)) return nullptr;
        return c;
    }
    auto pak = eGameDir::pakAdventuresDir() + name;
    if(pak.size() < 4 || pak.substr(pak.size() - 4) != ".pak") pak += ".pak";
    if(fs::exists(fs::u8path(pak))) {
        c->readPak(name, pak);
        return c;
    }
    return nullptr;
}

// --building-table: makes one building of every kind in the table of
// building kinds (buildings/ebuildingregistry.cpp) and checks that the
// table says the truth about it: its type and its footprint. Prints the
// table as it should read. Returns 0 when every line is right.
int checkBuildingTable(eGameBoard& board) {
    const auto mine = board.personPlayerCitiesOnBoard();
    if(mine.empty()) {
        std::printf("No city of the player on this map.\n");
        return 1;
    }
    const auto cid = mine[0];
    int wrong = 0;
    for(const auto& k : eBuildingRegistry::sAll()) {
        const auto b = k.fCreate(board, cid);
        if(!b) {
            std::printf("kind %d: nothing was made\n",
                        static_cast<int>(k.fType));
            wrong++;
            continue;
        }
        const bool ok = b->type() == k.fType &&
                        b->spanW() == k.fW && b->spanH() == k.fH;
        if(!ok) wrong++;
        std::printf("kind %d %s: table says type %d, %d x %d; "
                    "the building is type %d, %d x %d%s\n",
                    static_cast<int>(k.fType),
                    eBuilding::sNameForBuilding(k.fType).c_str(),
                    static_cast<int>(k.fType), k.fW, k.fH,
                    static_cast<int>(b->type()), b->spanW(), b->spanH(),
                    ok ? "" : "   <-- WRONG");
        b->erase();
    }
    board.emptyRubbish();
    std::printf("%d of %d lines are wrong.\n", wrong,
                static_cast<int>(eBuildingRegistry::sAll().size()));
    std::fflush(stdout);
    return wrong == 0 ? 0 : 1;
}

} // namespace

bool eHeadlessCheckRequested(const int argc, char** const argv) {
    for(int i = 1; i < argc; i++) {
        if(std::strcmp(argv[i], "--headless-check") == 0) return true;
    }
    return false;
}

int eHeadlessCheckRun(const int argc, char** const argv) {
    eArgs args;
    if(!parseArgs(argc, argv, args)) {
        std::printf("Usage: --headless-check <adventure> <years> "
                    "[--seed N] [--out file] [--no-city] "
                    "[--reload-days N] [--keep-saves] [--threads]\n");
        return 2;
    }
    using std::chrono::steady_clock;
    const auto t0 = steady_clock::now();

    eThreadPool::sSynchronous = !args.fThreads;
    if(!args.fRandDiffA.empty()) {
        return eHeadlessRandDiff(args.fRandDiffA.c_str(),
                                 args.fRandDiffB.c_str());
    }
    std::FILE* randLog = nullptr;
    if(!args.fRandLog.empty()) {
        randLog = std::fopen(args.fRandLog.c_str(), "wb");
        if(randLog) eRand::setLog(randLog, eHeadlessProgramStart());
    }
    eRand::seed(args.fSeed);

    eGameDir::initialize();
    eNumbers::sLoad();

    // No window: no picture is ever loaded or drawn. With every texture
    // size switched off the game's "load this picture" calls do nothing.
    eSettings settings;
    settings.fTinyTextures = false;
    settings.fSmallTextures = false;
    settings.fMediumTextures = false;
    settings.fLargeTextures = false;
    eGameTextures::setSettings(settings);
    if(!eGameTextures::initialize(nullptr)) return 1;

    // never loaded, so nothing is ever played
    eMusic music;
    eSounds sounds;

    eLanguage::load();
    eSanctBlueprints::load();
    eMessages::load();

    auto c = openAdventure(args.fAdventure);
    if(!c) {
        std::printf("Could not open adventure '%s'.\n",
                    args.fAdventure.c_str());
        return 1;
    }
    c->setDifficulty(eDifficulty::hero); // the game's default
    c->startEpisode();
    auto board = c->currentEpisode()->fBoard;
    if(!board) {
        std::printf("The adventure has no city map.\n");
        return 1;
    }
    board->updateAppealMapIfNeeded();
    board->waitUntilFinished();

    if(args.fBuildingTable) return checkBuildingTable(*board);

    eCounters counters;
    connect(*board, counters);

    std::printf("Running '%s' for %d year(s), seed %u...\n",
                args.fAdventure.c_str(), args.fYears, args.fSeed);
    std::fflush(stdout);

    std::string years;
    std::string cityReport = "test city: not built (--no-city)\n";
    {
        eTestCity city(*board);
        // For hunting down why two runs differ: every so many days, print
        // the fingerprint of a save made right then. The first day on which
        // two runs print different fingerprints is where they part ways.
        std::function<void(int)> trace;
        if(args.fTrace > 0) {
            trace = [&](const int day) {
                if(day % args.fTrace >= gChunkDays) return;
                const std::string base =
                    args.fOut.empty() ? "headless-check" : args.fOut;
                const auto path = base + ".day" + std::to_string(day) + ".ez";
                if(!saveCampaign(*c, path)) return;
                std::string fp;
                long long size = 0;
                fileFingerprint(path, fp, size);
                std::printf("trace day %d: %s %lld\n", day, fp.c_str(), size);
                std::fflush(stdout);
                const bool keep = day >= args.fTraceKeepFrom &&
                                  day <= args.fTraceKeepTo;
                if(keep) return;
                std::error_code ec;
                std::filesystem::remove(std::filesystem::u8path(path), ec);
            };
        }
        // the same after every single step, between two given days (the
        // saves are kept)
        std::function<void(int, int)> traceSteps;
        if(args.fTraceStepsTo >= args.fTraceStepsFrom) {
            traceSteps = [&](const int day, const int i) {
                if(day < args.fTraceStepsFrom) return;
                if(day > args.fTraceStepsTo) return;
                const std::string base =
                    args.fOut.empty() ? "headless-check" : args.fOut;
                const auto path = base + ".day" + std::to_string(day) +
                                  "s" + std::to_string(i) + ".ez";
                if(!saveCampaign(*c, path)) return;
                std::string fp;
                long long size = 0;
                fileFingerprint(path, fp, size);
                std::printf("trace day %d step %d: %s %lld\n",
                            day, i, fp.c_str(), size);
                std::fflush(stdout);
            };
        }
        run(*board, args.fYears*gDaysPerYear,
            args.fCity ? &city : nullptr, &years, trace, traceSteps);
        if(args.fCity) cityReport = city.report();
    }

    std::ostringstream o;
    o << "[run]\n";
    o << "adventure: " << args.fAdventure << "\n";
    o << "years: " << args.fYears << "\n";
    o << "seed: " << args.fSeed << "\n";
    o << "threads: " << (args.fThreads ? "yes" : "no") << "\n";
    o << "save format version: " << eFileFormat::version << "\n";
    {
        // the game's tunable numbers shape the result; if this line
        // changed, numbers.txt in the game folder did
        std::string fp = "missing";
        long long size = 0;
        fileFingerprint(eGameDir::numbersPath(), fp, size);
        o << "numbers.txt fingerprint: " << fp << "\n";
    }
    o << cityReport;
    o << "\n[year by year, on 1 January]\n" << years;

    const auto after = summary(*board);
    o << "\n===== after " << args.fYears << " year(s) =====\n";
    o << after;
    o << "\n[events by event number]\n";
    o << eventsSummary(counters);

    // Save, load the save again, and look at the city once more. Then let
    // the loaded city run on, to notice anything a save forgets.
    namespace fs = std::filesystem;
    std::string saveBase = args.fOut.empty() ? "headless-check" : args.fOut;
    const auto save1 = saveBase + ".save1.ez";
    const auto save2 = saveBase + ".save2.ez";
    o << "\n===== save and reload =====\n";
    if(!args.fSaveWho.empty()) {
        eHeadlessSaveWatchSet(args.fSaveWho.data(),
                              static_cast<int>(args.fSaveWho.size()));
        eWriteTarget::sWatch = eHeadlessSaveWatch;
    }
    const bool saved1 = saveCampaign(*c, save1);
    eWriteTarget::sWatch = nullptr;
    if(!saved1) {
        o << "save: FAILED\n";
    } else {
        std::string fp1;
        long long size1 = 0;
        fileFingerprint(save1, fp1, size1);
        o << "save fingerprint: " << fp1 << " (" << size1 << " bytes)\n";

        board = nullptr;
        c = nullptr; // the first city is gone before the copy is loaded
        c = loadCampaign(save1);
        if(!c || !c->currentEpisode() || !c->currentEpisode()->fBoard) {
            o << "reload: FAILED\n";
        } else {
            board = c->currentEpisode()->fBoard;
            board->updateAppealMapIfNeeded();
            board->waitUntilFinished();
            eCounters counters2;
            connect(*board, counters2);

            const auto reloaded = summary(*board);
            if(reloaded == after) {
                o << "city after reload: SAME as before saving\n";
            } else {
                o << "city after reload: DIFFERENT from before saving\n";
                o << reloaded;
            }

            if(!args.fSave2Who.empty()) {
                eHeadlessSaveWatchSet(args.fSave2Who.data(),
                                      static_cast<int>(args.fSave2Who.size()));
                eWriteTarget::sWatch = eHeadlessSaveWatch;
            }
            const bool saved2 = saveCampaign(*c, save2);
            eWriteTarget::sWatch = nullptr;
            if(saved2) {
                std::string fp2;
                long long size2 = 0;
                fileFingerprint(save2, fp2, size2);
                o << "saved again right away: "
                  << (fp2 == fp1 ? "SAME file" : "DIFFERENT file")
                  << " (" << fp2 << ", " << size2 << " bytes)\n";
            } else {
                o << "saved again right away: FAILED\n";
            }

            if(args.fReloadDays > 0) {
                eRand::seed(args.fSeed + 1);
                run(*board, args.fReloadDays, nullptr, nullptr);
                o << "\n===== reloaded city, " << args.fReloadDays
                  << " days later =====\n";
                o << summary(*board);
                o << "\n[events by event number]\n";
                o << eventsSummary(counters2);
            }
        }
    }
    if(!args.fKeepSaves) {
        std::error_code ec;
        fs::remove(fs::u8path(save1), ec);
        fs::remove(fs::u8path(save2), ec);
    }

    const auto text = o.str();
    if(args.fOut.empty()) {
        std::printf("%s", text.c_str());
    } else {
        std::ofstream file(fs::u8path(args.fOut),
                           std::ios::out | std::ios::binary | std::ios::trunc);
        file << text;
        if(!file) {
            std::printf("Could not write '%s'.\n", args.fOut.c_str());
            return 1;
        }
    }
    if(randLog) {
        eRand::setLog(nullptr, nullptr);
        std::fclose(randLog);
    }
    const std::chrono::duration<double> el = steady_clock::now() - t0;
    std::printf("Done in %.1f s.\n", el.count());
    std::fflush(stdout);

    board = nullptr;
    c = nullptr;
    return 0;
}
