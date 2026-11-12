#include "echoosegameeditmenu.h"

#include "eframedwidget.h"
#include "eframedbutton.h"
#include "escrollwidgetcomplete.h"
#include "ebuttonbase.h"
#include "ecancelbutton.h"
#include "emainwindow.h"
#include "eproceedbutton.h"
#include "elineedit.h"
#include "equestionwidget.h"
#include "eeditormainmenu.h"
#include "ebitmapwidget.h"
#include "eepisodeintroductionwidget.h"
#include "estringhelpers.h"
#include "elanguage.h"
#include "egamedir.h"
#include "textures/egametextures.h"

#include <cmath>

#include <filesystem>
namespace fs = std::filesystem;
#include <algorithm>

#include "pak/zeusfile.h"
#include "pak/epakhelpers.h"

bool readPakGlossary(const std::string& filename,
                     eCampaignGlossary& glossary) {
    glossary.fIsPak = true;
    const auto name = eStringHelpers::pathToName(filename);
//    const bool test = name.find("Test") != name.npos;
//    if(test && name != "Test6.pak") return false;
    const auto ext = name.substr(name.size() - 3);
    if(ext != "pak") return false;
    const auto txtFile = filename.substr(0, filename.size() - 3) + "txt";
    glossary.fPakPath = filename;
    std::ifstream file(txtFile);
    ZeusFile in(filename);
    in.readVersion();
    const auto version = in.version();
    const bool poseidon = version == eZeusFileVersion::poseidon_2_0;
    glossary.fPoseidon = poseidon;
    uint8_t bitmapId;
    if(poseidon) {
        in.seek(836249);
    } else {
        in.seek(835185);
    }
    bitmapId = in.readUByte();
    glossary.fBitmap = ePakHelpers::pakBitmapIdConvert(bitmapId);
    if(file.good()) {
        std::map<std::string, std::string> map;
        const bool r = eCampaign::sLoadStrings(txtFile, map);
        if(!r) return false;
        glossary.fTitle = map["Adventure_Title"];
        glossary.fIntroduction = map["Adventure_Introduction"];
        glossary.fComplete = map["Adventure_Complete"];
    } else {
        const std::vector<char> special = {'@', '[', '&', ']',
                                           '{', '}', '^', '#'};
        bool found = false;
        for(const auto c : special) {
            const auto pos = std::find(name.begin(), name.end(), c);
            if(pos != name.end()) {
                found = true;
                break;
            }
        }
        if(!found) return false;
        in.seek(35648);
        const auto briefId = in.readUShort();

        const auto brief = eLanguage::zeusMM(briefId);
        glossary.fTitle = brief.fTitle;
        glossary.fIntroduction = brief.fContent;
    }
    return true;
}

