#ifndef EMAINWINDOW_H
#define EMAINWINDOW_H

#include "widgets/ewidget.h"
#include "widgets/eresolution.h"
#include "textures/eterraintextures.h"
#include "textures/egodtextures.h"
#include "textures/ebuildingtextures.h"
#include "textures/echaractertextures.h"
#include "widgets/esettingsmenu.h"

using eSlot = std::function<void()>;

class eGameBoard;
class eGameWidget;
class eWorldWidget;
class eCampaign;

struct eGameWidgetSettings;

class eMainWindow {
public:
    eMainWindow();
    ~eMainWindow();

    bool initialize(const eSettings& settings);
public:

    void setWidget(eWidget* const w);
    eWidget* takeWidget();

    int exec();

    void addSlot(const eSlot& slot);

    int width() const { return resolution().width(); }
    int height() const { return resolution().height(); }
    const eResolution& resolution() const { return mSettings.fRes; }
    SDL_Window* window() const { return mSdlWindow; }
    SDL_Renderer* renderer() const { return mSdlRenderer; }

    void setResolution(const eResolution& res);
    void setFullscreen(const bool f);
    // borderless: full screen as a desktop-sized window (SDL "desktop"
    // full screen); the game is drawn at the chosen resolution and
    // stretched to the desktop if the two differ
    void setFullscreen(const bool f, const bool borderless);
    // mouse position and warping in game pixels, also when the picture is
    // stretched in borderless full screen
    void mousePosition(int& x, int& y) const;
    void warpMouse(const int x, const int y);
    // F5 and F8 in the game (zeus.exe window procedure 0x005C4A90)
    void toggleFullscreen();
    void centerWindow();

    void startGameAction(eGameBoard* const board,
                         const eGameWidgetSettings& settings);
    void startGameAction(const stdsptr<eCampaign>& c,
                         const eGameWidgetSettings& settings);
    void startGameAction(const eAction& a);
    void episodeFinished();
    void adventureComplete();
    void episodeLost();

    bool saveGame(const std::string& path);
    bool loadGame(const std::string& path);
    void closeGame();

    // home page, as in the original: game chooser -> roster of leaders ->
    // player menu (showMainMenu) -> adventure lists
    void showGameChooser();
    void showRosterOfLeaders();
    void showMenuLoading();
    void showMainMenu();
    void showSettingsMenu();
    void showChooseGameMenu(const bool custom = false);
    void showChooseGameEditMenu();
    void showGame(const stdsptr<eCampaign>& c,
                  const eGameWidgetSettings& settings);
    void showGame(eGameBoard* b,
                  const eGameWidgetSettings& settings);
    void showWorld();

    eWidget* currentWidget() const { return mWidget; }
    eWorldWidget* worldWidget() const { return mWW; }

    const eSettings& settings() const { return mSettings; }

    void execDialog(eWidget* const d,
                    const bool closable = true,
                    const eAction& closeFunc = nullptr,
                    eWidget* const parent = nullptr);

    void showEpisodeIntroduction(const stdsptr<eCampaign>& c = nullptr);
    const stdsptr<eCampaign>& campaign() const { return mCampaign; }

    const std::string& leader() const { return mLeader; }
    void setLeader(const std::string& leader) { mLeader = leader; }
    std::string leaderSaveDir() const;
    // Zeus or Poseidon picked on the first screen (logos, roster picture
    // and which original adventures are listed)
    bool poseidon() const { return mPoseidon; }
    void setPoseidon(const bool p) { mPoseidon = p; }
private:
    void updateLogicalSize();
    void clearWidgets();
    // Escape on a menu screen: ask "Exit game?" before quitting
    void askToQuit();

    eSettings mSettings;

    std::string mLeader;
    bool mPoseidon = false;

    bool mQuit = false;
    bool mFirstFullscrenSetting = true;
    bool mFirstResolutionSetting = true;

    std::vector<eSlot> mSlots;

    int mShiftPressed = 0;
    int mCtrlPressed = 0;

    stdsptr<eCampaign> mCampaign;
    eGameBoard* mBoard = nullptr;
    eGameWidget* mGW = nullptr;
    eWorldWidget* mWW = nullptr;

    eWidget* mWidget = nullptr;
    SDL_Window* mSdlWindow = nullptr;
    SDL_Renderer* mSdlRenderer = nullptr;
};

#endif // EMAINWINDOW_H
