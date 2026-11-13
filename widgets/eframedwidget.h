#ifndef EFRAMEDWIDGET_H
#define EFRAMEDWIDGET_H

#include "ewidget.h"
#include "efontcolor.h"

enum class eFrameType {
    outer, message, inner
};

class eFramedWidget : public eWidget {
public:
    using eWidget::eWidget;

    void setType(const eFrameType type);
    // inner boxes are darker: dark text inside them is drawn in this
    // style instead (darkOnDark unless set; the side panel uses panel)
    void setTextColor(const eFontColor c) { mTextColor = c; }
protected:
    void paintEvent(ePainter& p);
private:
    eFrameType mType = eFrameType::outer;
    eFontColor mTextColor = eFontColor::darkOnDark;
};

#endif // EFRAMEDWIDGET_H
