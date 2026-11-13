#include "emainwindow.h"

#include "widgets/emainmenu.h"
#include "widgets/egamechooser.h"
#include "widgets/esettingsmenu.h"
#include "widgets/egamewidget.h"
#include "widgets/egameloadingwidget.h"
#include "widgets/egamemenu.h"
#include "widgets/emenuloadingwidget.h"
#include "widgets/eworldwidget.h"
#include "widgets/echoosegameeditmenu.h"
#include "widgets/eselectcolonywidget.h"

#include "audio/emusic.h"

#include "engine/ethreadpool.h"

#include "egamedir.h"

#include "fileIO/ereadstream.h"

#include <chrono>
#include <filesystem>
#include <fstream>

#include "widgets/efilewidget.h"
#include "elanguage.h"

#include "evectorhelpers.h"

#include "widgets/eeventbackground.h"
#include "widgets/eepisodeintroductionwidget.h"
#include "widgets/eepisodelostwidget.h"
#include "widgets/erosterofleaders.h"
#include "widgets/equestionwidget.h"
#include "widgets/ecursors.h"

eMainWindow::eMainWindow() {}

eMainWindow::~eMainWindow() {
    if(mSdlWindow) SDL_DestroyWindow(mSdlWindow);
    if(mSdlRenderer) SDL_DestroyRenderer(mSdlRenderer);
    setWidget(nullptr);
}

