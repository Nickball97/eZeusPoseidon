#ifndef ESETTINGS_H
#define ESETTINGS_H

#include "widgets/eresolution.h"
#include "engine/etile.h"

struct eSettings {
    bool fTinyTextures = true;
    bool fSmallTextures = true;
    bool fMediumTextures = true;
    bool fLargeTextures = true;
    bool fFullscreen = false;
    // full screen as a borderless window the size of the desktop, so
    // switching to another program doesn't drop the game to the desktop
    bool fBorderless = false;
    eResolution fRes = eResolution(1280, 720);
    // interface size: 0 automatic (from the screen height), 1 = 1x,
    // 2 = 1.5x, 3 = 2x; the art exists only at these sizes
    int fInterfaceSize = 0;

    // Options menu, as in the original (zeus.exe 0x005C44C0 defaults, saved
    // like its Zeus.inf, not per city). Game speed 10-100 in steps of 10.
    int fGameSpeed = 90;
    bool fMusicOn = true;
    bool fSpeechOn = true;
    bool fSoundsOn = true;
    bool fAmbientOn = true;
    int fMusicVolume = 50;
    int fSpeechVolume = 100;
    int fSoundsVolume = 80;
    int fAmbientVolume = 80;
    // Options > Messages: per message category (text group 174) 0 off,
    // 1 condensed, 2 full; all full by default (Model/Zeus event
    // message categories.txt), saved like the original's Zeus.inf
    std::string fMessageSettings = std::string(17, '2');
    int messageSetting(const int category) const;

    // copies the sound options to the audio code and applies them
    void applyAudio() const;

    // gives fRes the chosen interface size, or the automatic one when the
    // chosen size doesn't fit the screen height
    void applyInterfaceSize();

    std::vector<eTileSize> availableSizes() const;

    void write() const;
    void read();
};

#endif // ESETTINGS_H
