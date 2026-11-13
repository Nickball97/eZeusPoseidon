#include "egameoptions.h"

#include "elabel.h"
#include "eframedbutton.h"
#include "eupbutton.h"
#include "edownbutton.h"
#include "elanguage.h"
#include "emainwindow.h"
#include "emessages.h"

#include <algorithm>

namespace {
eFramedButton* makeButton(eMainWindow* const w, const std::string& text) {
    const auto b = new eFramedButton(w);
    b->setUnderline(false);
    b->setSmallFontSize();
    b->setSmallPadding();
    b->setText(text);
    b->fitContent();
    return b;
}

eLabel* makeLabel(eMainWindow* const w, const std::string& text,
                  const bool huge = false) {
    const auto l = new eLabel(w);
    l->setNoPadding();
    if(huge) l->setHugeFontSize();
    else l->setSmallFontSize();
    l->setText(text);
    l->fitContent();
    return l;
}

std::string percent(const int v) {
    return std::to_string(v) + "%";
}

// "90%" with a down and an up arrow; the label keeps the width of "100%"
eWidget* makeValue(eMainWindow* const w, const int p,
                   const std::function<int()>& get,
                   const std::function<void(int)>& change) {
    const auto row = new eWidget(w);
    row->setNoPadding();
    const auto l = makeLabel(w, percent(100));
    l->setTextAlignment(eAlignment::right | eAlignment::vcenter);
    l->setText(percent(get()));
    row->addWidget(l);
    const auto down = new eDownButton(w);
    const auto up = new eUpButton(w);
    row->addWidget(down);
    row->addWidget(up);
    down->setPressAction([l, get, change]() {
        change(-1);
        l->setText(percent(get()));
    });
    up->setPressAction([l, get, change]() {
        change(1);
        l->setText(percent(get()));
    });
    row->stackHorizontally(p);
    row->fitContent();
    l->align(eAlignment::vcenter);
    down->align(eAlignment::vcenter);
    up->align(eAlignment::vcenter);
    return row;
}

// OK keeps the changes, Cancel puts back the settings from before
eWidget* makeOkCancel(eWidget* const dialog, const int p,
                      const int okGroup, const int okId,
                      const int cancelGroup, const int cancelId,
                      const std::shared_ptr<eSettings>& before) {
    const auto w = dialog->window();
    const auto row = new eWidget(w);
    row->setNoPadding();
    const auto ok = makeButton(w, eLanguage::zeusText(okGroup, okId));
    const auto cancel = makeButton(w, eLanguage::zeusText(cancelGroup, cancelId));
    const int bw = std::max(ok->width(), cancel->width());
    ok->setWidth(bw);
    cancel->setWidth(bw);
    ok->setPressAction([dialog]() { dialog->deleteLater(); });
    cancel->setPressAction([dialog, w, before]() {
        w->saveOptions(*before);
        dialog->deleteLater();
    });
    row->addWidget(ok);
    row->addWidget(cancel);
    row->stackHorizontally(4*p);
    row->fitContent();
    return row;
}

void finish(eFramedWidget* const dialog, eWidget* const cw, const int p,
            const int spacing = 1) {
    cw->stackVertically(spacing*p);
    cw->fitContent();
    dialog->addWidget(cw);
    cw->move(2*p, 2*p);
    dialog->resize(cw->width() + 4*p, cw->height() + 4*p);
}
}

void eGameOptionsMenu::initialize(eWidget* const parent) {
    setType(eFrameType::message);
    const auto w = window();
    const int p = padding();

    const auto cw = new eWidget(w);
    cw->setNoPadding();
    const auto title = makeLabel(w, eLanguage::zeusText(2, 0), true); // Options
    cw->addWidget(title);

    const auto open = [this, w, parent](eFramedWidget* const d) {
        w->execDialog(d, true, nullptr, parent);
        d->align(eAlignment::center);
        deleteLater();
    };
    const auto sound = makeButton(w, eLanguage::zeusText(2, 2)); // Sound options
    const auto speed = makeButton(w, eLanguage::zeusText(2, 3)); // Speed options
    const auto msgs = makeButton(w, eLanguage::zeusText(2, 13)); // Messages
    const int bw = std::max({sound->width(), speed->width(), msgs->width()});
    sound->setWidth(bw);
    speed->setWidth(bw);
    msgs->setWidth(bw);
    msgs->setPressAction([w, open]() {
        const auto d = new eMessageOptionsWidget(w);
        d->initialize();
        open(d);
    });
    sound->setPressAction([w, open]() {
        const auto d = new eSoundOptionsWidget(w);
        d->initialize();
        open(d);
    });
    speed->setPressAction([w, open]() {
        const auto d = new eSpeedOptionsWidget(w);
        d->initialize();
        open(d);
    });
    cw->addWidget(sound);
    cw->addWidget(speed);
    cw->addWidget(msgs);
    finish(this, cw, p, 2);
    title->align(eAlignment::hcenter);
    sound->align(eAlignment::hcenter);
    speed->align(eAlignment::hcenter);
    msgs->align(eAlignment::hcenter);
}

