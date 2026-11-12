#include "efontcolor.h"

void eFontColorHelpers::colors(const eFontColor c,
                               SDL_Color& col1,
                               SDL_Color& col2) {
    switch(c) {
    case eFontColor::light:
        col1 = SDL_Color{255, 255, 255, 255};
        col2 = SDL_Color{0, 0, 0, 255};
        break;
    case eFontColor::dark:
        // the original's dark navy text (measured in its windows)
        col1 = SDL_Color{8, 65, 115, 255};
        // no light outline: the original's dark text is plain
        col2 = col1;
        break;
    case eFontColor::yellow:
        col1 = SDL_Color{255, 207, 0, 255};
        col2 = SDL_Color{0, 0, 0, 255};
        break;
    case eFontColor::region:
        col1 = SDL_Color{168, 96, 80, 255};
        col2 = SDL_Color{0, 0, 0, 255};
        break;
    }
}
