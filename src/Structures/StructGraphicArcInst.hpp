#ifndef STRUCTGRAPHICARCINST_HPP
#define STRUCTGRAPHICARCINST_HPP


#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include <fmt/core.h>
#include <nameof.hpp>

#include "General.hpp"
#include "Structures/StructGraphicInst.hpp"


class StructGraphicArcInst : public StructGraphicInst
{
public:

    StructGraphicArcInst(ParserContext& aCtx) : StructGraphicInst{aCtx}
    { }

    std::string to_string() const override;

    void read(FileFormatVersion aVersion = FileFormatVersion::Unknown) override;

    virtual void accept(Visitor& aVisitor) const override
    {
        aVisitor.visit(*this);
    }
};


[[maybe_unused]]
static std::string to_string(const StructGraphicArcInst& aObj)
{
    return aObj.to_string();
}


inline std::string StructGraphicArcInst::to_string() const
{
    std::string str;

    str += fmt::format("{}:\n", nameof::nameof_type<decltype(*this)>());

    str += StructGraphicInst::to_string();

    return str;
}


[[maybe_unused]]
static std::ostream& operator<<(std::ostream& aOs, const StructGraphicArcInst& aVal)
{
    aOs << to_string(aVal);

    return aOs;
}


#endif // STRUCTGRAPHICARCINST_HPP