void eSoundOptionsWidget::initialize() {
    setType(eFrameType::message);
    const auto w = window();
    const int p = padding();
    const auto before = std::make_shared<eSettings>(w->settings());
    const auto cur = std::make_shared<eSettings>(w->settings());

    const auto cw = new eWidget(w);
    cw->setNoPadding();
    const auto title = makeLabel(w, eLanguage::zeusText(46, 0), true);
    cw->addWidget(title);

    // rows as in zeus.exe 0x0059B5E0: music, speech, general sounds,
    // ambient sounds; text 46: off/on pairs 1-2, 3-4, 5-6, 7-8
    struct eRow {
        int fTextId;
        bool eSettings::* fOn;
        int eSettings::* fVolume;
    };
    const eRow rows[] = {
        {1, &eSettings::fMusicOn, &eSettings::fMusicVolume},
        {3, &eSettings::fSpeechOn, &eSettings::fSpeechVolume},
        {5, &eSettings::fSoundsOn, &eSettings::fSoundsVolume},
        {7, &eSettings::fAmbientOn, &eSettings::fAmbientVolume}
    };

    int toggleW = 0;
    for(const auto& r : rows) {
        for(int i = 0; i < 2; i++) {
            const auto b = makeButton(w, eLanguage::zeusText(46, r.fTextId + i));
            toggleW = std::max(toggleW, b->width());
            b->deleteLater();
        }
    }

    const auto header = new eWidget(w);
    header->setNoPadding();
    const auto stateL = makeLabel(w, eLanguage::zeusText(46, 10)); // Current audio state
    const auto volumeL = makeLabel(w, eLanguage::zeusText(46, 11)); // Volume
    header->addWidget(stateL);
    header->addWidget(volumeL);
    cw->addWidget(header);

    int valueX = 0;
    for(const auto& r : rows) {
        const auto row = new eWidget(w);
        row->setNoPadding();
        const auto on = r.fOn;
        const auto vol = r.fVolume;
        const int textId = r.fTextId;
        const auto toggle = makeButton(w, eLanguage::zeusText(46, textId + ((*cur).*on ? 1 : 0)));
        toggle->setWidth(toggleW);
        toggle->setPressAction([w, cur, on, textId, toggle]() {
            (*cur).*on = !((*cur).*on);
            toggle->setText(eLanguage::zeusText(46, textId + ((*cur).*on ? 1 : 0)));
            w->saveOptions(*cur);
        });
        row->addWidget(toggle);
        // volume 0-100; the original's arrow step isn't traced, 10 is used
        const auto get = [cur, vol]() { return (*cur).*vol; };
        const auto change = [w, cur, vol](const int dir) {
            (*cur).*vol = std::clamp((*cur).*vol + 10*dir, 0, 100);
            w->saveOptions(*cur);
        };
        const auto value = makeValue(w, p, get, change);
        row->addWidget(value);
        row->stackHorizontally(4*p);
        row->fitContent();
        toggle->align(eAlignment::vcenter);
        value->align(eAlignment::vcenter);
        valueX = value->x();
        cw->addWidget(row);
    }
    stateL->setX(0);
    volumeL->setX(valueX);
    header->fitContent();

    const auto okCancel = makeOkCancel(this, p, 46, 12, 46, 9, before);
    cw->addWidget(okCancel);
    finish(this, cw, p);
    title->align(eAlignment::hcenter);
    okCancel->align(eAlignment::hcenter);
}

