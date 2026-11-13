#ifndef EGAMEOPTIONS_H
#define EGAMEOPTIONS_H

#include "eframedwidget.h"

// The in-city Options menu (the original's Options drop-down, text group 2)
// and its Sound options (group 46) and Speed options (group 45) windows.
// Changes apply at once; Cancel puts back what was there when the window
// opened, OK keeps them (as zeus.exe 0x00402060 / 0x00402400 do).
class eGameOptionsMenu : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;
    // parent: the widget the windows open over (the city)
    void initialize(eWidget* const parent);
};

class eSoundOptionsWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;
    void initialize();
};

class eSpeedOptionsWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;
    void initialize();
};

#endif // EGAMEOPTIONS_H
