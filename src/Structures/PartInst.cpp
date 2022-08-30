#include <cstdint>
#include <ostream>
#include <string>

#include <nameof.hpp>
#include <spdlog/spdlog.h>

#include "../Enums/LineStyle.hpp"
#include "../Enums/LineWidth.hpp"
#include "../General.hpp"
#include "../Parser.hpp"


// @todo return real data object
bool Parser::readPartInst()
{
    spdlog::debug(getOpeningMsg(__func__, mDs.getCurrentOffset()));

    const std::optional<FutureData> thisFuture = getFutureData();

    bool obj = false;

    mDs.printUnknownData(8, std::string(__func__) + " - 0");

    std::string pkgName = mDs.readStringLenZeroTerm();

    uint32_t dbId = mDs.readUint32();

    mDs.printUnknownData(8, std::string(__func__) + " - 1");

    int16_t locX = mDs.readInt16();
    int16_t locY = mDs.readInt16();

    Color color = ToColor(mDs.readUint16()); // @todo educated guess

    mDs.printUnknownData(2, std::string(__func__) + " - 2");

    uint16_t len = mDs.readUint16();

    for(size_t i = 0u; i < len; ++i)
    {
        // Structure structure = read_prefixes(3);
        Structure structure = auto_read_prefixes();
        readPreamble();
        parseStructure(structure); // @todo push struct
    }

    mDs.printUnknownData(1, std::string(__func__) + " - 3");

    std::string reference = mDs.readStringLenZeroTerm();

    mDs.printUnknownData(14, std::string(__func__) + " - 4");

    uint16_t len2 = mDs.readUint16();

    for(size_t i = 0u; i < len2; ++i)
    {
        // Structure structure = read_prefixes(3);
        Structure structure = auto_read_prefixes();
        readPreamble();
        parseStructure(structure); // @todo push struct
    }

    std::string sth1 = mDs.readStringLenZeroTerm(); // @todo needs verification

    mDs.printUnknownData(2, std::string(__func__) + " - 5");

    // @todo implement type_prefix_very_long
    mDs.printUnknownData(18, std::string(__func__) + " - 6");
    // Structure structure = read_prefixes(4);
    Structure structure = auto_read_prefixes();
    readPreamble();

    // sanitizeThisFutureSize(thisFuture);

    checkTrailingFuture();

    spdlog::debug(getClosingMsg(__func__, mDs.getCurrentOffset()));

    return obj;
}