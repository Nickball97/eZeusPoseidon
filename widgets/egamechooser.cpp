#include "egamechooser.h"

#include "eframedbutton.h"
#include "eframedwidget.h"
#include "elanguage.h"
#include "textures/egametextures.h"

#include <algorithm>
#include <cmath>

namespace {
// the two panels painted into Poseidon_FE_MainMenu.JPG (1920x1080)
struct ePanel { double fX0, fY0, fX1, fY1; };
const ePanel gZeusPanel{338, 208, 908, 315};
const ePanel gPoseidonPanel{1012, 208, 1582, 315};
}

void eGameChooser::initialize(const eAction& playZeusA,
                              const eAction& playPoseidonA,
                              const eAction& editorA,
                              const eAction& optionsA,
                              const eAction& quitA) {
    const auto res = resolution();
    const int iRes = static_cast<int>(res.uiScale());
    const auto& texs = eGameTextures::interface()[iRes];
    eMainMenuBase::initialize(texs.fChooseGameImage);

    const auto w = window();
    const double s = picScale();

    const auto addPanel = [&](const ePanel& pn,
                              const std::shared_ptr<eTexture>& logo) {
        const int x0 = picX(pn.fX0);
        const int y0 = picY(pn.fY0);
        const int pw = picX(pn.fX1) - x0;
        const int ph = picY(pn.fY1) - y0;
        if(!wideBackground()) {
            // eZeus's own 4:3 picture has no painted panels
            const auto f = new eFramedWidget(w);
            f->setType(eFrameType::message);
            f->resize(pw, ph);
            addWidget(f);
            f->move(x0, y0);
        }
        const auto l = new eFitPicture(w);
        l->setNoPadding();
        l->setTexture(logo);
        l->resize(pw, std::round(0.6*ph));
        addWidget(l);
        l->move(x0, y0 + (ph - l->height())/2);
    };
    addPanel(gZeusPanel, texs.fZeusLogo);
    addPanel(gPoseidonPanel, texs.fPoseidonLogo);

    // all buttons as wide as the widest label, as in the original
    const std::vector<std::string> labels{
        eLanguage::zeusText(30, 0),  // Play Zeus
        eLanguage::zeusText(30, 1),  // Play Poseidon
        eLanguage::zeusText(287, 3), // Adventure Editor
        eLanguage::zeusText(2, 0),   // Options
        eLanguage::zeusText(30, 4)}; // Quit
    int bw = 0;
    {
        for(const auto& t : labels) {
            const auto b = eHomeButton(w, t, nullptr);
            bw = std::max(bw, b->width());
            b->deleteLater();
        }
    }
    bw += res.largePadding();

    const auto zeusB = eHomeButton(w, labels[0], playZeusA, bw);
    addWidget(zeusB);
    placeOnPic(zeusB, (gZeusPanel.fX0 + gZeusPanel.fX1)/2,
               gZeusPanel.fY1 + 40);
    const auto poseidonB = eHomeButton(w, labels[1], playPoseidonA, bw);
    addWidget(poseidonB);
    placeOnPic(poseidonB, (gPoseidonPanel.fX0 + gPoseidonPanel.fX1)/2,
               gPoseidonPanel.fY1 + 40);

    // the rest in a column below, in the open middle of the picture
    const int gap = std::round(14*s) + zeusB->height();
    double y = 560;
    for(int i = 2; i < 5; i++) {
        const eAction a = i == 2 ? editorA : i == 3 ? optionsA : quitA;
        const auto b = eHomeButton(w, labels[i], a, bw);
        addWidget(b);
        placeOnPic(b, 960, y);
        y += (i == 3 ? 2.0 : 1.0)*gap/s; // Quit set a little apart
    }
}
