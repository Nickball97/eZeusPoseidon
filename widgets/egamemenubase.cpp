#include "egamemenubase.h"

#include "datawidgets/edatawidget.h"

void eGameMenuBase::initialize() {
    int x;
    int y;
    switch(resolution().uiScale()) {
    case eUIScale::tiny:
        x = 2;
        y = 11;
        break;
    case eUIScale::small:
        x = 6;
        y = 22;
        break;
    case eUIScale::medium:
        x = 9;
        y = 33;
        break;
    case eUIScale::large:
    default:
        x = 12;
        y = 44;
    }
    mButtonsWidget = new eWidget(window());
    mButtonsWidget->setX(x);
    mButtonsWidget->setY(y);
    mButtonsWidget->setPadding(0);
    addWidget(mButtonsWidget);
}

eCheckableButton* eGameMenuBase::addButton(
        const eTextureCollection& texs,
        const eWid& w) {
    const auto b = eCheckableButton::sCreate(texs, window(), mButtonsWidget);
    mButtons.push_back(b);
    mWidgets.push_back(w);
    return b;
}

void eGameMenuBase::connectAndLayoutButtons() {
    connectButtons();
    layoutButtons();
}

void eGameMenuBase::layoutButtons() {
    double margin;
    switch(resolution().uiScale()) {
    case eUIScale::tiny:
        margin = 1;
        break;
    case eUIScale::small:
        margin = 1;
        break;
    case eUIScale::medium:
        margin = 1.85;
        break;
    case eUIScale::large:
    default:
        margin = 2;
    }
    const int n = mButtons.size();
    const int buttH = mButtons.front()->height();
    const int h = n*(buttH + margin);
    mButtonsWidget->setHeight(h);
    mButtonsWidget->layoutVerticallyWithoutSpaces();
    mButtonsWidget->fitContent();
}

void eGameMenuBase::connectButtons() {
    const int iMax = mButtons.size();
    for(int i = 0; i < iMax; i++) {
        const auto b = mButtons[i];
        b->setCheckAction([this, i, b](const bool c) {
            if(c) {
                const int jMax = mButtons.size();
                const int wSize = mWidgets.size();
                for(int j = 0; j < jMax; j++) {
                    if(j < wSize) {
                        const auto w = mWidgets[j];
                        w.fW->setVisible(j == i);
                        if(j == i && w.fDW) {
                            w.fDW->shown();
                        }
                    }
                    if(j == i) continue;
                    const auto b = mButtons[j];
                    b->setChecked(false);
                }
                tabShown(i);
            } else {
                b->setChecked(true);
            }
        });
    }
}

bool eGameMenuBase::mousePressEvent(const eMouseEvent& e) {
    (void)e;
    return true;
}

bool eGameMenuBase::mouseReleaseEvent(const eMouseEvent& e) {
    (void)e;
    return true;
}

bool eGameMenuBase::mouseMoveEvent(const eMouseEvent& e) {
    (void)e;
    return false;
}

bool eGameMenuBase::mouseEnterEvent(const eMouseEvent& e) {
    (void)e;
    return true;
}

bool eGameMenuBase::mouseLeaveEvent(const eMouseEvent& e) {
    (void)e;
    return true;
}

bool eGameMenuBase::selectTab(const int i) {
    if(i < 0 || i >= static_cast<int>(mButtons.size())) return false;
    const auto b = mButtons[i];
    if(!b->visible()) return false;
    if(!b->checked()) {
        b->trigger();
        return true;
    }
    // the same key again opens the tab's full report (zeus.exe 0x0051CA20
    // opens the tab's advisor); eZeus has one only where the tab shows
    // its "more info" button
    if(i < static_cast<int>(mWidgets.size()) && mWidgets[i].fDW) {
        mWidgets[i].fDW->openMoreInfo();
    }
    return true;
}

void eGameMenuBase::toggleOverviewTab(const int overviewTab) {
    const int n = mButtons.size();
    if(overviewTab < 0 || overviewTab >= n) return;
    const auto ob = mButtons[overviewTab];
    if(ob->checked()) {
        const int t = mTabBeforeOverview;
        mTabBeforeOverview = -1;
        if(t < 0 || t >= n || t == overviewTab) return;
        const auto b = mButtons[t];
        if(b->visible()) b->trigger();
        return;
    }
    mTabBeforeOverview = -1;
    for(int i = 0; i < n; i++) {
        if(mButtons[i]->checked()) {
            mTabBeforeOverview = i;
            break;
        }
    }
    ob->trigger();
}
