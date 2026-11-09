#ifndef ECHEATWIDGET_H
#define ECHEATWIDGET_H

#include "eframedwidget.h"

// The original's cheat box (Ctrl+Alt+C): a code typed exactly, capitals
// included, takes effect at once and closes the box.
class eCheatWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;

    // returns true when the text is a cheat it applied
    using eCheatAction = std::function<bool(const std::string&)>;
    void initialize(const eCheatAction& a);
};

#endif // ECHEATWIDGET_H
