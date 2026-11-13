#include "einvasioneventwidget.h"

#include "engine/egameboard.h"

#include "elanguage.h"
#include "widgets/eswitchbutton.h"
#include "widgets/elabel.h"
#include "widgets/evaluebutton.h"

void eInvasionEventWidget::initialize(eInvasionEvent* const e) {
    const int p = padding();

    const auto hardcodedButton = new eSwitchButton(window());
    hardcodedButton->setUnderline(false);
    hardcodedButton->addValue(eLanguage::text("hardcoded"));
    hardcodedButton->addValue(eLanguage::text("automatic"));
    hardcodedButton->fitContent();
    hardcodedButton->setSwitchAction([this, e](const int h) {
        mHardcoded = h == 0;
        e->setHardcoded(mHardcoded);
    });
    mHardcoded = e->hardcoded();
    hardcodedButton->setValue(mHardcoded ? 0 : 1);
    addWidget(hardcodedButton);

    // the warships that come with a sea invasion (the original editor's
    // "Warships"; the event's byte 0x36, zeus.exe 0x00503FD0)
    const auto warshipsW = new eWidget(window());
    warshipsW->setNoPadding();
    const auto warshipsL = new eLabel(window());
    warshipsL->setNoPadding();
    warshipsL->setSmallFontSize();
    warshipsL->setText(eLanguage::text("warships"));
    warshipsL->fitContent();
    warshipsW->addWidget(warshipsL);
    const auto warshipsB = new eValueButton(window());
    warshipsB->setSmallFontSize();
    warshipsB->initialize(0, 99);
    warshipsB->setValue(e->warships());
    warshipsB->setValueChangeAction([e](const int v) {
        e->setWarships(v);
    });
    warshipsW->addWidget(warshipsB);
    warshipsW->stackHorizontally(p);
    warshipsW->fitContent();
    warshipsL->align(eAlignment::vcenter);
    warshipsB->align(eAlignment::vcenter);
    addWidget(warshipsW);

    stackVertically(p);
    fitContent();
}
