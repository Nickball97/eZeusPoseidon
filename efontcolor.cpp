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
        // the original's carved look (measured in its windows): navy
        // letters with a light-blue edge one pixel below and to the right
        // (eTexture::loadText draws the edge under the letters)
        col1 = SDL_Color{16, 65, 99, 255};
        col2 = SDL_Color{140, 211, 247, 255};
        break;
    case eFontColor::yellow:
        col1 = SDL_Color{255, 207, 0, 255};
        col2 = SDL_Color{0, 0, 0, 255};
        break;
    case eFontColor::region:
        col1 = SDL_Color{168, 96, 80, 255};
        col2 = SDL_Color{0, 0, 0, 255};
        break;
    case eFontColor::darkOnDark:
        // measured in the original's inner boxes (the Estate's occupants
        // box): deeper navy letters, a mid-blue edge below-right
        col1 = SDL_Color{0, 16, 82, 255};
        col2 = SDL_Color{66, 162, 206, 255};
        break;
    case eFontColor::panel:
        // measured in the original's side panel box: pale letters with
        // a dark navy edge up-left
        col1 = SDL_Color{189, 211, 222, 255};
        col2 = SDL_Color{0, 40, 66, 255};
        break;
    }
}
