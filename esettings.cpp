#include "esettings.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "egamedir.h"
#include "eloadtexthelper.h"

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

