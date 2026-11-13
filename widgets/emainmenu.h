#ifndef EMAINMENU_H
#define EMAINMENU_H

#include "emainmenubase.h"

class eMainMenu : public eMainMenuBase {
public:
    using eMainMenuBase::eMainMenuBase;

    // the player menu; resumeA is empty when there is no save yet
    void initialize(const eAction& resumeA,
                    const eAction& loadA,
                    const eAction& adventuresA,
                    const eAction& customA,
                    const eAction& rosterA);
private:
    bool mousePressEvent(const eMouseEvent& e);
    bool mouseReleaseEvent(const eMouseEvent& e);
    bool mouseMoveEvent(const eMouseEvent& e);
    bool mouseEnterEvent(const eMouseEvent& e);
    bool mouseLeaveEvent(const eMouseEvent& e);

    bool mPressed = false;
    bool mHover = false;
};

#endif // EMAINMENU_H
