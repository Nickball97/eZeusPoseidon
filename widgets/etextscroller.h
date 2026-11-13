#ifndef ETEXTSCROLLER_H
#define ETEXTSCROLLER_H

#include "ewidget.h"
#include "efontcolor.h"

class eLabel;
class eFramedWidget;
class eScrollWidgetComplete;

class eTextScroller : public eWidget {
public:
    using eWidget::eWidget;

    void initialize();

    void setText(const std::string& text);
    void setTinyTextFontSize();
    void setSmallTextFontSize();
    void setTinyTextPadding();
    // colour of the text (pale in the darker boxes of the mission screens)
    void setTextColor(const eFontColor c);
private:
    eFramedWidget* mBg = nullptr;
    eLabel* mTextLabel = nullptr;
    eLabel* mScrollTextLabel = nullptr;
    eScrollWidgetComplete* mSW = nullptr;
};

#endif // ETEXTSCROLLER_H
