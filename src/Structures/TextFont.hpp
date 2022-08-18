#ifndef TEXTFONT_H
#define TEXTFONT_H


#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <nameof.hpp>

#include "../General.hpp"


struct TextFont
{
    int32_t height;
    int32_t width;

    uint16_t escapement;

    uint16_t weight;

    uint16_t italic;

    std::string fontName;
};


[[maybe_unused]]
static std::string to_string(const TextFont& aObj)
{
    std::string str;

    str += fmt::format("{}:\n", nameof::nameof_type<decltype(aObj)>());
    str += fmt::format("{}height = {}\n", indent(1), aObj.height);
    str += fmt::format("{}width  = {}\n", indent(1), aObj.width);
    str += fmt::format("{}escapement = {}\n", indent(1), aObj.escapement);
    str += fmt::format("{}weight = {}\n", indent(1), aObj.weight);
    str += fmt::format("{}italic = {}\n", indent(1), aObj.italic);
    str += fmt::format("{}fontName = {}\n", indent(1), aObj.fontName);

    return str;
}


[[maybe_unused]]
static std::ostream& operator<<(std::ostream& aOs, const TextFont& aVal)
{
    aOs << to_string(aVal);

    return aOs;
}


#endif // TEXTFONT_H