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

    std::vector<eTileSize> availableSizes() const;

    void write() const;
    void read();
};

#endif // ESETTINGS_H
