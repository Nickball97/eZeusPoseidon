#ifndef ECHARACTERINFOWIDGET_H
#define ECHARACTERINFOWIDGET_H

#include "einfowidget.h"

class eCharacter;
class eBasicButton;

class eCharacterInfoWidget : public eInfoWidget {
public:
    eCharacterInfoWidget(eMainWindow* const window,
                         eMainWidget* const mw);

    void initialize(const std::vector<eCharacter*> chars);
private:
    void setCharacter(eCharacter* const c);
    // fills the name, occupation and message and lays them out
    void fillTexts(eCharacter* const c, const bool sound);

    eWidget* mContent = nullptr;
    eLabel* mPortraitLabel = nullptr;
    eWidget* mTextWidget = nullptr;
    eLabel* mNameLabel = nullptr;
    eLabel* mOccupationLabel = nullptr;
    eLabel* mMsgLabel = nullptr;
    eLabel* mAdditionalInfo = nullptr;
    eBasicButton* mSpeakButton = nullptr;
    eWidget* mTradeWidget = nullptr; // a caravan's Buys / Sells

    eAction mSpeakAction;
};

#endif // ECHARACTERINFOWIDGET_H