bool eMainWindow::initialize(const eSettings& settings) {
    const auto& res = settings.fRes;
    const int w = res.width();
    const int h = res.height();
    const auto window = SDL_CreateWindow("eZeus",
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         w, h, SDL_WINDOW_SHOWN);

    if(!window) {
        printf("Window could not be created! SDL Error: %s\n",
               SDL_GetError());
        return false;
    }
    const Uint32 flags = SDL_RENDERER_ACCELERATED/* |
                         SDL_RENDERER_PRESENTVSYNC*/;
    const auto renderer = SDL_CreateRenderer(window, -1, flags);
    if(!renderer) {
        printf("Renderer could not be created! SDL Error: %s\n",
               SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    if(mSdlWindow) SDL_DestroyWindow(mSdlWindow);
    if(mSdlRenderer) SDL_DestroyRenderer(mSdlRenderer);
    mSdlWindow = window;
    mSdlRenderer = renderer;
    setResolution(res);
    setFullscreen(settings.fFullscreen, settings.fBorderless);
    mSettings = settings;
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    const std::string icoPath = eGameDir::path("zeus.ico");
    const auto icon = IMG_Load(icoPath.c_str());
    SDL_SetWindowIcon(window, icon);
    eGameTextures::setSettings(mSettings);
    return true;
}

void eMainWindow::setWidget(eWidget* const w) {
    eCursors::set(eCursors::eCursor::arrow); // a new screen: the normal pointer
    if(mWidget) {
        if(mWidget != mGW && mWidget != mWW) {
            mWidget->deleteLater();
        }
    }
    mWidget = w;
}

eWidget* eMainWindow::takeWidget() {
    const auto w = mWidget;
    mWidget = nullptr;
    return w;
}

void eMainWindow::addSlot(const eSlot& slot) {
    mSlots.push_back(slot);
}

void eMainWindow::setResolution(const eResolution& res) {
    if(mSettings.fRes == res && !mFirstFullscrenSetting) return;
    mFirstResolutionSetting = false;
    mSettings.fRes = res;
    const int w = res.width();
    const int h = res.height();
    SDL_SetWindowSize(mSdlWindow, w, h);
    updateLogicalSize();
}

void eMainWindow::setFullscreen(const bool f) {
    setFullscreen(f, mSettings.fBorderless);
}

void eMainWindow::setFullscreen(const bool f, const bool borderless) {
    if(mSettings.fFullscreen == f &&
       mSettings.fBorderless == borderless &&
       !mFirstFullscrenSetting) return;
    mFirstFullscrenSetting = false;
    mSettings.fFullscreen = f;
    mSettings.fBorderless = borderless;
    Uint32 flags = 0;
    if(f) {
        flags = borderless ? SDL_WINDOW_FULLSCREEN_DESKTOP :
                             SDL_WINDOW_FULLSCREEN;
    }
    SDL_SetWindowFullscreen(mSdlWindow, flags);
    updateLogicalSize();
}

void eMainWindow::updateLogicalSize() {
    if(!mSdlRenderer) return;
    if(mSettings.fFullscreen && mSettings.fBorderless) {
        // the window is the desktop's size; draw at the chosen resolution
        // and let SDL stretch it (with black bars if the shape differs)
        const auto& res = mSettings.fRes;
        SDL_RenderSetLogicalSize(mSdlRenderer, res.width(), res.height());
    } else {
        SDL_RenderSetLogicalSize(mSdlRenderer, 0, 0);
    }
}

void eMainWindow::mousePosition(int& x, int& y) const {
    int wx;
    int wy;
    SDL_GetMouseState(&wx, &wy);
    float lx;
    float ly;
    SDL_RenderWindowToLogical(mSdlRenderer, wx, wy, &lx, &ly);
    x = static_cast<int>(lx);
    y = static_cast<int>(ly);
}

void eMainWindow::warpMouse(const int x, const int y) {
    int wx;
    int wy;
    SDL_RenderLogicalToWindow(mSdlRenderer, x, y, &wx, &wy);
    SDL_WarpMouseInWindow(mSdlWindow, wx, wy);
}

void eMainWindow::toggleFullscreen() {
    setFullscreen(!mSettings.fFullscreen);
    mSettings.write();
}

void eMainWindow::centerWindow() {
    if(mSettings.fFullscreen) return;
    SDL_SetWindowPosition(mSdlWindow, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
}

void eMainWindow::startGameAction(eGameBoard* const board,
                                  const eGameWidgetSettings& settings) {
    const auto show = [this, board, settings]() {
        showGame(board, settings);
    };
    startGameAction(show);
}

void eMainWindow::startGameAction(const stdsptr<eCampaign>& c,
                                  const eGameWidgetSettings& settings) {
    const auto show = [this, c, settings]() {
        showGame(c, settings);
    };
    startGameAction(show);
}

void eMainWindow::startGameAction(const eAction& a) {
    clearWidgets();
    const auto l = new eGameLoadingWidget(this);
    l->resize(width(), height());
    l->setDoneAction(a);
    setWidget(l);
    l->initialize();
}

void eMainWindow::showEpisodeIntroduction(
        const stdsptr<eCampaign>& c) {
    clearWidgets();
    if(c) mCampaign = c;
    const auto e = new eEpisodeIntroductionWidget(this);
    const auto proceedA = [this]() {
        mCampaign->startEpisode();
        const auto dir = leaderSaveDir();
        saveGame(dir + "autosave replay.ez");
        startGameAction([this]() {
            eGameWidgetSettings settings;
            settings.fPaused = true;
            showGame(mCampaign, settings);
        });
    };
    e->resize(width(), height());
    const auto ee = mCampaign->currentEpisode();

    // the original plays the intro music under the narrator
    eMusic::playMissionIntroMusic();
    const auto path = mCampaign->currentEpisodeAudioFilePath(true);
    eMusic::playCampaignVoice(path);

    e->initialize(mCampaign,
                  mCampaign->titleText(),
                  ee->fIntroduction,
                  ee->fGoals,
                  proceedA,
                  eEpisodeIntroType::intro);
    setWidget(e);
}

std::string eMainWindow::leaderSaveDir() const {
    return eGameDir::saveDir() + mLeader + "/";
}

void eMainWindow::clearWidgets() {
    if(mGW && mWidget != mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }
    if(mWW && mWidget != mWW) {
        mWW->deleteLater();
        mWW = nullptr;
    }
}

void eMainWindow::episodeFinished() {
    clearWidgets();
    if(!mCampaign) return;
    mCampaign->episodeFinished();
    const bool f = mCampaign->finished();
    if(f) return adventureComplete();
    const auto n = mCampaign->currentEpisodeType();
    if(n == eEpisodeType::parentCity) {
        showEpisodeIntroduction();
    } else {
        const auto w = new eSelectColonyWidget(this);
        const auto sel = mCampaign->remainingColonies();
        const auto selA = [this](const stdsptr<eWorldCity>& c) {
            int cid = 0;
            const auto& eps = mCampaign->colonyEpisodes();
            for(const auto& e : eps) {
                if(e->fCity == c) break;
                cid++;
            }
            mCampaign->setCurrentColonyEpisode(cid);
            showEpisodeIntroduction();
        };
        w->resize(width(), height());
        w->initialize(sel, selA, &mCampaign->worldBoard());
        setWidget(w);
    }
}

void eMainWindow::adventureComplete() {
    clearWidgets();
    if(!mCampaign) return;
    const auto e = new eEpisodeIntroductionWidget(this);
    const auto proceedA = [this]() {
        showMainMenu();
    };
    e->resize(width(), height());

    // the original plays the victory music under the narrator
    eMusic::playCampaignVictoryMusic();
    const auto path = mCampaign->adventureVictoryAudioFilePath();
    eMusic::playCampaignVoice(path);

    e->initialize(mCampaign,
                  eLanguage::zeusText(62, 0),
                  mCampaign->completeText(),
                  {},
                  proceedA,
                  eEpisodeIntroType::campaingVictory);
    setWidget(e);
}

void eMainWindow::episodeLost() {
    clearWidgets();
    const auto e = new eEpisodeLostWidget(this);
    const auto proceedA = [this]() {
        showMainMenu();
    };
    e->resize(width(), height());
    e->initialize(proceedA);
    setWidget(e);
}

bool eMainWindow::saveGame(const std::string& path) {
    namespace fs = std::filesystem;
    if(!mCampaign) return false;
    // Make sure no board task is still running while the state is written.
    mCampaign->waitUntilFinished();

    std::error_code ec;
    const auto fsp = fs::u8path(path);
    const auto fspd = fsp.parent_path();
    if(!fspd.empty()) {
        fs::create_directories(fspd, ec);
        if(ec) {
            printf("Could not create save directory '%s': %s\n",
                   fspd.u8string().c_str(), ec.message().c_str());
            return false;
        }
    }

    // Write to a temporary file first so that a failed save never
    // destroys an existing save with the same name.
    auto tmp = fsp;
    tmp += ".tmp";
    {
        std::ofstream file(tmp, std::ios::out | std::ios::binary |
                           std::ios::trunc);
        if(!file) {
            printf("Could not open '%s' for writing.\n",
                   tmp.u8string().c_str());
            return false;
        }
        eWriteTarget target(&file);
        eWriteStream dst(target);
        dst.writeFormat("eZeus.ez");
        if(mGW) {
            const auto s = mGW->settings();
            s.write(dst);
        } else {
            eGameWidgetSettings s;
            s.fPaused = true;
            s.write(dst);
        }
        mCampaign->write(dst);
        file.flush();
        const bool ok = static_cast<bool>(file);
        file.close();
        if(!ok || file.fail()) {
            printf("Failed writing '%s'.\n", tmp.u8string().c_str());
            fs::remove(tmp, ec);
            return false;
        }
    }

    fs::rename(tmp, fsp, ec);
    if(ec) {
        // Some platforms refuse to replace an existing file on rename.
        std::error_code rec;
        fs::remove(fsp, rec);
        ec.clear();
        fs::rename(tmp, fsp, ec);
    }
    if(ec) {
        printf("Could not move '%s' to '%s': %s\n",
               tmp.u8string().c_str(), fsp.u8string().c_str(),
               ec.message().c_str());
        std::error_code rec;
        fs::remove(tmp, rec);
        return false;
    }
    return true;
}

bool eMainWindow::loadGame(const std::string& path) {
    std::ifstream file(std::filesystem::u8path(path),
                       std::ios::in | std::ios::binary);
    if(!file) return false;
    eReadSource source(&file);
    eReadStream src(source);
    src.readFormat();
    const auto& format = src.format();
    const int version = src.formatVersion();
    if(format != "eZeus.ez") {
        printf("Invalid file '%s' format '%s', expected 'eZeus.ez'.\n",
               path.c_str(), format.c_str());
        return false;
    }
    if(version > eFileFormat::version) {
        printf("Attempting to read '%s' format '%s' version '%i' newer than the executable.\n",
               path.c_str(), format.c_str(), version);
    }
    eGameWidgetSettings s;
    s.read(src);
    const auto c = std::make_shared<eCampaign>();
    c->read(src);
    c->loadStrings();
    c->loadNumbers();
    src.handlePostFuncs();
    file.close();

    startGameAction(c, s);
    return true;
}

void eMainWindow::closeGame() {
    if(!mGW) return;
    if(mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }
    if(mWW) {
        mWW->deleteLater();
        mWW = nullptr;
    }
    showMainMenu();
}

void eMainWindow::showRosterOfLeaders() {
    clearWidgets();
    eMusic::playMenuMusic();
    const auto rol = new eRosterOfLeaders(this);
    rol->resize(width(), height());
    rol->initialize();
    setWidget(rol);
}

void eMainWindow::showMenuLoading() {
    const auto mlw = new eMenuLoadingWidget(this);
    mlw->setDoneAction([this]() {
        const auto ls = eRosterOfLeaders::sLeaders();
        if(ls.size() == 1) setLeader(ls[0]);
        showGameChooser();
    });
    mlw->initialize();
    mlw->resize(width(), height());
    setWidget(mlw);
}

void eMainWindow::showGameChooser() {
    mCampaign = nullptr;
    clearWidgets();
    eMusic::playMenuMusic();

    const auto gc = new eGameChooser(this);
    gc->resize(width(), height());
    setWidget(gc);

    const auto play = [this](const bool poseidon) {
        setPoseidon(poseidon);
        showRosterOfLeaders();
    };
    gc->initialize([play]() { play(false); },
                   [play]() { play(true); },
                   [this]() { showChooseGameEditMenu(); },
                   [this]() { showSettingsMenu(); },
                   [this]() { mQuit = true; });
}

void eMainWindow::showMainMenu() {
    mCampaign = nullptr;
    clearWidgets();
    eMusic::playMenuMusic();

    const auto mm = new eMainMenu(this);
    mm->resize(width(), height());
    setWidget(mm);

    // Resume Adventure loads the leader's most recent save, if any
    std::string latest;
    {
        std::error_code ec;
        const auto dir = leaderSaveDir();
        std::filesystem::file_time_type latestTime;
        for(const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if(!entry.is_regular_file(ec)) continue;
            if(entry.path().extension() != ".ez") continue;
            const auto t = entry.last_write_time(ec);
            if(latest.empty() || t > latestTime) {
                latestTime = t;
                latest = entry.path().u8string();
            }
        }
    }
    eAction resumeAction;
    if(!latest.empty()) {
        resumeAction = [this, latest]() {
            loadGame(latest);
        };
    }

    const auto loadGameAction = [this, mm]() {
        const auto fw = new eFileWidget(this);
        const auto func = [this](const std::string& path) {
            return loadGame(path);
        };
        const auto closeAct = [mm, fw]() {
            mm->removeWidget(fw);
            fw->deleteLater();
        };
        const auto dir = leaderSaveDir();
        fw->intialize(eLanguage::zeusText(293, 3),
                      dir, func, closeAct);
        mm->addWidget(fw);
        fw->align(eAlignment::center);
    };

    mm->initialize(resumeAction,
                   loadGameAction,
                   [this]() { showChooseGameMenu(false); },
                   [this]() { showChooseGameMenu(true); },
                   [this]() { showRosterOfLeaders(); });
}

void eMainWindow::showSettingsMenu() {
    const auto esm = new eSettingsMenu(mSettings, this);
    esm->resize(width(), height());

    const auto applyA = [this](const eSettings& settings) {
        const bool loadNeeded = settings.fRes != mSettings.fRes ||
                                settings.fRes.uiScale() !=
                                mSettings.fRes.uiScale();
        setResolution(settings.fRes);
        setFullscreen(settings.fFullscreen, settings.fBorderless);
        mSettings = settings;
        mSettings.write();
        if(!mSettings.fTinyTextures &&
           !mSettings.fSmallTextures &&
           !mSettings.fMediumTextures &&
           !mSettings.fLargeTextures) {
            mSettings.fSmallTextures = true;
        }
        eGameTextures::setSettings(mSettings);
        if(loadNeeded) showMenuLoading();
        else showGameChooser();
    };
    const auto fullscrennA = [this](const bool f, const bool b) {
        setFullscreen(f, b);
    };
    esm->initialize(applyA, fullscrennA);
    setWidget(esm);
}

void eMainWindow::showChooseGameMenu(const bool custom) {
    const auto gem = new eChooseGameEditMenu(this);
    gem->resize(width(), height());
    gem->initialize(false, custom);
    setWidget(gem);
}

void eMainWindow::showChooseGameEditMenu() {
    const auto gem = new eChooseGameEditMenu(this);
    gem->resize(width(), height());
    gem->initialize(true);
    setWidget(gem);
}

void eMainWindow::showGame(const stdsptr<eCampaign>& c,
                           const eGameWidgetSettings& settings) {
    mCampaign = c;
    const auto e = c->currentEpisode();
    showGame(e->fBoard, settings);
}

void eMainWindow::showGame(eGameBoard* b,
                           const eGameWidgetSettings& settings) {
    if(!b) b = mBoard;

    if(b == mBoard && mGW) {
        return setWidget(mGW);
    }

    if(mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }

    mBoard = b;
    if(mBoard) {
        mBoard->updateAppealMapIfNeeded();
        mBoard->waitUntilFinished();
    }

    // entering the city: the mission narrator stops
    eMusic::stopCampaignVoice();
    eMusic::playRandomMusic();
    mGW = new eGameWidget(this);
    mGW->setBoard(b);
    mGW->resize(width(), height());
    mGW->initialize();
    mGW->setSettings(settings);
    setWidget(mGW);
}

void eMainWindow::showWorld() {
    if(mWidget == mWW) return;
    if(!mCampaign) return;
    if(!mWW) {
        mWW = new eWorldWidget(this);
        mWW->resize(width(), height());
        mWW->initialize();
        mWW->setBoard(mBoard);
    } else {
        mWW->update();
    }
    setWidget(mWW);
}

void eMainWindow::askToQuit() {
    // only on the menu screens; the city and world map use Escape themselves,
    // and quitting mid-load could leave work half done
    if(!mWidget || mWidget == mGW || mWidget == mWW) return;
    if(dynamic_cast<eMenuLoadingWidget*>(mWidget)) return;
    if(dynamic_cast<eGameLoadingWidget*>(mWidget)) return;
    const auto q = new eQuestionWidget(this);
    // the original's own question, "Exit Game?" (Zeus_Text group 5, item 1)
    q->initialize(eLanguage::zeusText(5, 1), "",
                  [this]() { mQuit = true; }, nullptr);
    execDialog(q);
    q->align(eAlignment::center);
}

void eMainWindow::execDialog(
        eWidget* const d, const bool closable,
        const eAction &closeFunc,
        eWidget* const parent) {
    if(!mWidget) return;
    const auto bg = new eEventBackground(this);
    if(closeFunc) {
        bg->initialize(parent ? parent : mWidget, d, closable, closeFunc);
    } else {
        const auto closeFunc = [d]() {
            d->deleteLater();
        };
        bg->initialize(parent ? parent : mWidget, d, closable, closeFunc);
    }
}

class eTooltip {
public:
    eTooltip(eMainWindow& w) : mWindow(w) {}

    void update() {
        const auto txt = eWidget::sTooltip();
        const bool updateTxt = mText != txt;
        if(updateTxt) {
            mText = txt;
        }

        const auto& res = mWindow.resolution();
        const int fontSize = res.verySmallFontSize();
        const bool updateFont = mFontSize != fontSize;
        if(updateFont) {
            mFontSize = fontSize;
            mFont = eFonts::defaultFont(mFontSize);
        }

        const bool updateTexture = updateTxt || updateFont;
        if(updateTexture) {
            const auto r = mWindow.renderer();
            if(mText.empty()) {
                mTexture->reset();
            } else {
                mTexture->loadText(r, mText, eFontColor::light, *mFont, 50*fontSize);
            }
        }
    }

    void paint(const int x, const int y, ePainter& p) {
        const int pp = padding();
        SDL_Rect rect{x, y, width(), height()};
        p.fillRect(rect, SDL_Color{16, 108, 144, 255});
        p.drawRect(rect, SDL_Color{0, 32, 32, 255}, 1);
        p.drawTexture(x + pp, y + pp, mTexture);
    }

    int width() const { return mTexture->width() + 2*padding(); }
    int height() const { return mTexture->height() + 2*padding(); }

    bool empty() const { return mText.empty(); }
private:
    int padding() const { return mFontSize/2; }

    eMainWindow& mWindow;
    int mFontSize = -1;
    TTF_Font* mFont = nullptr;
    std::string mText;
    stdsptr<eTexture> mTexture = std::make_shared<eTexture>();
};

int eMainWindow::exec() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    showMenuLoading();

    eMouseButton button{eMouseButton::none};
    eMouseButton buttons{eMouseButton::none};

    SDL_Event e;
    eTooltip tooltip(*this);

    const bool showFPS = false;
    const double fpsClamp = 20;

    int c = 0;
    int fpsVal = 0;
    bool resetRenderTargets = false;
    while(!mQuit) {
        const auto fpsStart = high_resolution_clock::now();

        while(SDL_PollEvent(&e)) {
            int x, y;
            mousePosition(x, y);
            const bool shift = mShiftPressed > 0;
            const bool ctrl = mCtrlPressed > 0;
            if(e.type == SDL_QUIT) {
                mQuit = true;
            } else if(e.type == SDL_WINDOWEVENT) {
                const auto we = e.window.event;
                if(we == SDL_WINDOWEVENT_MINIMIZED) {
                    resetRenderTargets = true;
                    while(SDL_WaitEvent(&e)) {
                        if(e.window.event == SDL_WINDOWEVENT_RESTORED) {
                            break;
                        }
                    }
                } else if(we == SDL_WINDOWEVENT_EXPOSED) {
                    resetRenderTargets = true;
                }
            } else if(e.type == SDL_RENDER_TARGETS_RESET ||
                      e.type == SDL_RENDER_DEVICE_RESET) {
                resetRenderTargets = true;
            } else if(e.type == SDL_MOUSEMOTION) {
                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mouseMove(me);
            } else if(e.type == SDL_MOUSEBUTTONDOWN) {
                switch(e.button.button) {
                case SDL_BUTTON_LEFT:
                    button = eMouseButton::left;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = eMouseButton::right;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = eMouseButton::middle;
                    break;
                default: continue;
                }
                buttons = button | buttons;

                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mousePress(me);
            } else if(e.type == SDL_MOUSEBUTTONUP) {
                switch(e.button.button) {
                case SDL_BUTTON_LEFT:
                    button = eMouseButton::left;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = eMouseButton::right;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = eMouseButton::middle;
                    break;
                default: continue;
                }
                buttons = buttons & ~button;
                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mouseRelease(me);
            } else if(e.type == SDL_MOUSEWHEEL) {
                const eMouseWheelEvent me(x, y, shift, ctrl, buttons, e.wheel.y);
                if(mWidget) mWidget->mouseWheel(me);
            } else if(e.type == SDL_KEYDOWN) {
                const auto k = e.key.keysym.scancode;
                if(k == SDL_Scancode::SDL_SCANCODE_LSHIFT ||
                   k == SDL_Scancode::SDL_SCANCODE_RSHIFT) {
                    mShiftPressed++;
                } else if(k == SDL_Scancode::SDL_SCANCODE_LCTRL ||
                          k == SDL_Scancode::SDL_SCANCODE_RCTRL) {
                    mCtrlPressed++;
                }
                const eKeyPressEvent ke(x, y, shift, ctrl, buttons, k);
                const bool handled = mWidget && mWidget->keyPress(ke);
                if(!handled && k == SDL_Scancode::SDL_SCANCODE_ESCAPE) {
                    askToQuit();
                }
            } else if(e.type == SDL_KEYUP) {
                const auto k = e.key.keysym.scancode;
                if(k == SDL_Scancode::SDL_SCANCODE_LSHIFT ||
                   k == SDL_Scancode::SDL_SCANCODE_RSHIFT) {
                    mShiftPressed--;
                } else if(k == SDL_Scancode::SDL_SCANCODE_LCTRL ||
                          k == SDL_Scancode::SDL_SCANCODE_RCTRL) {
                    mCtrlPressed--;
                }
            }
        }

        if(resetRenderTargets) {
            resetRenderTargets = false;
            if(mWidget) mWidget->renderTargetsReset();
        }

        SDL_SetRenderDrawColor(mSdlRenderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderClear(mSdlRenderer);

        ePainter p(mSdlRenderer);

        eMusic::incTime();
        if(mWidget) {
            mWidget->paint(p);
            tooltip.update();
            if(!tooltip.empty()) {
                const auto& res = resolution();
                const int pp = 25*res.multiplier();
                const int wtt = tooltip.width();
                const int htt = tooltip.height();
                int mx, my;
                mousePosition(mx, my);
                int xtt;
                int ytt;
                if(mx > width()/2) {
                    xtt = mx - wtt;
                } else {
                    xtt = mx;
                }
                if(my > height()/2) {
                    ytt = my - htt - pp;
                } else {
                    ytt = my + pp;
                }
                tooltip.paint(xtt, ytt, p);
            }
        }

        if(showFPS) {
            p.setFont(eFonts::defaultFont(resolution()));
            p.drawText(0, 0, std::to_string(fpsVal), eFontColor::dark);
        }

        SDL_RenderPresent(mSdlRenderer);

        for(const auto& s : mSlots) {
            s();
        }
        mSlots.clear();

        const auto fpsEnd = high_resolution_clock::now();
        const duration<double, std::milli> fpsElapsed = fpsEnd - fpsStart;
        const duration<double, std::milli> fpsDuration(1000./fpsClamp);
        const duration<double, std::milli> fpsSleep(fpsDuration - fpsElapsed);
        std::this_thread::sleep_for(fpsSleep);

        if(showFPS) {
            c++;
            if(c % 25 == 0) {
                const auto fpsEnd = high_resolution_clock::now();
                const duration<double, std::milli> fpsElapsed = fpsEnd - fpsStart;
                fpsVal = (int)std::round(1000./fpsElapsed.count());
            }
        }
    }

    return 0;
}

void eMainWindow::saveOptions(const eSettings& s) {
    mSettings.fGameSpeed = s.fGameSpeed;
    mSettings.fMusicOn = s.fMusicOn;
    mSettings.fSpeechOn = s.fSpeechOn;
    mSettings.fSoundsOn = s.fSoundsOn;
    mSettings.fAmbientOn = s.fAmbientOn;
    mSettings.fMusicVolume = s.fMusicVolume;
    mSettings.fSpeechVolume = s.fSpeechVolume;
    mSettings.fSoundsVolume = s.fSoundsVolume;
    mSettings.fAmbientVolume = s.fAmbientVolume;
    mSettings.applyAudio();
    mSettings.write();
}
