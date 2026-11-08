#ifndef EBINARYIMAGELOADER_H
#define EBINARYIMAGELOADER_H

#include "etexture.h"

class eBinaryImageLoader {
public:
    static std::shared_ptr<eTexture> load(SDL_Renderer* const r,
                                          const std::string& path);
    // true if the packed i*.e archives contain this image
    static bool has(const std::string& path);
};

#endif // EBINARYIMAGELOADER_H
