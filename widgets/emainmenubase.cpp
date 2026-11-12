#include "emainmenubase.h"

#include "textures/egametextures.h"
#include "eframedbutton.h"

#include <algorithm>
#include <cmath>

void eMainMenuBase::initialize() {
    const auto& intrfc = eGameTextures::interface();
    const auto res = resolution();
    const int iRes = static_cast<int>(res.uiScale());
    const auto& texs = intrfc[iRes];
    initialize(texs.fMainMenuImage);
}

void eMainMenuBase::initialize(const std::shared_ptr<eTexture>& bg) {
    setTexture(bg);
    setCoverTexture(true);
}

namespace {
const double gPicW = 1920;
const double gPicH = 1080;
}

bool eMainMenuBase::wideBackground() const {
    const auto& tex = const_cast<eMainMenuBase*>(this)->texture();
    if(!tex) return false;
    return 3*tex->width() > 4*tex->height();
}

double eMainMenuBase::picScale() const {
    return std::max(width()/gPicW, height()/gPicH);
}

int eMainMenuBase::picX(const double x) const {
    const double s = picScale();
    return std::round((width() - gPicW*s)/2 + x*s);
}

int eMainMenuBase::picY(const double y) const {
    const double s = picScale();
    return std::round((height() - gPicH*s)/2 + y*s);
}

void eMainMenuBase::placeOnPic(eWidget* const w,
                               const double x, const double y) const {
    w->move(picX(x) - w->width()/2, picY(y) - w->height()/2);
}

void eFitPicture::paintEvent(ePainter& p) {
    const auto& tex = texture();
    if(!tex || tex->width() <= 0 || tex->height() <= 0) return;
    const double s = std::min(double(width())/tex->width(),
                              double(height())/tex->height());
    const int w = std::round(tex->width()*s);
    const int h = std::round(tex->height()*s);
    const SDL_Rect src{tex->x(), tex->y(), tex->width(), tex->height()};
    const SDL_Rect dst{p.x() + (width() - w)/2, p.y() + (height() - h)/2,
                       w, h};
    tex->render(p.renderer(), src, dst);
}

eFramedButton* eHomeButton(eMainWindow* const window,
                           const std::string& text,
                           const eAction& action,
                           const int w) {
    const auto b = new eFramedButton(window);
    b->setRenderBg(true);
    b->setUnderline(false);
    b->setText(text);
    b->fitContent();
    if(w > 0) b->setWidth(w);
    b->setPressAction(action);
    return b;
}
