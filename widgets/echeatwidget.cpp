#include "echeatwidget.h"

#include "elineedit.h"

void eCheatWidget::initialize(const eCheatAction& a) {
    setType(eFrameType::message);
    const auto innerWidget = new eWidget(window());
    innerWidget->setNoPadding();
    const int p = 2*padding();
    innerWidget->move(p, p);
    addWidget(innerWidget);

    const auto le = new eLineEdit(window());
    le->setRenderBg(true);
    le->setText("Fireballs from Heaven");
    le->fitContent();
    le->setWidth(3*le->width()/2);
    le->setText("");
    le->grabKeyboard();
    le->setChangeAction([this, le, a]() {
        if(a && a(le->text())) deleteLater();
    });
    innerWidget->addWidget(le);

    innerWidget->fitContent();
    resize(innerWidget->x() + innerWidget->width() + p,
           innerWidget->y() + innerWidget->height() + p);
}
