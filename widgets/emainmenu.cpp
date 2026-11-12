#include "emainmenu.h"

#include "ebutton.h"
#include "eframedbutton.h"

#include "eframedwidget.h"
#include "elanguage.h"
#include "emainwindow.h"
#include "textures/egametextures.h"

#include <algorithm>
#include <cmath>

namespace {
// the panel painted into Zeus_FE_ChooseGame.jpg (1920x1080)
const double gPanelX0 = 603;
const double gPanelY0 = 135;
const double gPanelX1 = 1317;
const double gPanelY1 = 222;
}

// The player menu, as in the original: the game's logo and the leader's
// name in the panel at the top, then Resume Adventure, Load Saved
// Adventure, Adventures, Custom Adventures and Return to Roster over the
// picture, here lined up evenly. Tutorials are left out: eZeus can't run
// the original's tutorial adventures yet.
void eMainMenu::initialize(const eAction& resumeA,
                           const eAction& loadA,
                           const eAction& adventuresA,
                           const eAction& customA,
                           const eAction& rosterA) {
    const auto res = resolution();
    const int iRes = static_cast<int>(res.uiScale());
    const auto& texs = eGameTextures::interface()[iRes];
    eMainMenuBase::initialize(texs.fPlayerMenuImage);

    const auto w = window();
    const double s = picScale();

    {   // the panel: logo on the left, the leader's name in the middle
        const int x0 = picX(gPanelX0);
        const int y0 = picY(gPanelY0);
        const int pw = picX(gPanelX1) - x0;
        const int ph = picY(gPanelY1) - y0;
        if(!wideBackground()) {
            const auto f = new eFramedWidget(w);
            f->setType(eFrameType::message);
            f->resize(pw, ph);
            addWidget(f);
            f->move(x0, y0);
        }
        const auto logo = new eFitPicture(w);
        logo->setNoPadding();
        logo->setTexture(w->poseidon() ? texs.fPoseidonLogo :
                                         texs.fZeusLogo);
        logo->resize(std::round(0.25*pw), std::round(0.7*ph));
        addWidget(logo);
        logo->move(x0 + std::round(0.03*pw), y0 + (ph - logo->height())/2);

        const auto name = new eLabel(w);
        name->setNoPadding();
        name->setHugeFontSize();
        name->setText(w->leader());
        name->fitContent();
        addWidget(name);
        name->move(x0 + (pw - name->width())/2, y0 + (ph - name->height())/2);
    }

    const std::vector<std::string> labels{
        eLanguage::zeusText(293, 0),  // Resume Adventure
        eLanguage::zeusText(293, 3),  // Load Saved Adventure
        eLanguage::zeusText(293, 9),  // Adventures
        eLanguage::zeusText(293, 5),  // Custom Adventures
        eLanguage::zeusText(293, 6)}; // Return to Roster
    int bw = 0;
    for(const auto& t : labels) {
        const auto b = eHomeButton(w, t, nullptr);
        bw = std::max(bw, b->width());
        b->deleteLater();
    }
    bw += res.largePadding();

    const auto add = [&](const int i, const eAction& a,
                         const double x, const double y) {
        const auto b = eHomeButton(w, labels[i], a, bw);
        addWidget(b);
        placeOnPic(b, x, y);
        return b;
    };
    const auto resumeB = add(0, resumeA, 960, 300);
    const double gap = (resumeB->height() + 14*s)/s;
    if(!resumeA) { // no saved game yet
        resumeB->setEnabled(false);
    }
    add(1, loadA, 700, 470);
    add(2, adventuresA, 1220, 470);
    add(3, customA, 1220, 470 + gap);
    add(4, rosterA, 960, 790);
}

bool eMainMenu::mousePressEvent(const eMouseEvent& e) {
    (void)e;
    mPressed = true;
    return true;
}

bool eMainMenu::mouseReleaseEvent(const eMouseEvent& e) {
    (void)e;
    mPressed = false;
    return true;
}

bool eMainMenu::mouseMoveEvent(const eMouseEvent& e) {
    (void)e;
    return true;
}

bool eMainMenu::mouseEnterEvent(const eMouseEvent& e) {
    (void)e;
    mHover = true;
    return true;
}

bool eMainMenu::mouseLeaveEvent(const eMouseEvent& e) {
    (void)e;
    mHover = false;
    return true;
}
