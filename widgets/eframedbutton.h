#ifndef EFRAMEDBUTTON_H
#define EFRAMEDBUTTON_H

#include "ebutton.h"

class eFramedButton : public eButton {
public:
    using eButton::eButton;

    void setRenderBg(const bool r) { mRenderBg = r; }
    // false: draw only the text, like the original's clickable orders
    // ("Accept", "Get") in storage windows
    void setRenderFrame(const bool f) { mRenderFrame = f; }
protected:
    void paintEvent(ePainter& p);
private:
    void renderBg(ePainter& p);

    bool mRenderBg = false;
    bool mRenderFrame = true;
};

#endif // EFRAMEDBUTTON_H
