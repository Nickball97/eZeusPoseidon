#include "eemployingbuildinginfowidget.h"

#include "buildings/eemployingbuilding.h"

#include "widgets/elabel.h"
#include "textures/egametextures.h"

#include "elanguage.h"

#include <algorithm>

void eEmployingBuildingInfoWidget::initialize(
        const std::string& title,
        eEmployingBuilding* const b) {
    eInfoWidget::initialize(title);
    addCentralWidget();
    addEmploymentWidget(b);
}

void eEmployingBuildingInfoWidget::initialize(
        const std::string& title,
        const std::string& info,
        const std::string& text,
        eEmployingBuilding* const b,
        const std::string& subText) {
    eInfoWidget::initialize(title);
    // the original's order: the building's state first, then the workers
    // panel, then the description of the building at the bottom
    if(!text.empty()) addText(text);
    addEmploymentWidget(b);
    if(!subText.empty()) addText(subText);
    if(!info.empty()) addText(info);
}

void eEmployingBuildingInfoWidget::addEmploymentWidget(
        eEmployingBuilding* const b, const std::string& activity) {
    const int p = padding();

    // as in the original: a worker figure on the left, "12 Employees
    // (12 needed)" at the top, and what the workers are doing under it
    int iRes;
    int mult;
    iResAndMult(iRes, mult);
    const auto& coll = eGameTextures::interface()[iRes];

    const auto icon = new eLabel(window());
    icon->setNoPadding();
    icon->setTexture(coll.fPopulationTopMenu);
    icon->fitContent();

    const int e = b->employed();
    const int me = b->maxEmployees();
    const auto estr = std::to_string(e);
    const auto mestr = std::to_string(me);
    const auto emplstr = eLanguage::zeusText(8, 13);
    const auto memplstr = eLanguage::zeusText(69, 0);
    const auto str = estr + " " + emplstr +
                     "  (" + mestr + " " + memplstr;
    const auto empl = new eLabel(str, window());
    empl->setSmallFontSize();
    empl->setNoPadding();
    empl->fitContent();

    const int textX = icon->width() + p;
    const int innerW = widgetWidth() - 2*p;
    eLabel* act = nullptr;
    int textH = empl->height();
    if(!activity.empty()) {
        act = new eLabel(window());
        act->setNoPadding();
        act->setSmallFontSize();
        act->setWrapWidth(innerW - textX);
        act->setText(activity);
        act->fitContent();
        textH += p + act->height();
    }
    const int innerH = std::max({icon->height(), textH, 4*p});

    const auto wid = addFramedWidget(innerH + 2*p);
    wid->addWidget(icon);
    icon->move(0, 0);
    wid->addWidget(empl);
    empl->move(textX, 0);
    if(act) {
        wid->addWidget(act);
        act->move(textX, empl->height() + p);
    }
}
