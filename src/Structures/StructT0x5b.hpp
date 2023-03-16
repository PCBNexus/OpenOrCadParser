#ifndef STRUCTT0X5B_HPP
#define STRUCTT0X5B_HPP


#include <cstdint>
#include <optional>
#include <ostream>
#include <string>

#include <fmt/core.h>
#include <nameof.hpp>

#include "CommonBase.hpp"
#include "General.hpp"


class StructT0x5b : public CommonBase
{
public:

    StructT0x5b(ParserContext& aCtx) : CommonBase{aCtx}
    { }

    std::string to_string() const override;

    virtual void accept(Visitor& aVisitor) const override
    {
        aVisitor.visit(*this);
    }

    void read(FileFormatVersion aVersion = FileFormatVersion::Unknown) override;

};


[[maybe_unused]]
static std::string to_string(const StructT0x5b& aObj)
{
    std::string str;

    str += fmt::format("{}:\n", nameof::nameof_type<decltype(aObj)>());

    return str;
}


inline std::string StructT0x5b::to_string() const
{
    return ::to_string(*this);
}


[[maybe_unused]]
static std::ostream& operator<<(std::ostream& aOs, const StructT0x5b& aVal)
{
    aOs << to_string(aVal);

    return aOs;
}


#endif // STRUCTT0X5B_HPP