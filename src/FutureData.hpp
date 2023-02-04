#ifndef FUTUREDATA_HPP
#define FUTUREDATA_HPP

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "General.hpp"
#include "ParserContext.hpp"


class FutureData
{
public:

    FutureData(std::size_t aPreambleOffset, std::size_t aSize) : mParsed{false}
    {
        const size_t PREAMBLE_STRIDE = 9U; //!< Stride from one preamble to the next one in byte

        mAbsStartOffset = aPreambleOffset + PREAMBLE_STRIDE;
        mAbsStopOffset  = mAbsStartOffset + aSize;
    }

    std::size_t getStartOffset() const
    {
        return mAbsStartOffset;
    }

    std::size_t getStopOffset() const
    {
        return mAbsStopOffset;
    }

    std::size_t getByteLen() const
    {
        return mAbsStopOffset - mAbsStartOffset;
    }

    void setParsed(bool aParsed)
    {
        mParsed = aParsed;
    }

    bool getParsed() const
    {
        return mParsed;
    }

    std::string string() const
    {
        return fmt::format("0x{:08x} -> 0x{:08x}: {} Byte{}", getStartOffset(), getStopOffset(), getByteLen(),
            (getParsed() ? " (verified by checkpoint)" : ""));
    }

private:

    std::size_t mAbsStartOffset; //!< Absolute offset from the beginning of the file where the data starts
    std::size_t mAbsStopOffset; //!< Absolute offset from the beginning of the file where the data ends

    bool mParsed; //!< Set if the specified range has been parsed
};



class FutureDataLst : public std::vector<FutureData>
{
public:

    FutureDataLst(ParserContext& aCtx) : std::vector<FutureData>{}, mCtx{aCtx}
    { }

    std::optional<FutureData> getByStartOffset(std::size_t aAbsStartOffset) const
    {
        spdlog::debug("Searching for StartOffset 0x{:08x}", aAbsStartOffset);

        const auto cmp = [&aAbsStartOffset] (FutureData aFutureData) -> bool
            { return aFutureData.getStartOffset() == aAbsStartOffset; };

        const auto res = std::find_if(this->begin(), this->end(), cmp);

        if(res != this->end())
        {
            return std::make_optional<FutureData>(*res);
        }

        return std::nullopt;
    }

    std::optional<FutureData> getByStopOffset(std::size_t aAbsStopOffset) const
    {
        spdlog::debug("Searching for StopOffset 0x{:08x}", aAbsStopOffset);

        const auto cmp = [&aAbsStopOffset] (FutureData aFutureData) -> bool
            { return aFutureData.getStopOffset() == aAbsStopOffset; };

        const auto res = std::find_if(this->begin(), this->end(), cmp);

        if(res != this->end())
        {
            return std::make_optional<FutureData>(*res);
        }

        return std::nullopt;
    }

    bool removeByStartOffset(std::size_t aAbsStartOffset)
    {
        const auto cmp = [&aAbsStartOffset] (FutureData aFutureData) -> bool
            { return aFutureData.getStartOffset() == aAbsStartOffset; };

        const auto res = std::find_if(this->begin(), this->end(), cmp);

        if(res != this->end())
        {
            this->erase(res);
            return true;
        }

        return false;
    }

    bool removeByStopOffset(std::size_t aAbsStopOffset)
    {
        const auto cmp = [&aAbsStopOffset] (FutureData aFutureData) -> bool
            { return aFutureData.getStopOffset() == aAbsStopOffset; };

        const auto res = std::find_if(this->begin(), this->end(), cmp);

        if(res != this->end())
        {
            this->erase(res);
            return true;
        }

        return false;
    }