void eSpeedOptionsWidget::initialize() {
    setType(eFrameType::message);
    const auto w = window();
    const int p = padding();
    const auto before = std::make_shared<eSettings>(w->settings());
    const auto cur = std::make_shared<eSettings>(w->settings());

    const auto cw = new eWidget(w);
    cw->setNoPadding();
    const auto title = makeLabel(w, eLanguage::zeusText(45, 0), true);
    cw->addWidget(title);

    // Game speed 10-100% in steps of 10 (zeus.exe 0x00402430). The
    // original's scroll speed isn't offered: how far it scrolls depends on
    // its frame rate, which isn't known.
    const auto row = new eWidget(w);
    row->setNoPadding();
    const auto label = makeLabel(w, eLanguage::zeusText(45, 2)); // Game speed
    row->addWidget(label);
    const auto get = [cur]() { return cur->fGameSpeed; };
    const auto change = [w, cur](const int dir) {
        cur->fGameSpeed = std::clamp(cur->fGameSpeed + 10*dir, 10, 100);
        w->saveOptions(*cur);
    };
    const auto value = makeValue(w, p, get, change);
    row->addWidget(value);
    row->stackHorizontally(4*p);
    row->fitContent();
    label->align(eAlignment::vcenter);
    value->align(eAlignment::vcenter);
    cw->addWidget(row);

    const auto okCancel = makeOkCancel(this, p, 45, 4, 45, 1, before);
    cw->addWidget(okCancel);
    finish(this, cw, p);
    title->align(eAlignment::hcenter);
    row->align(eAlignment::hcenter);
    okCancel->align(eAlignment::hcenter);
}

void eMessageOptionsWidget::initialize() {
    // as the original's Messages options (zeus.exe 0x0059BBE0, 0x004026C0):
    // a row per category (text group 174) with its setting (group 183:
    // Off, Condensed, Full); clicking a setting moves it to the next
    setType(eFrameType::message);
    const auto w = window();
    const int p = padding();
    const auto before = std::make_shared<eSettings>(w->settings());
    const auto cur = std::make_shared<eSettings>(w->settings());

    const auto cw = new eWidget(w);
    cw->setNoPadding();
    const auto title = makeLabel(w, eLanguage::zeusText(2, 13), true); // Messages
    cw->addWidget(title);

    int nameW = 0;
    int valueW = 0;
    for(int i = 0; i < eMessages::sCategoryCount; i++) {
        const auto l = makeLabel(w, eLanguage::zeusText(174, i));
        nameW = std::max(nameW, l->width());
        l->deleteLater();
    }
    for(int v = 0; v < 3; v++) {
        const auto b = makeButton(w, eLanguage::zeusText(183, v));
        valueW = std::max(valueW, b->width());
        b->deleteLater();
    }

    const auto list = new eWidget(w);
    list->setNoPadding();
    for(int i = 0; i < eMessages::sCategoryCount; i++) {
        const auto row = new eWidget(w);
        row->setNoPadding();
        const auto name = makeLabel(w, eLanguage::zeusText(174, i));
        name->setTextAlignment(eAlignment::left | eAlignment::vcenter);
        name->setWidth(nameW);
        row->addWidget(name);
        const auto value = makeButton(w, eLanguage::zeusText(183,
                                          cur->messageSetting(i)));
        value->setWidth(valueW);
        value->setPressAction([w, cur, i, value]() {
            const int v = (cur->messageSetting(i) + 1) % 3;
            cur->fMessageSettings[i] = static_cast<char>('0' + v);
            value->setText(eLanguage::zeusText(183, v));
            w->saveOptions(*cur);
        });
        row->addWidget(value);
        row->stackHorizontally(4*p);
        row->fitContent();
        name->align(eAlignment::vcenter);
        value->align(eAlignment::vcenter);
        list->addWidget(row);
    }
    list->stackVertically();
    list->fitContent();
    cw->addWidget(list);

    const auto okCancel = makeOkCancel(this, p, 46, 12, 46, 9, before);
    cw->addWidget(okCancel);
    finish(this, cw, p);
    title->align(eAlignment::hcenter);
    list->align(eAlignment::hcenter);
    okCancel->align(eAlignment::hcenter);
}
