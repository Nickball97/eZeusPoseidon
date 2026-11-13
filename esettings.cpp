#include "esettings.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "egamedir.h"
#include "eloadtexthelper.h"
#include "audio/eaudiosettings.h"

std::vector<eTileSize> eSettings::availableSizes() const {
    std::vector<eTileSize> sizes;
    if(fTinyTextures) {
        sizes.push_back(eTileSize::s15);
    }
    if(fSmallTextures) {
        sizes.push_back(eTileSize::s30);
    }
    if(fMediumTextures) {
        sizes.push_back(eTileSize::s45);
    }
    if(fLargeTextures) {
        sizes.push_back(eTileSize::s60);
    }
    return sizes;
}

void eSettings::write() const {
    const auto path = eGameDir::settingsPath();
    std::ofstream file;
    file.open(path);
    file << "tiny_textures" << " " <<
            (fTinyTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "small_textures" << " " <<
            (fSmallTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "medium_textures" << " " <<
            (fMediumTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "large_textures" << " " <<
            (fLargeTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "fullscreen" << " " <<
            (fFullscreen ? "\"true\"" : "\"false\"") << "\n";
    file << "borderless" << " " <<
            (fBorderless ? "\"true\"" : "\"false\"") << "\n";
    file << "interface_size" << " " << "\"" <<
            std::to_string(fInterfaceSize) << "\"" << "\n";
    const auto wStr = std::to_string(fRes.width());
    file << "width" << " " << "\"" << wStr << "\"" << "\n";
    const auto hStr = std::to_string(fRes.height());
    file << "height" << " " << "\"" << hStr << "\"" << "\n";
    const auto b = [](const bool v) {
        return std::string(v ? "\"true\"" : "\"false\"");
    };
    const auto n = [](const int v) {
        return "\"" + std::to_string(v) + "\"";
    };
    file << "game_speed " << n(fGameSpeed) << "\n";
    file << "music_on " << b(fMusicOn) << "\n";
    file << "speech_on " << b(fSpeechOn) << "\n";
    file << "sounds_on " << b(fSoundsOn) << "\n";
    file << "ambient_on " << b(fAmbientOn) << "\n";
    file << "music_volume " << n(fMusicVolume) << "\n";
    file << "speech_volume " << n(fSpeechVolume) << "\n";
    file << "sounds_volume " << n(fSoundsVolume) << "\n";
    file << "ambient_volume " << n(fAmbientVolume) << "\n";
    file << "message_settings \"" << fMessageSettings << "\"\n";
    file.close();
}

void eSettings::read() {
    const auto path = eGameDir::settingsPath();
    std::map<std::string, std::string> settings;
    const bool r = eLoadTextHelper::load(path, settings);
    if(!r) return;
    fTinyTextures = settings["tiny_textures"] == "true";
    fSmallTextures = settings["small_textures"] == "true";
    fMediumTextures = settings["medium_textures"] == "true";
    fLargeTextures = settings["large_textures"] == "true";
    fFullscreen = settings["fullscreen"] == "true";
    fBorderless = settings["borderless"] == "true"; // missing in old files: off
    const auto widthStr = settings["width"];
    const auto heightStr = settings["height"];
    if(!widthStr.empty() && !heightStr.empty()) {
        const int width = std::stoi(widthStr);
        const int height = std::stoi(heightStr);
        fRes = eResolution(width, height);
    }
    const auto sizeStr = settings["interface_size"]; // missing: automatic
    if(!sizeStr.empty()) {
        fInterfaceSize = std::clamp(std::atoi(sizeStr.c_str()), 0, 3);
    }
    // options missing from older files keep the original's defaults
    const auto readInt = [&settings](const char* const key, int& v,
                                     const int min, const int max) {
        const auto s = settings[key];
        if(!s.empty()) v = std::clamp(std::atoi(s.c_str()), min, max);
    };
    const auto readBool = [&settings](const char* const key, bool& v) {
        const auto s = settings[key];
        if(!s.empty()) v = s == "true";
    };
    readInt("game_speed", fGameSpeed, 10, 100);
    fGameSpeed = 10*((fGameSpeed + 5)/10);
    readBool("music_on", fMusicOn);
    readBool("speech_on", fSpeechOn);
    readBool("sounds_on", fSoundsOn);
    readBool("ambient_on", fAmbientOn);
    readInt("music_volume", fMusicVolume, 0, 100);
    readInt("speech_volume", fSpeechVolume, 0, 100);
    readInt("sounds_volume", fSoundsVolume, 0, 100);
    readInt("ambient_volume", fAmbientVolume, 0, 100);
    {
        const auto s = settings["message_settings"];
        for(int i = 0; i < static_cast<int>(s.size()) && i < 17; i++) {
            const char c = s[i];
            if(c >= '0' && c <= '2') fMessageSettings[i] = c;
        }
    }
    applyInterfaceSize();
}

void eSettings::applyInterfaceSize() {
    const int h = fRes.height();
    auto s = eResolution::sAutoUIScale(h);
    eUIScale chosen = s;
    switch(fInterfaceSize) {
    case 1: chosen = eUIScale::small; break;
    case 2: chosen = eUIScale::medium; break;
    case 3: chosen = eUIScale::large; break;
    default: break;
    }
    if(eResolution::sUIScaleFits(chosen, h)) s = chosen;
    fRes.setUIScale(s);
}


void eSettings::applyAudio() const {
    eAudioSettings::sMusicOn = fMusicOn;
    eAudioSettings::sSpeechOn = fSpeechOn;
    eAudioSettings::sSoundsOn = fSoundsOn;
    eAudioSettings::sAmbientOn = fAmbientOn;
    eAudioSettings::sMusicVolume = fMusicVolume;
    eAudioSettings::sSpeechVolume = fSpeechVolume;
    eAudioSettings::sSoundsVolume = fSoundsVolume;
    eAudioSettings::sAmbientVolume = fAmbientVolume;
    eAudioSettings::sApply();
}

int eSettings::messageSetting(const int category) const {
    if(category < 0 || category >= static_cast<int>(fMessageSettings.size())) {
        return 2; // no category, or win/lose (19): always full
    }
    return fMessageSettings[category] - '0';
}
