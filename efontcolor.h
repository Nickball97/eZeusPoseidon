#ifndef EFONTCOLOR_H
#define EFONTCOLOR_H

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

enum class eFontColor {
    light, dark, yellow, region,
    // the original's styles on its darker blue boxes: carved text in the
    // inner boxes of windows, pale text in the side panel's box
    darkOnDark, panel
};

namespace eFontColorHelpers {
    void colors(const eFontColor c,
                SDL_Color& col1,
                SDL_Color& col2);
};

#endif // EFONTCOLOR_H
