#ifndef BITMAP_H
#define BITMAP_H


#include <cstdint>
#include <ostream>
#include <string>

#include <fmt/core.h>
#include <nameof.hpp>

#include "../General.hpp"


// @todo update Kaitai file
struct Bitmap
{
    void writeBmpToFile(const std::string& aFilePath) const;

    int32_t locX;
    int32_t locY;

    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;

    uint32_t bmpWidth;
    uint32_t bmpHeight;

    // @todo Add better Bitmap support and figure out how to
    //       create a *.bmp file out of the rawImgData
    // @note Looks like the XSD uses Base64 encoding for the data, but I was not
    //       able to extract the correct content from there. Are there some parameters
    //       to adjust in the decoding? https://cryptii.com/pipes/base64-to-binary
    std::vector<uint8_t> rawImgData; // @todo called val in the XSD file
};


[[maybe_unused]]
static std::string to_string(const Bitmap& aObj)
{
    std::string str;

    str += fmt::format("{}:\n", nameof::nameof_type<decltype(aObj)>());
    str += fmt::format("{}locX = {}\n", indent(1), aObj.locX);
    str += fmt::format("{}locY = {}\n", indent(1), aObj.locY);
    str += fmt::format("{}x1   = {}\n", indent(1), aObj.x1);
    str += fmt::format("{}y1   = {}\n", indent(1), aObj.y1);
    str += fmt::format("{}x2   = {}\n", indent(1), aObj.x2);
    str += fmt::format("{}y2   = {}\n", indent(1), aObj.y2);
    str += fmt::format("{}bmpWidth  = {}\n", indent(1), aObj.bmpWidth);
    str += fmt::format("{}bmpHeight = {}\n", indent(1), aObj.bmpHeight);

    // @todo Should we print rawImgData somehow?

    return str;
}


[[maybe_unused]]
static std::ostream& operator<<(std::ostream& aOs, const Bitmap& aVal)
{
    aOs << to_string(aVal);

    return aOs;
}


#endif // BITMAP_H