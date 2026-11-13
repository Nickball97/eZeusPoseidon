#include "emessageswidget.h"

#include "elabel.h"
#include "ebutton.h"
#include "eframedbutton.h"
#include "eokbutton.h"
#include "esmallupbutton.h"
#include "esmalldownbutton.h"
#include "elanguage.h"
#include "textures/egametextures.h"
#include "engine/egameboard.h"

#include <algorithm>

namespace {
// "Jul 965 BC": the list shows month and year, as the original does
std::string monthYear(const eDate& d) {
    const auto m = eMonthHelper::shortName(d.month());
    const auto y = std::to_string(std::abs(d.year()));
    const auto bcad = eLanguage::zeusText(20, d.year() < 0 ? 0 : 1);
    return m + "  " + y + "  " + bcad;
}
}

void eMessagesWidget::initialize(eGameBoard* const board,
                                 const eOpenAction& open) {
    mBoard = board;
    mOpen = open;
    setType(eFrameType::message);
    const int p = padding();

    const auto cw = new eWidget(window());
    cw->setNoPadding();

    const auto title = new eLabel(eLanguage::zeusText(63, 0), window());
    title->setHugeFontSize();
    title->fitContent();
    cw->addWidget(title);

    // column widths from the longest date and a fixed list width
    {
        const auto probe = new eLabel(window());
        probe->setSmallFontSize();
        probe->setText("May  9999  BC");
        probe->fitContent();
        mDateW = probe->width() + 2*p;
        probe->deleteLater();
    }
    mRowW = 40*p;
    // the envelope column, as wide as the widest envelope picture
    {
        const auto& t = eGameTextures::interface()[static_cast<int>(resolution().uiScale())];
        int iw = 0;
        if(t.fMessageUnread) iw = std::max(iw, t.fMessageUnread->width());
        if(t.fMessageRead) iw = std::max(iw, t.fMessageRead->width());
        mIconW = iw > 0 ? iw + p : 0;
    }
    mRowW += mIconW;

    const auto header = new eWidget(window());
    header->setNoPadding();
    const auto dateH = new eLabel(eLanguage::zeusText(63, 2), window());
    dateH->setSmallFontSize();
    dateH->fitContent();
    header->addWidget(dateH);
    dateH->setX(mIconW);
    const auto subjH = new eLabel(eLanguage::zeusText(63, 3), window());
    subjH->setSmallFontSize();
    subjH->fitContent();
    header->addWidget(subjH);
    subjH->setX(mIconW + mDateW);
    header->fitContent();
    cw->addWidget(header);

    const auto listW = new eFramedWidget(window());
    listW->setType(eFrameType::inner);
    listW->setNoPadding();
    mRowsW = new eWidget(window());
    mRowsW->setNoPadding();
    listW->addWidget(mRowsW);
    mRowsW->move(p, p/2);

    // a row's height, from a sample
    int rowH;
    {
        const auto probe = new eLabel(window());
        probe->setSmallFontSize();
        probe->setText("Ag");
        probe->fitContent();
        rowH = probe->height();
        const auto& t = eGameTextures::interface()[static_cast<int>(resolution().uiScale())];
        if(t.fMessageUnread) rowH = std::max(rowH, t.fMessageUnread->height() + 2);
        probe->deleteLater();
    }
    mRowsW->resize(mRowW, sRows*rowH);
    listW->resize(mRowW + 2*p, sRows*rowH + p);

    const auto arrows = new eWidget(window());
    arrows->setNoPadding();
    const auto up = new eSmallUpButton(window());
    const auto down = new eSmallDownButton(window());
    arrows->addWidget(up);
    arrows->addWidget(down);
    arrows->stackVertically();
    arrows->fitContent();
    up->setPressAction([this]() {
        mTop = std::max(0, mTop - 1);
        updateRows();
    });
    down->setPressAction([this]() {
        const int n = mBoard->messageHistory().count();
        mTop = std::clamp(mTop + 1, 0, std::max(0, n - sRows));
        updateRows();
    });

    const auto listRow = new eWidget(window());
    listRow->setNoPadding();
    listRow->addWidget(listW);
    listRow->addWidget(arrows);
    listRow->stackHorizontally(p);
    listRow->fitContent();
    cw->addWidget(listRow);

    mEmptyW = new eLabel(window());
    {
        const auto l = static_cast<eLabel*>(mEmptyW);
        l->setSmallFontSize();
        l->setWrapWidth(mRowW);
        l->setText(eLanguage::zeusText(63, 1)); // no messages to read
        l->fitContent();
    }
    listW->addWidget(mEmptyW);
    mEmptyW->move(p, p);

    const auto hint = new eLabel(eLanguage::zeusText(63, 4), window());
    hint->setSmallFontSize();
    hint->fitContent();
    cw->addWidget(hint);

    const auto bottom = new eWidget(window());
    bottom->setNoPadding();
    const auto delRead = new eFramedButton(window());
    delRead->setUnderline(false);
    delRead->setSmallFontSize();
    delRead->setSmallPadding();
    delRead->setText(eLanguage::zeusText(63, 6)); // Delete opened messages
    delRead->fitContent();
    delRead->setPressAction([this]() {
        mBoard->messageHistory().removeRead();
        mTop = 0;
        updateRows();
    });
    bottom->addWidget(delRead);
    const auto ok = new eOkButton(window());
    ok->setPressAction([this]() { deleteLater(); });
    bottom->addWidget(ok);
    bottom->stackHorizontally(4*p);
    bottom->fitContent();
    delRead->align(eAlignment::vcenter);
    ok->align(eAlignment::vcenter);
    cw->addWidget(bottom);

    cw->stackVertically(p);
    cw->fitContent();
    addWidget(cw);
    cw->move(2*p, 2*p);
    resize(cw->width() + 4*p, cw->height() + 4*p);
    title->align(eAlignment::hcenter);
    bottom->align(eAlignment::hcenter);

    updateRows();
}