    void checkpoint()
    {
        const size_t currOffset = mCtx.get().mDs.get().getCurrentOffset();

        const auto cmp = [&currOffset] (FutureData aFutureData) -> bool
            { return aFutureData.getStopOffset() == currOffset; };

        auto res = std::find_if(this->begin(), this->end(), cmp);

        if(res != this->end())
        {
            auto& futureData = *res;

            futureData.setParsed(true);

            spdlog::debug("{}: Checkpoint at 0x{:08x} was successful",
                getMethodName(this, __func__), currOffset);
        }
        else
        {
            if(!empty())
            {
                const std::string msg = fmt::format("{}: Checkpoint position at 0x{:08x} is incorrect",
                    getMethodName(this, __func__), currOffset);

                throw std::runtime_error(msg);
            }

            spdlog::trace("{}: Checkpoint at 0x{:08x} was not found",
                getMethodName(this, __func__), currOffset);
        }
    }

    void sanitizeCheckpoints() const
    {
        bool checkpoint_missing = false;

        for(const FutureData& data : *this)
        {
            if(!data.getParsed())
            {
                checkpoint_missing = true;

                spdlog::debug("{}: Checkpoint missing for 0x{:08x} -> 0x{:08x}",
                    getMethodName(this, __func__),
                    data.getStartOffset(), data.getStopOffset());
            }
        }

        if(checkpoint_missing)
        {
            const std::string msg = fmt::format("{}: Check your code for missing checkpoints!\n{}",
                getMethodName(this, __func__), string());

            spdlog::debug(msg);
            throw std::runtime_error(msg);
        }
    }

    std::string string() const
    {
        std::string txt{fmt::format("Future Data List with {} Element{}:",
            size(), size() > 1U ? "s" : "")};

        for(const auto& e : *this)
        {
            txt += indent(1) + "\n" + e.string();
        }

        return txt;
    }

    void readUntilNextFutureData()
    {
        auto& ds = mCtx.get().mDs.get();

        const size_t curPos = ds.getCurrentOffset();

        const auto pred = [&curPos] (const FutureData aFutureData) -> bool {
                return curPos < aFutureData.getStopOffset();
            };

        auto res = std::find_if(this->rbegin(), this->rend(), pred);

        if(res != this->rend())
        {
            size_t byteDiff = res->getStopOffset() - curPos;

            ds.printUnknownData(byteDiff, fmt::format("{}: Reading rest of future data ({} Byte)",
                getMethodName(this, __func__), byteDiff));
        }
        else
        {
            spdlog::debug("{}: Did not find any future data following current offset 0x{:08x}",
                getMethodName(this, __func__), curPos);
        }
    }

    void readRestOfStructure()
    {
        spdlog::trace(string());

        int64_t endPos = 0;

        for(const auto& data : *this)
        {
            endPos = std::max(static_cast<int64_t>(data.getStopOffset()), endPos);
        }

        const int64_t curPos = static_cast<int64_t>(mCtx.get().mDs.get().getCurrentOffset());

        const int64_t byteDiff = endPos - curPos;

        if(byteDiff > 0)
        {
            const std::string msg = fmt::format("{}: Your structure implementation is too small."
                " Expected it to end at 0x{:08x} but ended at 0x{:08x}. Too small by {} Byte.",
                getMethodName(this, __func__), endPos, curPos, std::abs(byteDiff));

            mCtx.get().mDs.get().printUnknownData(byteDiff, msg);
        }
        else if(byteDiff < 0)
        {
            // @todo We could set to file position pointer to
            //       the correct position and continue after a
            //       warning message. This might be a better
            //       approach to see where in the structure the
            //       parser fails. With the current method the
            //       structure as a whole would be skipped

            const std::string msg = fmt::format("{}: Your structure implementation is too large."
                " Expected it to end at 0x{:08x} but ended at 0x{:08x}. Too large by {} Byte.",
                getMethodName(this, __func__), endPos, curPos, std::abs(byteDiff));

            spdlog::debug(msg);
            throw std::runtime_error(msg);
        }
    }

private:

    std::reference_wrapper<ParserContext> mCtx;

};

#endif // FUTUREDATA_HPP