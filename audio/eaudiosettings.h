#ifndef EAUDIOSETTINGS_H
#define EAUDIOSETTINGS_H

#include <SDL2/SDL_mixer.h>
#include <string>
#include <cctype>

// The original's sound options (zeus.exe dialog 0x0059B5E0, text group 46):
// music, speech, general sounds and ambient sounds, each on or off with a
// volume of 0-100. Defaults from zeus.exe 0x005C44C0: all on; music 50,
// speech 100, general sounds 80, ambient sounds 80.
enum class eSoundKind {
    general, // Audio/Wavs
    speech, // Audio/Voice and the mission narrator
    ambient // Audio/Ambient
};

struct eAudioSettings {
    static inline bool sMusicOn = true;
    static inline bool sSpeechOn = true;
    static inline bool sSoundsOn = true;
    static inline bool sAmbientOn = true;
    static inline int sMusicVolume = 50;
    static inline int sSpeechVolume = 100;
    static inline int sSoundsVolume = 80;
    static inline int sAmbientVolume = 80;

    static int sMix(const int percent) {
        return percent*MIX_MAX_VOLUME/100;
    }

    static bool sOn(const eSoundKind k) {
        switch(k) {
        case eSoundKind::speech: return sSpeechOn;
        case eSoundKind::ambient: return sAmbientOn;
        default: return sSoundsOn;
        }
    }

    static int sVolume(const eSoundKind k) {
        switch(k) {
        case eSoundKind::speech: return sSpeechVolume;
        case eSoundKind::ambient: return sAmbientVolume;
        default: return sSoundsVolume;
        }
    }

    // SDL_mixer volume (0-128) for a sound of this kind, 0 when it is off
    static int sMixVolume(const eSoundKind k) {
        return sOn(k) ? sMix(sVolume(k)) : 0;
    }

    static eSoundKind sKindForPath(const std::string& path) {
        const auto has = [&path](const char* const dir) {
            const std::string d(dir);
            if(path.size() < d.size()) return false;
            for(size_t i = 0; i + d.size() <= path.size(); i++) {
                bool eq = true;
                for(size_t j = 0; j < d.size(); j++) {
                    if(std::tolower(static_cast<unsigned char>(path[i + j])) != d[j]) {
                        eq = false;
                        break;
                    }
                }
                if(eq) return true;
            }
            return false;
        };
        if(has("audio/voice/")) return eSoundKind::speech;
        if(has("audio/ambient/")) return eSoundKind::ambient;
        return eSoundKind::general;
    }

    // music volume applies at once; the others when a sound starts, and
    // the narrator's channel at once
    static void sApply() {
        Mix_VolumeMusic(sMusicOn ? sMix(sMusicVolume) : 0);
        Mix_Volume(0, sMixVolume(eSoundKind::speech)); // narrator channel
    }
};

#endif // EAUDIOSETTINGS_H