void eMessagesWidget::updateRows() {
    // later, not now: this may run from a row's own click
    const auto old = mRowsW->children();
    for(const auto w : old) w->deleteLater();
    const auto& h = mBoard->messageHistory();
    const int n = h.count();
    mEmptyW->setVisible(n == 0);
    mTop = std::clamp(mTop, 0, std::max(0, n - sRows));
    int y = 0;
    for(int i = mTop; i < std::min(n, mTop + sRows); i++) {
        const auto& r = h.at(i);
        const auto open = [this, i]() {
            if(mOpen) mOpen(i);
            deleteLater();
        };
        const auto remove = [this, i]() {
            mBoard->messageHistory().remove(i);
            updateRows();
        };
        const auto row = new eWidget(window());
        row->setNoPadding();
        const auto make = [&](const std::string& text) {
            const auto b = new eButton(window());
            b->setNoPadding();
            b->setSmallFontSize();
            b->setText(text);
            b->fitContent();
            b->setPressAction(open);
            b->setRightPressAction(remove);
            row->addWidget(b);
            return b;
        };
        // a closed envelope until read, then the scroll (0x00522620)
        {
            const auto& t = eGameTextures::interface()[static_cast<int>(resolution().uiScale())];
            const auto& tex = r.fRead ? t.fMessageRead : t.fMessageUnread;
            if(tex) {
                const auto icon = new eButton(window());
                icon->setNoPadding();
                icon->setTexture(tex);
                icon->fitContent();
                icon->setPressAction(open);
                icon->setRightPressAction(remove);
                row->addWidget(icon);
                icon->setX((mIconW - icon->width())/2);
            }
        }
        const auto date = make(monthYear(r.fDate));
        date->setX(mIconW);
        const auto subj = make(r.fTitle);
        subj->setX(mIconW + mDateW);
        row->fitContent();
        for(const auto c : row->children()) c->align(eAlignment::vcenter);
        mRowsW->addWidget(row);
        row->setY(y);
        y += row->height();
    }
}

void eCondensedMessages::initialize(eGameBoard* const board,
                                    const eOpenAction& open) {
    mBoard = board;
    mOpen = open;
    setNoPadding();

    const auto up = new eSmallUpButton(window());
    const auto down = new eSmallDownButton(window());
    addWidget(up);
    addWidget(down);
    up->setX(width() - up->width());
    down->setX(width() - down->width());
    down->setY(height() - down->height());
    up->setTooltip(eLanguage::zeusText(68, 132)); // scroll through stored messages
    down->setTooltip(eLanguage::zeusText(68, 132));
    up->setPressAction([this]() { // newer
        mIndex = std::max(0, mIndex - 1);
        refresh();
    });
    down->setPressAction([this]() { // older
        const int n = mBoard ? mBoard->messageHistory().count() : 0;
        mIndex = std::clamp(mIndex + 1, 0, std::max(0, n - 1));
        refresh();
    });

    mText = new eButton(window());
    mText->setNoPadding();
    mText->setTinyFontSize();
    mText->setFontColor(eFontColor::panel);
    mText->setWrapWidth(width() - up->width() - 2);
    mText->setTextAlignment(eAlignment::left | eAlignment::top);
    mText->resize(width() - up->width() - 2, height());
    mText->setPressAction([this]() {
        if(!mBoard) return;
        const int n = mBoard->messageHistory().count();
        if(mIndex < n && mOpen) mOpen(mIndex);
    });
    addWidget(mText);
    refresh();
}

void eCondensedMessages::paintEvent(ePainter& p) {
    eWidget::paintEvent(p);
    if(!mBoard) return;
    const auto& h = mBoard->messageHistory();
    if(h.revision() == mRevision) return;
    // a new message: show it
    if(h.count() > mCount) mIndex = 0;
    refresh();
}

void eCondensedMessages::refresh() {
    if(!mBoard || !mText) return;
    const auto& h = mBoard->messageHistory();
    mRevision = h.revision();
    mCount = h.count();
    if(mCountLabel && mCountButton) {
        mCountLabel->setText(mCount > 0 ? std::to_string(mCount) : "");
        mCountLabel->fitContent();
        // on the envelope, centred and a little below its middle, as in
        // the original
        const int bh = mCountButton->height();
        mCountLabel->setX(mCountButton->x() +
                          (mCountButton->width() - mCountLabel->width())/2);
        mCountLabel->setY(mCountButton->y() +
                          (bh - mCountLabel->height())/2 + bh/8);
        mCountLabel->setVisible(mCount > 0);
    }
    mIndex = std::clamp(mIndex, 0, std::max(0, mCount - 1));
    if(mCount == 0) {
        mText->setText("");
        return;
    }
    mText->setText(h.at(mIndex).fCondensed);
}

void eCondensedMessages::setCountLabel(eLabel* const l, eWidget* const button) {
    mCountLabel = l;
    mCountButton = button;
    mRevision = -1; // show it at the next paint
}
