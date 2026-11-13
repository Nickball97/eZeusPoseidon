#include "egamewidget.h"

#include "buildings/allbuildings.h"

#include "evectorhelpers.h"
#include "infowidgets/eagorainfowidget.h"
#include "infowidgets/echaracterinfowidget.h"
#include "infowidgets/ecommonhouseinfowidget.h"
#include "infowidgets/eheroshallinfowidget.h"
#include "infowidgets/ehippodromeinfowidget.h"
#include "infowidgets/esanctuaryinfowidget.h"
#include "infowidgets/estorageinfowidget.h"
#include "infowidgets/etradepostinfowidget.h"
#include "infowidgets/etriremewharfinfowidget.h"

#include "engine/egameboard.h"
#include "engine/edate.h"
#include "buildings/efarmbase.h"
#include "buildings/eprocessingbuilding.h"

#include <cmath>

eInfoWidget* eGameWidget::openInfoWidget(eBuilding* const b) {
    eInfoWidget* wid = nullptr;
    eAction closeAct;
    if(const auto house = dynamic_cast<eHouseBase*>(b)) {
        const auto w = new eCommonHouseInfoWidget(window(), this);
        w->initialize(house);
        wid = w;
    } else if(const auto stor = dynamic_cast<eTradePost*>(b)) {
        const auto storWid = new eTradePostInfoWidget(window(), this);
        const auto prevNext = [this, storWid, stor, closeAct](const bool next) {
            const auto pid = stor->playerId();
            const auto& board = stor->getBoard();
            std::vector<eTradePost*> posts;
            const auto cities = board.playerCitiesOnBoard(pid);
            for(const auto cid : cities) {
                const auto cboard = board.boardCityWithId(cid);
                const auto& cposts = cboard->tradePosts();
                for(const auto p : cposts) {
                    posts.emplace_back(p);
                }
            }
            if(posts.size() <= 1) return;
            int id = eVectorHelpers::index(posts, stor);
            if(next) {
                id++;
            } else {
                id--;
            }
            if(id < 0) id = posts.size() - 1;
            else if(id >= int(posts.size())) id = 0;
            {
                if(closeAct) closeAct();
                mInfoWidget = nullptr;
                removeWidget(storWid);
                storWid->deleteLater();
            }
            const auto post = posts[id];
            mInfoWidget = openInfoWidget(post);
            const auto tile = post->centerTile();
            viewTile(tile);
        };
        storWid->initialize(stor, prevNext);
        wid = storWid;
    } else if(const auto stor = dynamic_cast<eStorageBuilding*>(b)) {
        const auto storWid = new eStorageInfoWidget(window(), this);
        storWid->initialize(stor);
        wid = storWid;
    } else if(const auto hp = dynamic_cast<eHippodromePiece*>(b)) {
        const auto h = hp->hippodrome();
        if(h) {
            const auto hWid = new eHippodromeInfoWidget(window(), this, true, true);
            hWid->initialize(h);
            wid = hWid;
        }
    } else if(const auto hh = dynamic_cast<eHerosHall*>(b)) {
        const auto hhWid = new eHerosHallInfoWidget(window(), this, false, false);
        hhWid->initialize(hh);
        wid = hhWid;
    } else if(const auto tw = dynamic_cast<eTriremeWharf*>(b)) {
        const auto twWid = new eTriremeWharfInfoWidget(window(), this, false, false);
        twWid->initialize(tw);
        wid = twWid;
    } else if(const auto encl = dynamic_cast<eHorseRanchEnclosure*>(b)) {
        return openInfoWidget(encl->ranch());
    } else {
        eAgoraBase* a = nullptr;
        if(const auto aa = dynamic_cast<eAgoraBase*>(b)) {
            a = aa;
        } else if(const auto v = dynamic_cast<eVendor*>(b)) {
            a = v->agora();
        } else if(const auto v = dynamic_cast<eAgoraSpace*>(b)) {
            a = v->agora();
        } else if(const auto v = dynamic_cast<eRoad*>(b)) {
            a = v->underAgora();
        }
        if(a) {
            const auto aWid = new eAgoraInfoWidget(window(), this);
            aWid->initialize(a);
            wid = aWid;
        } else if(const auto s = dynamic_cast<eMonument*>(b)) {
            const auto sWid = new eSanctuaryInfoWidget(window(), this);
            sWid->initialize(s);
            wid = sWid;
        } else if(const auto sb = dynamic_cast<eSanctBuilding*>(b)) {
            return openInfoWidget(sb->monument());
        } else if(const auto eb = dynamic_cast<eEmployingBuilding*>(b)) {
            const auto ebWid = new eEmployingBuildingInfoWidget(
                                    window(), this, true, true);
            std::string title;
            std::string info;
            std::string employmentInfo;
            std::string additionalInfo;
            int group = 0;
            eBuilding::sInfoText(eb, title, info,
                                 employmentInfo,
                                 additionalInfo, &group);
            // As in the original: "Production is 75% complete." and
            // "Stored olives: 2 Loads" above the staffing line, and a
            // farm's "The next wheat harvest is in July" below the workers
            // panel. Texts from the building's own group (2, 3, 12, 14).
            std::vector<std::string> top;
            std::string harvest;
            const auto farm = dynamic_cast<eFarmBase*>(eb);
            const auto proc = dynamic_cast<eProcessingBuilding*>(eb);
            const auto prodIs = eLanguage::zeusText(group, 2);
            if((farm || proc) && prodIs == "Production is") {
                const double f = farm ? farm->growth() : proc->progress();
                const int pct = static_cast<int>(std::round(100*f));
                top.push_back(prodIs + " " + std::to_string(pct) + "% " +
                              eLanguage::zeusText(group, 3));
            }
            if(proc) {
                const auto stored = eLanguage::zeusText(group, 12);
                if(stored.rfind("Stored", 0) == 0) {
                    const int n = proc->rawCount();
                    top.push_back(stored + " " + std::to_string(n) + " " +
                                  eLanguage::zeusText(8, n == 1 ? 10 : 11));
                }
            }
            if(farm) {
                const auto next = eLanguage::zeusText(group, 14);
                if(!next.empty()) {
                    harvest = next + " " +
                              eMonthHelper::name(farm->harvestMonth());
                }
            }
            ebWid->eInfoWidget::initialize(title);
            for(const auto& t : top) ebWid->addText(t);
            if(!employmentInfo.empty()) ebWid->addText(employmentInfo);
            ebWid->addEmploymentWidget(eb);
            // towers show their Atlantean Fire status underneath
            const bool tower = eb->type() == eBuildingType::tower;
            if(tower && !additionalInfo.empty()) {
                ebWid->addText(additionalInfo);
            }
            if(!harvest.empty()) ebWid->addText(harvest);
            if(!info.empty()) ebWid->addText(info);
            wid = ebWid;
        } else if(const auto p = dynamic_cast<ePalace*>(b)) {
            const auto pWid = new eInfoWidget(window(), this, true, true);
            std::string title;
            std::string info;
            std::string employmentInfo;
            std::string additionalInfo;
            eBuilding::sInfoText(p, title, info,
                                 employmentInfo,
                                 additionalInfo);
            pWid->initialize(title);
            pWid->addText(info);
            if(!employmentInfo.empty()) pWid->addText(employmentInfo);
            wid = pWid;
        } else if(const auto r = dynamic_cast<eRuins*>(b)) {
            const auto rWid = new eInfoWidget(window(), this, true, true);
            std::string title;
            std::string info;
            std::string employmentInfo;
            std::string additionalInfo;
            eBuilding::sInfoText(r, title, info,
                                 employmentInfo,
                                 additionalInfo);
            rWid->initialize(title);

            const auto wtype = r->wasType();
            if(wtype != eBuildingType::none) {
                const auto name = eBuilding::sNameForBuilding(wtype);
                rWid->addText(name);
            }
            rWid->addText(info);
            wid = rWid;
        } else {
            const auto bWid = new eInfoWidget(window(), this, true, true);
            if(const auto r = dynamic_cast<eRoad*>(b)) {
                if(r->isRoadblock()) {
                    bWid->initialize(eLanguage::zeusText(155, 0));
                    bWid->addText(eLanguage::zeusText(155, 1));
                    wid = bWid;
                }
            }
            if(!wid) {
                const auto title = eBuilding::sNameForBuilding(b);
                bWid->initialize(title);
                // the building's description, as the original shows for
                // walls, parks and the like
                std::string t;
                std::string info;
                std::string ei;
                std::string ai;
                int group = 0;
                eBuilding::sInfoText(b, t, info, ei, ai, &group);
                if(group > 0 && !info.empty()) bWid->addText(info);
                wid = bWid;
            }
        }
    }
    if(wid) {
        wid->fitHeightToContent();
        addWidget(wid);
        wid->setX((width() - mGm->width() - wid->width())/2);
        wid->align(eAlignment::vcenter);
        wid->setCloseAction([this, wid, closeAct]() {
            if(closeAct) closeAct();
            mInfoWidget = nullptr;
            removeWidget(wid);
            wid->deleteLater();
        });
    }
    return wid;
}

eInfoWidget *eGameWidget::openInfoWidget(const std::vector<eCharacter*> chars) {
    const auto wid = new eCharacterInfoWidget(window(), this);
    wid->initialize(chars);
    if(wid) {
        wid->fitHeightToContent();
        addWidget(wid);
        wid->setX((width() - mGm->width() - wid->width())/2);
        wid->align(eAlignment::vcenter);
        wid->setCloseAction([this, wid]() {
            mInfoWidget = nullptr;
            removeWidget(wid);
            wid->deleteLater();
        });
    }
    return wid;
}
