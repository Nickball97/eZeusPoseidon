#ifndef EMUSIC_H
#define EMUSIC_H

#include "emusicvector.h"

#include <memory>
#include <map>

enum class eMusicType {
    none, setup, music, battle
};

class eMusic {
public:
    eMusic();

    static void loadMenu();
    static void load();
    static bool loaded();

    static void incTime();

    static void playMenuMusic();
    static void playRandomMusic();
    static void playRandomBattleMusic();
    static void playMissionIntroMusic();
    static void playMissionVictoryMusic();
    static void playCampaignVictoryMusic();

    // the narrator of mission intros and victories; it plays on its own
    // sound channel so the music goes on under it, as in the original
    // (zeus.exe 0x005A87A0 and 0x005A88C0 start the voice and then
    // mission_intro.wav / mission_victory.wav together)
    static bool playCampaignVoice(const std::string& path);
    static void stopCampaignVoice();
    // sound channel kept for the narrator (reserved in main.cpp)
    static const int sVoiceChannel = 0;
private:
    void incTimeImpl();

    void playMenuMusicImpl();
    void playRandomMusicImpl();
    void playRandomBattleMusicImpl();
    void playMissionIntroMusicImpl();
    void playMissionVictoryMusicImpl();
    void playCampaignVictoryMusicImpl();
    bool playCampaignVoiceImpl(const std::string& path);
    void stopCampaignVoiceImpl();

    void loadImpl();
    void loadMenuImpl();
    static eMusic* sInstance;

    bool mLoaded{false};
    bool mMenuLoaded{false};
    eMusicType mMusicType{eMusicType::none};

    Mix_Chunk* mCampaignVoice = nullptr;

    eMusicVector mSetupMusic;
    eMusicVector mMusic;
    eMusicVector mBattleMusic;
    eMusicVector mMissionIntro;
    eMusicVector mMissionVictory;
    eMusicVector mCampaignVictory;
};

#endif // EMUSIC_H
