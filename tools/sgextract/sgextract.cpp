// sgextract: cuts sprite frames out of the original Zeus/Poseidon .sg3/.555
// files and writes them as eZeus sprite sheets, so eZeus can show art that
// is not in its packed i15/i30/i45/i60 archives.
//
// The original files belong to the player; this tool reads them from the
// player's own game folder and writes the sheets there. Never commit the
// output images.
//
// Usage:
//   sgextract <file.sg3> <first> <last> <name> <textures dir> [header dir]
// Writes <textures dir>/<15|30|45|60>/<name>_<sheet>.png for frames
// first..last (inclusive) and, if a header dir is given,
// <header dir>/<name><size>.h with the eSpriteData rectangles in the same
// layout eZeus's spriteData headers use (one entry per frame, -1 for frames
// that are mirrored copies of another frame).
//
// SG3 layout (version 0xd6, as used by Zeus and Poseidon):
//   header 680 bytes (image count at 16, bitmap count at 20),
//   200 bitmap records of 200 bytes, then 72-byte image records starting
//   with a dummy record 0. Image record: offset 0, length 4, invert offset
//   16, width 20, height 22, type 50, flags 52 (external), alpha offset 64,
//   alpha length 68. Sprite images (type 256/257/276) are run-length
//   coded 16-bit 555 colour: byte 255 = skip <next byte> transparent
//   pixels, otherwise <byte> pixels follow.

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "zlib.h" // from the zlib copy SDL_image brings along (see build.cmd)

namespace fs = std::filesystem;

struct Image {
    int fW = 0;
    int fH = 0;
    std::vector<uint8_t> fRGBA; // fW*fH*4
};

static uint32_t u32(const std::vector<uint8_t>& b, size_t o) {
    return b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (uint32_t(b[o + 3]) << 24);
}
static int16_t i16(const std::vector<uint8_t>& b, size_t o) {
    return int16_t(b[o] | (b[o + 1] << 8));
}
static uint16_t u16(const std::vector<uint8_t>& b, size_t o) {
    return uint16_t(b[o] | (b[o + 1] << 8));
}

static bool readFile(const fs::path& p, std::vector<uint8_t>& out) {
    std::ifstream f(p, std::ios::binary);
    if(!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), {});
    return true;
}