void eChooseGameEditMenu::initialize(const bool editor, const bool custom) {
    const auto& texs = eGameTextures::interface()[
        static_cast<int>(resolution().uiScale())];
    // the adventure lists use the original's tree-and-sea picture, whose
    // frames for the adventure's picture and description are painted in
    if(editor) eMainMenuBase::initialize();
    else eMainMenuBase::initialize(texs.fCampaignImage);

    std::vector<eCampaignGlossary> glossaries;
    {
        const auto folder = eGameDir::adventuresDir();
        std::filesystem::create_directories(folder);
        for(const auto& entry : fs::directory_iterator(folder)) {
            const bool dir = entry.is_directory();
            if(!dir) continue;
            const auto path = entry.path();
            const std::string pathStr = path.u8string();
            const auto name = eStringHelpers::pathToName(pathStr);
            eCampaignGlossary glossary;
            const bool r = eCampaign::sReadGlossary(name, glossary);
            if(r) glossaries.push_back(glossary);
        }
    }
    {
        // adventures in subfolders are the player's own (the original lists
        // them under Custom Adventures); the game's are at the top level
        std::function<void(std::string, bool)> procesFolder;
        procesFolder = [&](const std::string& folder, const bool nested) {
            for(const auto& entry : fs::directory_iterator(folder)) {
                const bool dir = entry.is_directory();
                const auto path = entry.path();
                const std::string pathStr = path.u8string();
                if(dir) {
                    procesFolder(pathStr, true);
                    continue;
                }
                const auto ext = pathStr.substr(pathStr.size() - 3);
                if(ext != "pak") continue;
                eCampaignGlossary glossary;
                const bool r = readPakGlossary(pathStr, glossary);
                glossary.fCustom = nested;
                if(r) glossaries.push_back(glossary);
            }
        };
        const auto folder = eGameDir::pakAdventuresDir();
        procesFolder(folder, false);
    }
    if(!editor) {
        // Adventures: the original game's, for the game picked on the first
        // screen (Zeus or Poseidon); Custom Adventures: eZeus's own
        const bool poseidon = window()->poseidon();
        std::vector<eCampaignGlossary> kept;
        for(const auto& g : glossaries) {
            const bool own = !g.fIsPak || g.fCustom;
            const bool sameGame = !g.fIsPak || g.fPoseidon == poseidon;
            const bool keep = sameGame && (custom ? own : !own);
            if(keep) kept.push_back(g);
        }
        glossaries = kept;
    }

    const auto iww = new eWidget(window());
    iww->setNoPadding();

    const auto res = resolution();
    const double mult = res.multiplier();
    const auto chooseW = new eFramedWidget(window());
    chooseW->setType(eFrameType::message);
    const int p = res.largePadding();
    const int cw = 400*mult;
    const int ch = 400*mult;
    chooseW->resize(cw, ch);
    iww->addWidget(chooseW);

    const auto iw = new eWidget(window());
    iw->setNoPadding();
    iw->resize(cw - 4*p, ch - 4*p);
    chooseW->addWidget(iw);
    iw->move(2*p, 2*p);

    const auto title = new eLabel(window());
    title->setHugeFontSize();
    title->setSmallPadding();
    title->setText(editor ? eLanguage::zeusText(287, 3) :
                   custom ? eLanguage::zeusText(293, 5) :
                            eLanguage::zeusText(293, 9));
    title->fitContent();
    iw->addWidget(title);
    title->align(eAlignment::hcenter);
    if(!editor) { // "Choose an Adventure" under the title, as in the original
        const auto sub = new eLabel(window());
        sub->setTinyFontSize();
        sub->setNoPadding();
        sub->setText(eLanguage::zeusText(287, 5));
        sub->fitContent();
        iw->addWidget(sub);
    }

    const auto scrollCont = new eScrollWidgetComplete(window());
    iw->addWidget(scrollCont);
    scrollCont->resize(cw - 4*p, ch/2);
    scrollCont->initialize();

    if(editor) {
        const auto buttonsW = new eWidget(window());
        buttonsW->setNoPadding();

        const auto newB = new eFramedButton(window());
        newB->setUnderline(false);
        newB->setSmallFontSize();
        newB->setSmallPadding();
        newB->setText(eLanguage::zeusText(287, 0));
        newB->fitContent();
        buttonsW->addWidget(newB);
        newB->setPressAction([cw, p, this]() {
            const auto box = new eFramedWidget(window());
            box->setType(eFrameType::message);
            box->setWidth(cw);

            const auto iw = new eWidget(window());
            iw->setNoPadding();
            iw->setWidth(cw - 4*p);

            const auto title = new eLabel(window());
            title->setHugeFontSize();
            title->setSmallPadding();
            title->setText(eLanguage::zeusText(287, 0));
            title->fitContent();
            iw->addWidget(title);
            title->align(eAlignment::hcenter);

            const auto edit = new eLineEdit(window());
            edit->setRenderBg(true);
            edit->setText(eLanguage::zeusText(287, 0));
            edit->fitContent();
            edit->setText("");
            edit->setWidth(cw - 6*p);
            iw->addWidget(edit);
            edit->align(eAlignment::hcenter);

            const auto buttonsW = new eWidget(window());
            buttonsW->setNoPadding();
            buttonsW->setWidth(iw->width());

            const auto cButton = new eCancelButton(window());
            buttonsW->addWidget(cButton);
            cButton->setPressAction([this]() {
                window()->showChooseGameEditMenu();
            });

            const auto proceedW = new eWidget(window());
            proceedW->setNoPadding();

            const auto proceedLabel = new eLabel(window());
            proceedLabel->setSmallFontSize();
            proceedLabel->setSmallPadding();
            proceedLabel->setText(eLanguage::zeusText(287, 2));
            proceedLabel->fitContent();
            proceedW->addWidget(proceedLabel);

            const auto proceedB = new eProceedButton(window());
            proceedB->setPressAction([this, edit]() {
                auto name = edit->text();
                std::string trimmedName;
                while(!name.empty() && name[0] == ' ') {
                    name.erase(name.begin());
                }
                while(!name.empty() && name[name.size() - 1] == ' ') {
                    name.pop_back();
                }
                if(name.empty()) return;
                eCampaign c;
                c.initialize(name);
                c.save();
                window()->showChooseGameEditMenu();
            });
            proceedW->addWidget(proceedB);

            proceedW->stackHorizontally(2*p);
            proceedW->fitContent();
            proceedLabel->align(eAlignment::vcenter);
            proceedB->align(eAlignment::vcenter);
            buttonsW->addWidget(proceedW);

            buttonsW->layoutHorizontallyWithoutSpaces();
            buttonsW->fitHeight();

            iw->addWidget(buttonsW);

            iw->stackVertically(2*p);
            iw->fitHeight();
            box->addWidget(iw);
            iw->move(2*p, 2*p);
            box->setHeight(iw->height() + 4*p);

            window()->execDialog(box);
            box->align(eAlignment::center);

            edit->grabKeyboard();
        });

        const auto deleteB = new eFramedButton(window());
        deleteB->setUnderline(false);
        deleteB->setSmallFontSize();
        deleteB->setSmallPadding();
        deleteB->setText(eLanguage::zeusText(287, 1));
        deleteB->fitContent();
        buttonsW->addWidget(deleteB);
        deleteB->setPressAction([this]() {
           const auto q = new eQuestionWidget(window());
           const auto acceptA = [this]() {
               const auto dir = eGameDir::adventuresDir() + mSelected.fFolderName + "/";
               std::filesystem::remove_all(dir);
               window()->showChooseGameEditMenu();
           };
           q->initialize(eLanguage::zeusText(5, 179),
                         eLanguage::zeusText(5, 180),
                         acceptA, nullptr);
           window()->execDialog(q);
           q->align(eAlignment::center);
        });

        buttonsW->setWidth(iw->width());
        buttonsW->layoutHorizontallyWithoutSpaces();
        buttonsW->fitHeight();
        iw->addWidget(buttonsW);
    }

    const auto buttons2W = new eWidget(window());
    buttons2W->setNoPadding();

    const auto cButton = new eCancelButton(window());
    buttons2W->addWidget(cButton);
    cButton->setPressAction([this, editor]() {
        // back to where the list was opened from
        if(editor) window()->showGameChooser();
        else window()->showMainMenu();
    });

    const auto proceedW = new eWidget(window());
    proceedW->setNoPadding();

    const auto proceedLabel = new eLabel(window());
    proceedLabel->setSmallFontSize();
    proceedLabel->setSmallPadding();
    proceedLabel->setText(editor ? eLanguage::zeusText(287, 2) :
                                   eLanguage::zeusText(287, 6));
    proceedLabel->fitContent();
    proceedW->addWidget(proceedLabel);

    const auto proceedB = new eProceedButton(window());
    proceedB->setPressAction([this, editor]() {
        if(!mSelected.fIsPak && mSelected.fFolderName.empty()) return;
        if(mSelected.fIsPak && mSelected.fPakPath.empty()) return;
        const auto w = window();

        if(editor) {
            const auto e = new eEditorMainMenu(window());
            e->resize(w->width(), w->height());
            const auto c = std::make_shared<eCampaign>();
            if(mSelected.fIsPak) c->readPak(mSelected.fTitle,
                                            mSelected.fPakPath);
            else c->load(mSelected.fFolderName);
            c->setEditorMode(true);
            e->initialize(c);
            w->setWidget(e);
        } else {
            const auto c = std::make_shared<eCampaign>();
            if(mSelected.fIsPak) c->readPak(mSelected.fTitle,
                                            mSelected.fPakPath);
            else c->load(mSelected.fFolderName);
            w->showEpisodeIntroduction(c);
        }
    });
    proceedW->addWidget(proceedB);

    proceedW->stackHorizontally(2*p);
    proceedW->fitContent();
    proceedLabel->align(eAlignment::vcenter);
    proceedB->align(eAlignment::vcenter);
    buttons2W->addWidget(proceedW);

    buttons2W->setWidth(iw->width());
    buttons2W->layoutHorizontallyWithoutSpaces();
    buttons2W->fitHeight();
    iw->addWidget(buttons2W);

    iw->layoutVerticallyWithoutSpaces();

    const auto sideW = new eWidget(window());
    sideW->setNoPadding();

    const auto bitmapWW = new eFramedWidget(window());
    bitmapWW->setType(eFrameType::message);
    mBitmap = new eBitmapWidget(window());
    mBitmap->setNoPadding();
    mBitmap->fitContent();
    bitmapWW->resize(mBitmap->width() + 2*p, mBitmap->height() + 2*p);
    bitmapWW->addWidget(mBitmap);
    mBitmap->move(p, p);
    sideW->addWidget(bitmapWW);

    const auto descWW = new eFramedWidget(window());
    descWW->setType(eFrameType::message);
    descWW->resize(bitmapWW->width(), 2*bitmapWW->height()/3);

    const auto descW = new eFramedWidget(window());
    descW->setType(eFrameType::inner);
    descW->resize(descWW->width() - 2*p, descWW->height() - 2*p);
    descWW->addWidget(descW);
    descW->move(p, p);

    const auto descIW = new eWidget(window());
    descIW->resize(descW->width() - 2*p, descW->height() - 2*p);
    descW->addWidget(descIW);
    descIW->move(p, p);

    mTitle = new eLabel(window());
    mTitle->setSmallFontSize();
    mTitle->setNoPadding();
    mTitle->setText("Height");
    mTitle->fitContent();
    mTitle->setText("");
    mTitle->setYellowFontColor();
    mTitle->align(eAlignment::hcenter);

    mDesc = new eLabel(window());
    mDesc->setWrapWidth(descIW->width());
    mDesc->setSmallFontSize();
    mDesc->setNoPadding();

    descIW->addWidget(mTitle);
    descIW->addWidget(mDesc);
    descIW->stackVertically(p);

    sideW->addWidget(descWW);

    sideW->stackVertically(p);
    sideW->fitContent();
    iww->addWidget(sideW);

    iww->stackHorizontally(p);
    iww->fitContent();
    addWidget(iww);
    iww->align(eAlignment::center);

    if(!editor && wideBackground()) {
        // Lay the list window and the adventure's picture and description
        // into the original's picture: the list on the left, the picture
        // and the text inside the two frames painted on the right
        // (Zeus_FE_CampaignSelection, 1920x1080: picture frame 896-1672 x
        // 142-585, description frame 896-1672 x 615-934).
        iww->removeWidget(chooseW);
        addWidget(chooseW);
        const int gapX = picX(880) - picX(860);
        chooseW->move(picX(880) - chooseW->width() - gapX,
                      picY(540) - chooseW->height()/2);

        const double s = picScale();
        bitmapWW->removeWidget(mBitmap);
        addWidget(mBitmap);
        // as large as fits the red frame, on its left like the original
        const double bmpH = 300*s;
        mBitmap->setScaling(bmpH/(300*resolution().multiplier()));
        mBitmap->fitContent();
        mBitmap->move(picX(934), picY(142 + (443 - 300)/2.));

        descW->removeWidget(descIW);
        addWidget(descIW);
        const int dx0 = picX(896 + 40);
        const int dx1 = picX(1672 - 40);
        const int dy0 = picY(615 + 28);
        const int dy1 = picY(934 - 28);
        descIW->resize(dx1 - dx0, dy1 - dy0);
        descIW->move(dx0, dy0);
        mDesc->setWrapWidth(descIW->width());
        // pale text in the painted dark box, as in the original
        mDesc->setFontColor(eFontColor::panel);

        removeWidget(iww);
        iww->deleteLater();
    }

    {
        const auto scrollArea = new eWidget(window());

        const auto selected = std::make_shared<eButtonBase*>(nullptr);
        bool first = true;
        for(const auto& g : glossaries) {
            const auto w = new eButtonBase(window());
            w->setTinyFontSize();
            w->setNoPadding();
            w->setText(g.fTitle);
            // pale in the dark list box, as in the original
            w->setFontColor(eFontColor::panel);
            w->fitContent();
            // white marks the item under the mouse or selected, as in the original
            w->setMouseEnterAction([w]() {
                w->setLightFontColor();
            });
            w->setMouseLeaveAction([w, selected]() {
                const auto s = *selected;
                if(s != w) w->setFontColor(eFontColor::panel);
            });
            w->setPressAction([selected, w, g, this]() {
                const auto s = *selected;
                if(s) s->setFontColor(eFontColor::panel);
                setGlossary(g);
                w->setLightFontColor();
                *selected = w;
            });
            if(first) {
                setGlossary(g);
                w->setLightFontColor();
                *selected = w;
                first = false;
            }
            scrollArea->addWidget(w);
        }

        scrollArea->stackVertically();
        scrollArea->setNoPadding();
        scrollArea->fitContent();
        scrollCont->setScrollArea(scrollArea);
    }
}

void eChooseGameEditMenu::setGlossary(const eCampaignGlossary& g) {
    mSelected = g;
    mBitmap->setBitmap(g.fBitmap);
    mTitle->setText(g.fTitle);
    mTitle->fitContent();
    mTitle->align(eAlignment::hcenter);
    auto textPrep = g.fIntroduction;
    eStringHelpers::replaceSpecial(textPrep);
    mDesc->setText(textPrep);
    mDesc->fitContent();
}
