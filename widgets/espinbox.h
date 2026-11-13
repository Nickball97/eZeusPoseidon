#ifndef ESPINBOX_H
#define ESPINBOX_H

#include "elabel.h"

class eSpinBox : public eWidget {
public:
    using eWidget::eWidget;

    void initialize(const eAction& changed = nullptr);

    void setRange(const int min, const int max);
    void setIncrement(const int i);
    // arrows in the original's order (down, then up); call before
    // initialize
    void setDownFirst(const bool d) { mDownFirst = d; }

    void setValue(const int v);
    int value() const { return mValue; }

    eLabel* label() const { return mValueLabel; }
private:
    int mIncrement = 1;
    bool mDownFirst = false;
    int mMinValue = 0;
    int mMaxValue = 100;
    int mValue = 0;

    eLabel* mValueLabel = nullptr;
};

#endif // ESPINBOX_H