// ---- PNG writer ----
static uint32_t crcTable[256];
static void initCrc() {
    for(uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for(int k = 0; k < 8; k++) c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
        crcTable[n] = c;
    }
}
static uint32_t crc(const uint8_t* d, size_t n, uint32_t c = 0xffffffffu) {
    for(size_t i = 0; i < n; i++) c = crcTable[(c ^ d[i]) & 0xff] ^ (c >> 8);
    return c;
}
static void put32(std::vector<uint8_t>& v, uint32_t x) {
    v.push_back(x >> 24); v.push_back(x >> 16); v.push_back(x >> 8); v.push_back(x);
}
static void chunk(std::vector<uint8_t>& png, const char* type,
                  const std::vector<uint8_t>& data) {
    put32(png, uint32_t(data.size()));
    std::vector<uint8_t> td(type, type + 4);
    td.insert(td.end(), data.begin(), data.end());
    png.insert(png.end(), td.begin(), td.end());
    put32(png, crc(td.data(), td.size()) ^ 0xffffffffu);
}
static bool writePng(const fs::path& p, const Image& img) {
    std::vector<uint8_t> raw;
    raw.reserve((img.fW * 4 + 1) * img.fH);
    for(int y = 0; y < img.fH; y++) {
        raw.push_back(0);
        const auto row = img.fRGBA.data() + y * img.fW * 4;
        raw.insert(raw.end(), row, row + img.fW * 4);
    }
    uLongf zLen = compressBound(uLong(raw.size()));
    std::vector<uint8_t> z(zLen);
    if(compress2(z.data(), &zLen, raw.data(), uLong(raw.size()), 9) != Z_OK) {
        return false;
    }
    z.resize(zLen);

    std::vector<uint8_t> png{0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
    std::vector<uint8_t> ihdr;
    put32(ihdr, img.fW); put32(ihdr, img.fH);
    ihdr.insert(ihdr.end(), {8, 6, 0, 0, 0}); // 8-bit RGBA
    chunk(png, "IHDR", ihdr);
    chunk(png, "IDAT", z);
    chunk(png, "IEND", {});
    std::ofstream f(p, std::ios::binary);
    if(!f) return false;
    f.write(reinterpret_cast<const char*>(png.data()), png.size());
    return bool(f);
}

// ---- decoding ----
static void put555(Image& img, int x, int y, uint16_t c) {
    if(x < 0 || y < 0 || x >= img.fW || y >= img.fH) return;
    auto p = &img.fRGBA[(y * img.fW + x) * 4];
    const int r = (c >> 10) & 0x1f, g = (c >> 5) & 0x1f, b = c & 0x1f;
    p[0] = uint8_t((r << 3) | (r >> 2));
    p[1] = uint8_t((g << 3) | (g >> 2));
    p[2] = uint8_t((b << 3) | (b >> 2));
    p[3] = 255;
}

static bool decodeSprite(const uint8_t* d, size_t len, Image& img) {
    size_t i = 0;
    int x = 0, y = 0;
    while(i < len && y < img.fH) {
        const uint8_t c = d[i++];
        if(c == 255) {
            if(i >= len) break;
            x += d[i++];
        } else {
            for(int j = 0; j < c && i + 1 < len; j++, i += 2) {
                put555(img, x++, y, uint16_t(d[i] | (d[i + 1] << 8)));
            }
        }
        while(x >= img.fW && img.fW > 0) { x -= img.fW; y++; }
    }
    return true;
}

static void decodeAlpha(const uint8_t* d, size_t len, Image& img) {
    // same run-length layout, one 5-bit alpha value per pixel
    size_t i = 0;
    int x = 0, y = 0;
    while(i < len && y < img.fH) {
        const uint8_t c = d[i++];
        if(c == 255) {
            if(i >= len) break;
            x += d[i++];
        } else {
            for(int j = 0; j < c && i < len; j++, i++) {
                if(x < img.fW && y < img.fH) {
                    const int a = d[i] & 0x1f;
                    img.fRGBA[(y * img.fW + x) * 4 + 3] = uint8_t((a << 3) | (a >> 2));
                }
                x++;
            }
        }
        while(x >= img.fW && img.fW > 0) { x -= img.fW; y++; }
    }
}

static void decodePlain(const uint8_t* d, size_t len, Image& img) {
    for(int y = 0; y < img.fH; y++) {
        for(int x = 0; x < img.fW; x++) {
            const size_t o = size_t(y * img.fW + x) * 2;
            if(o + 1 >= len) return;
            const uint16_t c = uint16_t(d[o] | (d[o + 1] << 8));
            if(c == 0xf81f) continue; // transparent
            put555(img, x, y, c);
        }
    }
}

// Isometric building image (type 30): a diamond footprint of size x size
// floor tiles (58x30 each, 1800 bytes of raw 555 pixels, tiles 60 pixels
// apart), stored row by row from the top of the diamond, at the bottom of
// the image; then the part above the floor, run-length coded like a
// sprite over the whole image.
static void decodeIsoTile(const uint8_t* d, Image& img, int ox, int oy) {
    const int w = 58, h = 30;
    size_t i = 0;
    for(int y = 0; y < h; y++) {
        const int start = y < h/2 ? h - 2*(y + 1) : 2*y - h;
        const int end = w - start;
        for(int x = start; x < end; x++, i += 2) {
            put555(img, ox + x, oy + y, uint16_t(d[i] | (d[i + 1] << 8)));
        }
    }
}

static void decodeIsometric(const uint8_t* d, size_t len, size_t uLen,
                            int size, Image& img) {
    const int tileBytes = 1800;
    const int baseH = (img.fW + 2)/2;
    if(size <= 0) size = baseH/30;
    if(size <= 0) return;
    int yOff = img.fH - baseH;
    size_t i = 0;
    for(int y = 0; y < 2*size - 1; y++) {
        int xOff = (y < size ? size - y - 1 : y - size + 1)*30;
        const int n = y < size ? y + 1 : 2*size - y - 1;
        for(int x = 0; x < n; x++, i++) {
            if((i + 1)*tileBytes > uLen || (i + 1)*tileBytes > len) return;
            decodeIsoTile(d + i*tileBytes, img, xOff, yOff);
            xOff += 60;
        }
        yOff += 15;
    }
    if(len > uLen) decodeSprite(d + uLen, len - uLen, img);
}

// round half up, as eZeus's own 15/45 sheets do
static int scaled(int v, double s) { return int(std::floor(v * s + 0.5)); }

static Image scale(const Image& src, double s) {
    Image dst;
    dst.fW = std::max(1, scaled(src.fW, s));
    dst.fH = std::max(1, scaled(src.fH, s));
    dst.fRGBA.assign(size_t(dst.fW) * dst.fH * 4, 0);
    if(s == 1) { dst.fRGBA = src.fRGBA; return dst; }
    for(int y = 0; y < dst.fH; y++) {
        for(int x = 0; x < dst.fW; x++) {
            // area average over the source pixels this pixel covers
            const double x0 = x / s, x1 = (x + 1) / s;
            const double y0 = y / s, y1 = (y + 1) / s;
            double acc[4] = {0, 0, 0, 0}, wsum = 0;
            for(int sy = int(y0); sy < std::min<double>(src.fH, std::ceil(y1)); sy++) {
                const double wy = std::min<double>(sy + 1, y1) - std::max<double>(sy, y0);
                if(wy <= 0) continue;
                for(int sx = int(x0); sx < std::min<double>(src.fW, std::ceil(x1)); sx++) {
                    const double wx = std::min<double>(sx + 1, x1) - std::max<double>(sx, x0);
                    if(wx <= 0) continue;
                    const double w = wx * wy;
                    const auto p = &src.fRGBA[(sy * src.fW + sx) * 4];
                    const double a = p[3] / 255.;
                    acc[0] += p[0] * a * w; acc[1] += p[1] * a * w;
                    acc[2] += p[2] * a * w; acc[3] += a * w;
                    wsum += w;
                }
            }
            auto q = &dst.fRGBA[(y * dst.fW + x) * 4];
            if(wsum > 0 && acc[3] > 0) {
                q[0] = uint8_t(std::clamp(acc[0] / acc[3], 0., 255.));
                q[1] = uint8_t(std::clamp(acc[1] / acc[3], 0., 255.));
                q[2] = uint8_t(std::clamp(acc[2] / acc[3], 0., 255.));
                q[3] = uint8_t(std::clamp(255. * acc[3] / wsum, 0., 255.));
            }
        }
    }
    return dst;
}

struct Rect { int fTex = -1, fX = 0, fY = 0, fW = 0, fH = 0; };

int main(int argc, char** argv) {
    if(argc < 6) {
        std::printf("usage: sgextract <file.sg3> <first> <last> <name> "
                    "<textures dir> [header dir]\n");
        return 1;
    }
    initCrc();
    const fs::path sgPath = argv[1];
    const int first = std::atoi(argv[2]);
    const int last = std::atoi(argv[3]);
    const std::string name = argv[4];
    const fs::path outDir = argv[5];
    const fs::path hdrDir = argc > 6 ? fs::path(argv[6]) : fs::path();
    if(!hdrDir.empty()) fs::create_directories(hdrDir);

    std::vector<uint8_t> sg, d555;
    if(!readFile(sgPath, sg)) { std::printf("can't read %s\n", argv[1]); return 1; }
    auto p555 = sgPath; p555.replace_extension(".555");
    if(!readFile(p555, d555)) { std::printf("can't read %s\n", p555.string().c_str()); return 1; }
    const uint32_t version = u32(sg, 4);
    const int nImages = int(u32(sg, 16));
    if(version != 0xd5 && version != 0xd6) {
        std::printf("unsupported sg3 version %x\n", version);
        return 1;
    }
    if(first < 1 || last > nImages || first > last) {
        std::printf("frames must be within 1..%d\n", nImages); return 1;
    }
    const size_t recStart = 680 + 200 * 200;
    // 0xd6 records carry an alpha mask offset and length (8 more bytes)
    const size_t recSize = version == 0xd6 ? 72 : 64;

    // decode at original size
    std::vector<Image> frames;
    std::vector<bool> mirrored;
    for(int i = first; i <= last; i++) {
        const size_t r = recStart + recSize * i;
        Image img;
        img.fW = i16(sg, r + 20);
        img.fH = i16(sg, r + 22);
        const int inv = int32_t(u32(sg, r + 16));
        const bool mir = inv != 0;
        mirrored.push_back(mir);
        if(!mir && img.fW > 0 && img.fH > 0) {
            img.fRGBA.assign(size_t(img.fW) * img.fH * 4, 0);
            const uint32_t off = u32(sg, r);
            const uint32_t len = u32(sg, r + 4);
            const uint16_t type = u16(sg, r + 50);
            const uint8_t ext = sg[r + 52];
            const uint32_t aLen = version == 0xd6 ? u32(sg, r + 68) : 0;
            const uint32_t uLen = u32(sg, r + 8); // isometric footprint bytes
            const int isoSize = sg[r + 55];
            if(ext) {
                std::printf("frame %d is in an external file, skipped\n", i);
            } else if(off + len + aLen <= d555.size()) {
                const uint8_t* dp = d555.data() + off;
                if(type == 256 || type == 257 || type == 276) {
                    decodeSprite(dp, len, img);
                    if(aLen > 0) decodeAlpha(dp + len, aLen, img);
                } else if(type == 0 || type == 1 || type == 10 || type == 12 || type == 13) {
                    decodePlain(dp, len, img);
                } else if(type == 30) {
                    decodeIsometric(dp, len, uLen, isoSize, img);
                } else {
                    std::printf("frame %d has unsupported type %d, left empty\n", i, type);
                }
            }
        }
        frames.push_back(std::move(img));
    }

    const int sizes[4] = {15, 30, 45, 60};
    const double scales[4] = {0.5, 1, 1.5, 2};
    const int maxW = 4096;
    for(int si = 0; si < 4; si++) {
        const double s = scales[si];
        std::vector<Rect> rects(frames.size());
        std::vector<Image> sheets;
        std::vector<Image> scaledFrames;
        for(const auto& f : frames) scaledFrames.push_back(f.fRGBA.empty() ? Image{} : scale(f, s));
        // shelf packing into sheets up to maxW x maxW
        int tex = 0, x = 0, y = 0, rowH = 0, sheetH = 0;
        std::vector<std::pair<int, int>> sheetSize{{0, 0}};
        for(size_t k = 0; k < frames.size(); k++) {
            const auto& f = scaledFrames[k];
            if(mirrored[k] || f.fRGBA.empty()) continue;
            if(x + f.fW > maxW) { x = 0; y += rowH; rowH = 0; }
            if(y + f.fH > maxW) { tex++; x = 0; y = 0; rowH = 0; sheetSize.push_back({0, 0}); }
            rects[k] = Rect{tex, x, y, f.fW, f.fH};
            x += f.fW;
            rowH = std::max(rowH, f.fH);
            auto& ss = sheetSize[tex];
            ss.first = std::max(ss.first, x);
            ss.second = std::max(ss.second, y + f.fH);
            sheetH = std::max(sheetH, y + rowH);
        }
        for(const auto& ss : sheetSize) {
            Image sh;
            sh.fW = std::max(1, ss.first);
            sh.fH = std::max(1, ss.second);
            sh.fRGBA.assign(size_t(sh.fW) * sh.fH * 4, 0);
            sheets.push_back(std::move(sh));
        }
        for(size_t k = 0; k < frames.size(); k++) {
            const auto& rc = rects[k];
            if(rc.fTex < 0) continue;
            const auto& f = scaledFrames[k];
            auto& sh = sheets[rc.fTex];
            for(int yy = 0; yy < f.fH; yy++) {
                std::memcpy(&sh.fRGBA[((rc.fY + yy) * sh.fW + rc.fX) * 4],
                            &f.fRGBA[yy * f.fW * 4], size_t(f.fW) * 4);
            }
        }
        const auto dir = outDir / std::to_string(sizes[si]);
        fs::create_directories(dir);
        for(size_t t = 0; t < sheets.size(); t++) {
            const auto p = dir / (name + "_" + std::to_string(t) + ".png");
            if(!writePng(p, sheets[t])) { std::printf("can't write %s\n", p.string().c_str()); return 1; }
        }
        if(!hdrDir.empty()) {
            std::string var = name;
            var[0] = char(std::toupper(var[0]));
            const auto hp = hdrDir / (name + std::to_string(sizes[si]) + ".h");
            std::ofstream h(hp);
            h << "#include <vector>\n\n#include \"espritedata.h\"\n\n"
              << "// generated by tools/sgextract from " << sgPath.filename().string()
              << " frames " << first << "-" << last << "\n"
              << "const auto e" << var << "SpriteData" << sizes[si]
              << " = std::vector<eSpriteData> {\n";
            for(size_t k = 0; k < rects.size(); k++) {
                const auto& rc = rects[k];
                h << "   eSpriteData{" << rc.fTex << ", " << rc.fX << ", " << rc.fY
                  << ", " << rc.fW << ", " << rc.fH << "}, // " << (first + int(k)) << "\n";
            }
            h << "};\n";
        }
    }
    std::printf("extracted %d frames %d-%d as '%s'\n", last - first + 1, first, last, name.c_str());
    return 0;
}
