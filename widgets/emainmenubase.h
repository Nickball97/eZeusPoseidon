#ifndef EMAINMENUBASE_H
#define EMAINMENUBASE_H

#include "elabel.h"

class eMainMenuBase : public eLabel {
public:
    using eLabel::eLabel;

    // the Acropolis picture (Zeus_FE_Registry), as before
    void initialize();
    // any full-screen picture, scaled to cover the screen
    void initialize(const std::shared_ptr<eTexture>& bg);

    // The home page pictures are 1920x1080 in the game folder, with the
    // original's framed panels painted in. These turn a point or a length
    // on that picture into this widget's coordinates, following the same
    // scaling as the picture (cover, centred).
    bool wideBackground() const;
    double picScale() const;
    int picX(const double x) const;
    int picY(const double y) const;
    // moves w so its centre sits at (x, y) on the picture
    void placeOnPic(eWidget* const w, const double x, const double y) const;
};

class eFramedButton;

// a button of the home page, in the original's style (framed, on the
// parchment), w wide if w > 0
eFramedButton* eHomeButton(eMainWindow* const window,
                           const std::string& text,
                           const eAction& action,
                           const int w = 0);

// a picture drawn scaled to fit its rectangle, keeping its shape (the
// game logos on the home page panels)
class eFitPicture : public eLabel {
public:
    using eLabel::eLabel;
protected:
    void paintEvent(ePainter& p) override;
};


#endif // EMAINMENUBASE_H
