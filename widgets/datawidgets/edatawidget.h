#ifndef EDATAWIDGET_H
#define EDATAWIDGET_H

#include "../ewidget.h"
#include "engine/ecityid.h"

class eViewModeButton;
class eGameWidget;
class eGameBoard;
class eBasicButton;
class eLabelBase;
enum class eCityId;

// How a side-panel status reads: yellow when it needs the player's
// attention, dark when it's fine, white when it's very good.
enum class eStatusLevel { bad, ok, good };

class eDataWidget : public eWidget {
public:
    eDataWidget(eGameBoard& b, eMainWindow* const w);

    virtual void initialize();

    void setGameWidget(eGameWidget* const gw);
    virtual void shown();
    void update();
    // opens the full report if this tab has one ("more info" button shown)
    void openMoreInfo();
protected:
    virtual void openMoreInfoWiget() {}

    void setLastPersonCityId(const eCityId id) { mLastPersonCityId = id; }
    eCityId lastPersonCityId() const { return mLastPersonCityId; }

    void addViewButton(eViewModeButton* const b);
    eWidget* innerWidget() const { return mInnerWidget; }
    eGameWidget* gameWidget() const { return mGW; }
    int spacing() const;
    void showMoreInfoButton();

    static int sCoverageToText(const int c);
    // the level of a coverage word (text 58, 10 to 14)
    static eStatusLevel sCoverageLevel(const int textId);
public:
    // colours a status label by its level (only redraws when it changes)
    static void sSetStatusColor(eLabelBase* const l, const eStatusLevel s);
protected:

    eCityId viewedCity();

    eGameBoard& mBoard;
    int mTime = 0;
private:
    eCityId mLastPersonCityId = eCityId::neutralFriendly;
    eGameWidget* mGW = nullptr;
    eBasicButton* mMoreInfo = nullptr;
    eWidget* mInnerWidget = nullptr;
    std::vector<eViewModeButton*> mButtons;
};

#endif // EDATAWIDGET_H
