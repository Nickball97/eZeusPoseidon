#include "estorageinfowidget.h"

#include "engine/egameboard.h"
#include "engine/eemploymentdistributor.h"
#include "engine/eworldcity.h"
#include "engine/eworldboard.h"

#include "elanguage.h"

#include <algorithm>
#include <cmath>

// Laid out like the original's storehouse and granary windows (Nicolás's
// screenshots of Zeus at 1x): a header row "Stock / Orders / Stock Limit"
// over one row per good with its icon, amount, name, order (clickable
// text), limit and down/up arrows. Texts are the original's, group 130.

namespace {

// a thin dark line, as under the original's header rows
class eStorageLine : public eWidget {
public:
    using eWidget::eWidget;
protected:
    void paintEvent(ePainter& p) override {
        p.fillRect(rect(), SDL_Color{0, 0, 0, 90});
    }
};

// The original lists only the goods the adventure uses (the granary of a
// city with fish, cheese and wheat shows just those three). eZeus has no
// such list, so a good is shown when the city can build something that
// makes it, a city on the world map sells it, or some is stored here.
bool goodUsed(eStorageBuilding* const stor, const eResourceType type) {
    if(stor->count(type) > 0) return true;
    auto& board = stor->getBoard();
    if(board.editorMode()) return true;
    const auto cid = stor->cityId();
    for(const auto bt : eIndustryHelpers::sBuildings(type)) {
        if(board.availableBuilding(cid, bt)) return true;
    }
    for(const auto& c : board.world().cities()) {
        if(!c || !c->active()) continue;
        if(c->sells(type)) return true;
    }
    return false;
}

void storedLoads(eStorageBuilding* const stor, int& stored, int& free) {
    stored = 0;
    free = 0;
    const int n = stor->spaceCount();
    for(int i = 0; i < n; i++) {
        const int c = stor->resourceCount(i);
        const auto t = stor->resourceType(i);
        if(c <= 0) {
            free += 4;
        } else if(t == eResourceType::sculpture) {
            stored += c; // a space holds one sculpture
        } else {
            stored += c;
            free += std::max(0, 4 - c);
        }
    }
}

}

class eResourceStorageStack : public eWidget {
public:
    using eWidget::eWidget;

