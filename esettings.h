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

    // gives fRes the chosen interface size, or the automatic one when the
    // chosen size doesn't fit the screen height
    void applyInterfaceSize();

    std::vector<eTileSize> availableSizes() const;

    void write() const;
    void read();
};

#endif // ESETTINGS_H
