#include "ecursors.h"

#include <SDL2/SDL.h>

#include "egamedir.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {
SDL_Cursor* gArrow = nullptr;
SDL_Cursor* gShovel = nullptr;
eCursors::eCursor gCurrent = eCursors::eCursor::arrow;

#ifdef _WIN32
// a cursor resource: two words of hot spot, then a DIB twice as high as
// the cursor (the colour image on top of the transparency mask)
SDL_Cursor* readCursor(HMODULE m, const int groupId) {
    const auto g = FindResourceA(m, MAKEINTRESOURCEA(groupId),
                                 MAKEINTRESOURCEA(12)); // RT_GROUP_CURSOR
    if(!g) return nullptr;
    const auto gd = static_cast<const BYTE*>(LockResource(LoadResource(m, g)));
    if(!gd || SizeofResource(m, g) < 20) return nullptr;
    // NEWHEADER (6 bytes), then 14-byte entries; the id is the last word
    const WORD id = *reinterpret_cast<const WORD*>(gd + 6 + 12);
    const auto c = FindResourceA(m, MAKEINTRESOURCEA(id),
                                 MAKEINTRESOURCEA(1)); // RT_CURSOR
    if(!c) return nullptr;
    const auto cd = static_cast<const BYTE*>(LockResource(LoadResource(m, c)));
    const DWORD size = SizeofResource(m, c);
    if(!cd || size < 4 + sizeof(BITMAPINFOHEADER)) return nullptr;
    const int hx = *reinterpret_cast<const WORD*>(cd);
    const int hy = *reinterpret_cast<const WORD*>(cd + 2);
    const auto bih = reinterpret_cast<const BITMAPINFOHEADER*>(cd + 4);
    const int w = bih->biWidth;
    const int h = bih->biHeight/2;
    const int bpp = bih->biBitCount;
    if(w <= 0 || h <= 0 || w > 256 || h > 256) return nullptr;
    if(bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32) return nullptr;
    int nColors = 0;
    if(bpp <= 8) nColors = bih->biClrUsed ? bih->biClrUsed : (1 << bpp);
    const auto pal = reinterpret_cast<const RGBQUAD*>(
                         cd + 4 + bih->biSize);
    const BYTE* const xorBits = cd + 4 + bih->biSize + nColors*sizeof(RGBQUAD);
    const int xorStride = ((w*bpp + 31)/32)*4;
    const BYTE* const andBits = xorBits + xorStride*h;
    const int andStride = ((w + 31)/32)*4;
    if(andBits + andStride*h > cd + size) return nullptr;

    const auto s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
                                                  SDL_PIXELFORMAT_ARGB8888);
    if(!s) return nullptr;
    for(int y = 0; y < h; y++) {
        const int srcY = h - 1 - y; // the DIB is stored bottom-up
        const BYTE* const xr = xorBits + srcY*xorStride;
        const BYTE* const ar = andBits + srcY*andStride;
        auto px = reinterpret_cast<Uint32*>(static_cast<Uint8*>(s->pixels) +
                                            y*s->pitch);
        for(int x = 0; x < w; x++) {
            Uint8 r = 0, gg = 0, b = 0;
            if(bpp <= 8) {
                int idx;
                if(bpp == 8) idx = xr[x];
                else if(bpp == 4) idx = (xr[x/2] >> ((1 - x%2)*4)) & 0xF;
                else idx = (xr[x/8] >> (7 - x%8)) & 1;
                if(idx < nColors) {
                    r = pal[idx].rgbRed;
                    gg = pal[idx].rgbGreen;
                    b = pal[idx].rgbBlue;
                }
            } else {
                const BYTE* const p = xr + x*(bpp/8);
                b = p[0]; gg = p[1]; r = p[2];
            }
            const bool transparent = (ar[x/8] >> (7 - x%8)) & 1;
            const Uint8 a = transparent ? 0 : 255;
            px[x] = (Uint32(a) << 24) | (Uint32(r) << 16) |
                    (Uint32(gg) << 8) | Uint32(b);
        }
    }
    const auto cur = SDL_CreateColorCursor(s, hx, hy);
    SDL_FreeSurface(s);
    return cur;
}
#endif
}

void eCursors::load() {
#ifdef _WIN32
    const auto path = eGameDir::path("zeus.exe");
    const auto m = LoadLibraryExA(path.c_str(), nullptr,
                                  LOAD_LIBRARY_AS_DATAFILE |
                                  LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if(!m) {
        printf("Could not read cursors from %s\n", path.c_str());
        return;
    }
    gArrow = readCursor(m, 0x66);
    gShovel = readCursor(m, 0x82);
    FreeLibrary(m);
    if(gArrow) SDL_SetCursor(gArrow);
#endif
}

void eCursors::set(const eCursor c) {
    if(c == gCurrent) return;
    gCurrent = c;
    switch(c) {
    case eCursor::arrow:
        if(gArrow) SDL_SetCursor(gArrow);
        else SDL_SetCursor(SDL_GetDefaultCursor());
        break;
    case eCursor::shovel:
        if(gShovel) SDL_SetCursor(gShovel);
        break;
    }
}