    void initialize(eStorageBuilding* const stor,
                    const std::vector<eResourceType>& types,
                    const eResourceType get,
                    const eResourceType empty,
                    const eResourceType accept,
                    std::map<eResourceType, eSwitchButton*>& buttons,
                    std::map<eResourceType, eSpinBox*>& spinBoxes,
                    const std::map<eResourceType, int>& maxCount,
                    const eAction& changed,
                    const int width) {
        setNoPadding();
        auto& board = stor->getBoard();

        const auto res = resolution();
        const double m = res.multiplier();
        const int xCount = std::round(26*m);
        const int xName = std::round(56*m);
        const int xOrder = std::round(250*m);
        const int rowHeight = std::round(19*m);
        const int lineH = std::max(1, static_cast<int>(std::round(m)));

        int y = 0;

        const auto addLine = [&]() {
            const auto l = new eStorageLine(window());
            l->resize(width, lineH);
            addWidget(l);
            l->move(0, y);
            y += lineH + std::round(2*m);
        };
        const auto smallLabel = [&](const std::string& text) {
            const auto l = new eLabel(window());
            l->setNoPadding();
            l->setSmallFontSize();
            l->setText(text);
            l->fitContent();
            addWidget(l);
            return l;
        };

        // "Storing 8 loads.  Space for 24 loads." (storehouse only)
        if(stor->type() == eBuildingType::warehouse) {
            int stored;
            int free;
            storedLoads(stor, stored, free);
            const auto loads = eLanguage::zeusText(8, 17); // loads.
            const auto s = smallLabel(eLanguage::zeusText(130, 5) + " " +
                                      std::to_string(stored) + " " + loads);
            s->move(0, y);
            const auto f = smallLabel(eLanguage::zeusText(130, 6) + " " +
                                      std::to_string(free) + " " + loads);
            f->move(width - f->width(), y);
            y += std::max(s->height(), f->height());
            addLine();
        }

        {   // header: Stock, Orders, Stock Limit
            const auto stock = smallLabel(eLanguage::zeusText(130, 8));
            stock->move(0, y);
            const auto orders = smallLabel(eLanguage::zeusText(130, 9));
            orders->move(xOrder, y);
            const auto limit = smallLabel(eLanguage::zeusText(130, 10));
            limit->move(width - limit->width(), y);
            y += std::max({stock->height(), orders->height(),
                           limit->height()});
            addLine();
        }

        for(const auto type : types) {
            const int c = stor->count(type);

            const auto icon = new eLabel(window());
            icon->setNoPadding();
            const auto tex = eResourceTypeHelpers::icon(
                                 res.uiScale(), type);
            icon->setTexture(tex);
            icon->fitContent();
            icon->setVisible(c > 0);
            addWidget(icon);
            icon->move(0, y + (rowHeight - icon->height())/2);

            const auto count = smallLabel(std::to_string(c));
            count->move(xCount, y + (rowHeight - count->height())/2);

            const auto nameStr = eResourceTypeHelpers::typeName(type);
            eLabel* n = nullptr;
            if(board.editorMode()) {
                const auto button = new eButton(window());
                button->setPressAction([stor, type, count, icon]() {
                    stor->add(type, 1);
                    const int c = stor->count(type);
                    count->setText(std::to_string(c));
                    count->fitContent();
                    icon->setVisible(c > 0);
                });
                n = button;
            } else {
                n = new eLabel(window());
            }
            n->setNoPadding();
            n->setSmallFontSize();
            n->setText(nameStr);
            n->fitContent();
            addWidget(n);
            n->move(xName, y + (rowHeight - n->height())/2);

            // the order, clicked to change it, drawn as plain text
            const auto b = new eSwitchButton(window());
            b->setUnderline(false);
            b->setRenderFrame(false);
            b->setNoPadding();
            b->setSmallFontSize();
            b->addValue(eLanguage::zeusText(130, 1)); // don't accept
            b->addValue(eLanguage::zeusText(130, 0)); // accept
            b->addValue(eLanguage::zeusText(130, 2)); // get
            b->addValue(eLanguage::zeusText(130, 3)); // empty
            b->fitValidContent();
            b->setSwitchAction([changed](const int) {
                changed();
            });
            addWidget(b);
            b->move(xOrder, y + (rowHeight - b->height())/2);
            buttons[type] = b;

            const auto s = new eSpinBox(window());
            s->setDownFirst(true);
            s->setNoPadding();
            s->setHeight(rowHeight);
            s->initialize(changed);
            s->label()->setSmallFontSize();
            const int space = stor->spaceCount();
            if(type == eResourceType::sculpture) {
                s->setRange(0, space);
            } else {
                s->setRange(0, 4*space);
                s->setIncrement(4);
            }
            s->setValue(maxCount.at(type));
            {
                int sw = 0;
                for(const auto ch : s->children()) {
                    sw = std::max(sw, ch->x() + ch->width());
                }
                s->setWidth(sw);
            }
            addWidget(s);
            s->move(width - s->width(), y);
            spinBoxes[type] = s;

            if(static_cast<bool>(get & type)) {
                b->setValue(2);
            } else if(static_cast<bool>(empty & type)) {
                b->setValue(3);
            } else if(static_cast<bool>(accept & type)) {
                b->setValue(1);
            } else {
                b->setValue(0);
            }

            y += rowHeight;
        }
        resize(width, y);
    }
};

eStorageInfoWidget::eStorageInfoWidget(
        eMainWindow* const window, eMainWidget* const mw) :
    eEmployingBuildingInfoWidget(window, mw, true, false) {}

