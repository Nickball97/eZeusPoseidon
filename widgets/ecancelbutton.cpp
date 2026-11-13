#include "ecancelbutton.h"

#include "textures/einterfacetextures.h"
#include "emainwindow.h"

#include <map>
#include <cstdlib>

namespace {
// copy a (possibly atlas) texture into a plain 32-bit surface
SDL_Surface* readPixels(SDL_Renderer* const r,
                        const std::shared_ptr<eTexture>& t) {
    const int w = t->width();
    const int h = t->height();
    if(w <= 0 || h <= 0) return nullptr;
    const auto fmt = SDL_PIXELFORMAT_ARGB8888;
    const auto target = SDL_CreateTexture(r, fmt, SDL_TEXTUREACCESS_TARGET, w, h);
    if(!target) return nullptr;
    const auto old = SDL_GetRenderTarget(r);
    SDL_SetRenderTarget(r, target);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    t->render(r, 0, 0);
    const auto s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, fmt);
    if(s) SDL_RenderReadPixels(r, nullptr, fmt, s->pixels, s->pitch);
    SDL_SetRenderTarget(r, old);
    SDL_DestroyTexture(target);
    return s;
}

bool similar(const Uint32 a, const Uint32 b) {
    for(int sh = 0; sh < 32; sh += 8) {
        const int ca = (a >> sh) & 0xFF;
        const int cb = (b >> sh) & 0xFF;
        if(std::abs(ca - cb) > 24) return false;
    }
    return true;
}

// The original X button art has no yellow glow for hover/press, unlike the
// checkmark. Build one: where the X and the checkmark share the same shell
// (rim and body), take the glowing checkmark frame; elsewhere (the X glyph)
// keep the X's own frame.
std::shared_ptr<eTexture> glowFrame(SDL_Renderer* const r,
                                    const eTextureCollection& accept,
                                    const eTextureCollection& cancel,
                                    const int frame) {
    const auto& c1 = cancel.getTexture(frame);
    const auto a0 = readPixels(r, accept.getTexture(0));
    const auto a1 = readPixels(r, accept.getTexture(frame));
    const auto c0 = readPixels(r, cancel.getTexture(0));
    const auto cf = readPixels(r, c1);
    std::shared_ptr<eTexture> result;
    const bool ok = a0 && a1 && c0 && cf &&
                    a0->w == c0->w && a0->h == c0->h &&
                    a1->w == cf->w && a1->h == cf->h &&
                    a0->w == cf->w && a0->h == cf->h;
    if(ok) {
        const int n = cf->w*cf->h;
        const auto pa0 = static_cast<Uint32*>(a0->pixels);
        const auto pa1 = static_cast<Uint32*>(a1->pixels);
        const auto pc0 = static_cast<Uint32*>(c0->pixels);
        const auto pcf = static_cast<Uint32*>(cf->pixels);
        for(int i = 0; i < n; i++) {
            if(similar(pa0[i], pc0[i])) pcf[i] = pa1[i];
        }
        result = std::make_shared<eTexture>();
        result->setOffset(c1->offsetX(), c1->offsetY());
        // load() takes ownership of the surface
        if(!result->load(r, cf)) result = nullptr;
    } else if(cf) {
        SDL_FreeSurface(cf);
    }
    if(a0) SDL_FreeSurface(a0);
    if(a1) SDL_FreeSurface(a1);
    if(c0) SDL_FreeSurface(c0);
    return result ? result : c1;
}
}

eCancelButton::eCancelButton(eMainWindow* const window) :
    eBasicButton(&eInterfaceTextures::fCancelButton, window) {
    const auto& intrfc = eGameTextures::interface();
    const int iRes = static_cast<int>(resolution().uiScale());
    const auto& texs = intrfc[iRes];
    // built once per interface size
    // (never freed: the textures must not outlive the renderer at exit)
    static auto& sGlow = *new std::map<int, std::pair<std::shared_ptr<eTexture>,
                                                      std::shared_ptr<eTexture>>>;
    auto it = sGlow.find(iRes);
    if(it == sGlow.end()) {
        const auto r = window->renderer();
        const auto hover = glowFrame(r, texs.fAcceptButton, texs.fCancelButton, 1);
        const auto press = glowFrame(r, texs.fAcceptButton, texs.fCancelButton, 2);
        it = sGlow.insert({iRes, {hover, press}}).first;
    }
    setHoverTexture(it->second.first);
    setPressedTexture(it->second.second);
}
