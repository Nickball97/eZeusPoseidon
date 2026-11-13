#ifndef EGAMEMENUBASE_H
#define EGAMEMENUBASE_H

#include "elabel.h"

#include "echeckablebutton.h"
#include "etexturecollection.h"

class eDataWidget;

struct eWid {
    eWid(eWidget* const w) :
        fW(w) {}
    eWid(eWidget* const w, eDataWidget* const dw) :
        fW(w), fDW(dw) {}

    eWidget* fW;
    eDataWidget* fDW = nullptr;
};

class eGameMenuBase : public eLabel {
public:
    using eLabel::eLabel;

    void initialize();
    eCheckableButton* addButton(const eTextureCollection& coll,
                                const eWid& w);
    void connectAndLayoutButtons();
    void layoutButtons();
    void connectButtons();
    // the original's number-key shortcuts: open tab i (0 = first) if it's shown
    bool selectTab(const int i);
    // Tab: switch to the overview tab (the minimap) and back
    void toggleOverviewTab(const int overviewTab);
    // called after a tab button shows its tab
    virtual void tabShown(const int i) { (void)i; }
protected:
    bool mousePressEvent(const eMouseEvent& e) override;
    bool mouseReleaseEvent(const eMouseEvent& e) override;
    bool mouseMoveEvent(const eMouseEvent& e) override;
    bool mouseEnterEvent(const eMouseEvent& e) override;
    bool mouseLeaveEvent(const eMouseEvent& e) override;
private:
    eWidget* mButtonsWidget = nullptr;
    std::vector<eCheckableButton*> mButtons;
    std::vector<eWid> mWidgets;
    int mTabBeforeOverview = -1;
};

#endif // EGAMEMENUBASE_H