void eStorageInfoWidget::initialize(eStorageBuilding* const stor) {
    const auto title = eBuilding::sNameForBuilding(stor);

    eResourceType get;
    eResourceType empty;
    eResourceType accept;
    stor->getOrders(get, empty, accept);
    const auto all = stor->canAccept();
    const auto& maxCount = stor->maxCount();

    std::vector<eResourceType> types;
    {
        const auto allTypes = eResourceTypeHelpers::extractResourceTypes(all);
        for(const auto t : allTypes) {
            if(goodUsed(stor, t)) types.push_back(t);
        }
        if(types.empty()) types = allTypes;
    }

    // the original's staffing line (group 69) at the top
    const int e = stor->employed();
    const int me = stor->maxEmployees();
    int staffString;
    if(e <= 0) {
        staffString = 9;  // This building has no employees.
    } else if(e >= me) {
        staffString = 15; // ... has all the workers it needs.
    } else if(2*e >= me) {
        staffString = 14; // ... is missing a few workers.
    } else {
        staffString = 13; // ... is missing many workers.
    }

    // what the deliverymen are doing, inside the workers panel (130:27-33)
    std::string activity;
    if(e > 0) {
        bool hasGet = false;
        bool hasStock = false;
        for(const auto t : types) {
            if(static_cast<bool>(get & t)) hasGet = true;
            if(stor->count(t) > 0) hasStock = true;
        }
        int a;
        if(stor->type() == eBuildingType::granary) {
            a = hasGet ? 31 : hasStock ? 32 : 33;
        } else {
            a = hasGet && hasStock ? 28 : hasGet ? 27 : hasStock ? 29 : 30;
        }
        activity = eLanguage::zeusText(130, a);
    }

    eInfoWidget::initialize(title);
    addText(eLanguage::zeusText(69, staffString));
    const auto stWid = addCentralWidget();
    addEmploymentWidget(stor, activity);

    const auto r = new eResourceStorageStack(window());
    const auto changed = [this, stor]() {
        std::map<eResourceType, int> maxCount;
        eResourceType get;
        eResourceType empty;
        eResourceType accept;
        eResourceType dontaccept;
        this->get(get, empty, accept, dontaccept, maxCount);
        // goods left out of the list keep their orders and limits
        const auto& oldMax = stor->maxCount();
        eResourceType oldGet;
        eResourceType oldEmpty;
        eResourceType oldAccept;
        stor->getOrders(oldGet, oldEmpty, oldAccept);
        const auto all = stor->canAccept();
        for(const auto t : eResourceTypeHelpers::extractResourceTypes(all)) {
            if(mButtons.find(t) != mButtons.end()) continue;
            if(static_cast<bool>(oldGet & t)) get = get | t;
            if(static_cast<bool>(oldEmpty & t)) empty = empty | t;
            if(static_cast<bool>(oldAccept & t)) accept = accept | t;
            const auto it = oldMax.find(t);
            if(it != oldMax.end()) maxCount[t] = it->second;
        }
        stor->setOrders(get, empty, accept);
        stor->setMaxCount(maxCount);
    };
    r->initialize(stor, types, get, empty, accept,
                  mButtons, mSpinBoxes, maxCount, changed,
                  stWid->width());
    stWid->addWidget(r);
    r->move(0, 0);
}

void eStorageInfoWidget::get(eResourceType& get,
                             eResourceType& empty,
                             eResourceType& accept,
                             eResourceType& dontaccept,
                             std::map<eResourceType, int>& count) const {
    get = eResourceType::none;
    empty = eResourceType::none;
    accept = eResourceType::none;
    dontaccept = eResourceType::none;
    for(const auto b : mButtons) {
        const auto type = b.first;
        const int val = b.second->currentValue();
        if(val == 0) {
            dontaccept = dontaccept | type;
        } else if(val == 1) {
            accept = accept | type;
        } else if(val == 2) {
            get = get | type;
        } else if(val == 3) {
            empty = empty | type;
        }
    }
    for(const auto s : mSpinBoxes) {
        const auto type = s.first;
        const int val = s.second->value();
        count[type] = val;
    }
}
