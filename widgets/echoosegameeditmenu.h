#ifndef ECHOOSEGAMEEDITMENU_H
#define ECHOOSEGAMEEDITMENU_H

#include "emainmenubase.h"

#include "engine/ecampaign.h"

class eBitmapWidget;

class eChooseGameEditMenu : public eMainMenuBase {
public:
    using eMainMenuBase::eMainMenuBase;
    // custom: the Custom Adventures list (eZeus's own adventures) rather
    // than the original game's adventures; ignored in the editor
    void initialize(const bool editor, const bool custom = false);

    void setGlossary(const eCampaignGlossary& g);
private:
    eCampaignGlossary mSelected;
    eBitmapWidget* mBitmap = nullptr;
    eLabel* mTitle = nullptr;
    eLabel* mDesc = nullptr;
};

#endif // ECHOOSEGAMEEDITMENU_H
