#include <cstdint>
#include <ostream>
#include <string>

#include <nameof.hpp>

#include "../General.hpp"
#include "../Parser.hpp"
#include "SymbolDisplayProp.hpp"


SymbolDisplayProp Parser::readSymbolDisplayProp()
{
    spdlog::debug(getOpeningMsg(__func__, mDs.getCurrentOffset()));

    const std::optional<FutureData> thisFuture = getFutureData();

    SymbolDisplayProp obj;

    obj.nameIdx = mDs.readUint32();

    // @todo move to left shift operator
    // @bug The required string is not this one but the value of the associated property!!!!
    //      This is just the name of the property!!
    spdlog::debug("strLst Item = {}", mLibrary.symbolsLibrary.strLst.at(obj.nameIdx - 1));

    obj.x = mDs.readInt16();
    obj.y = mDs.readInt16();

    // @todo maybe using a bitmap is a cleaner solution than shifting bits
    const uint16_t packedStruct = mDs.readUint16();

    obj.textFontIdx = packedStruct & 0xff; // Bit  7 downto  0

    if(obj.textFontIdx >= mLibrary.symbolsLibrary.textFonts.size())
    {
        const std::string msg = fmt::format("{}: textFontIdx is out of range! Expected {} < {}!",
            __func__, obj.textFontIdx, mLibrary.symbolsLibrary.textFonts.size());

        spdlog::error(msg);
        throw std::out_of_range(msg);
    }

    // @todo The meaning of the bits in between is unknown
    spdlog::debug("Unknown bits in bitmap: {}", (packedStruct >> 8u) & 0x3f); // Bit 13 downto  8
    if(((packedStruct >> 8u) & 0x3f) != 0x00)
    {
        throw std::runtime_error("Some bits in the bitmap are used but what is the meaning of them?");
    }

    obj.rotation = ToRotation(packedStruct >> 14u); // Bit 15 downto 14

    obj.propColor = ToColor(mDs.readUint8());

    // Somehow relates to the visiblity of text. See show "Value if Value exist" and the other options
    //        Do not display
    // cc 01  Value only
    // 00 02  Name and value
    // 00 03  Name only
    // 00 04  Both if value exist
    //        Value if value exist
    mDs.printUnknownData(2, std::string(__func__) + " - 0");

    mDs.assumeData({0x00}, std::string(__func__) + " - 1");

    // sanitizeThisFutureSize(thisFuture);

    checkTrailingFuture();

    spdlog::debug(getClosingMsg(__func__, mDs.getCurrentOffset()));
    spdlog::info(to_string(obj));

    return obj;
}