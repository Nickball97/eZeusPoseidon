#ifndef EBITMAPWIDGET_H
#define EBITMAPWIDGET_H

#include "ewidget.h"

class eBitmapWidget : public eWidget {
public:
    using eWidget::eWidget;

    void setBitmap(const int b) {
        mBitmap = b;
    }

    void setScaling(const double s) {
        mScaling = s;
    }
    // fill the whole widget with the picture, keeping its shape and
    // trimming what overflows (the adventure picture in its frame)
    void setFill(const bool f) { mFill = f; }
protected:
    void sizeHint(int& w, int& h);

    void paintEvent(ePainter& p);
private:
    std::shared_ptr<eTexture> texture() const;

    int mBitmap = 0;
    double mScaling = 1.;
    bool mFill = false;
};

#endif // EBITMAPWIDGET_H
