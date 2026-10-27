#include "DecompiledFunction.hpp"

namespace scrDbgApp::Decomp
{
    std::string DecompiledFunction::ToText() const
    {
        size_t size = 0;
        for (const auto& line : Lines)
            size += line.size() + 1;

        std::string text;
        text.reserve(size);
        for (const auto& line : Lines)
        {
            text += line;
            text += '\n';
        }
        return text;
    }

    int DecompiledFunction::FindLine(uint32_t address) const
    {
        auto it = AddressToLine.find(address);
        return it != AddressToLine.end() ? it->second : -1;
    }

    std::vector<DecompiledSpan> DecompiledFunction::FindSpans(uint32_t address) const
    {
        std::vector<DecompiledSpan> result;
        for (const auto& span : Spans)
        {
            if (span.Address == address)
                result.push_back(span);
        }

        int home = FindLine(address);
        std::stable_sort(result.begin(), result.end(), [home](const DecompiledSpan& a, const DecompiledSpan& b) {
            bool aHome = a.Line == home, bHome = b.Line == home;
            if (aHome != bHome)
                return aHome;
            return (a.End - a.Start) < (b.End - b.Start);
        });
        return result;
    }

    std::optional<DecompiledSpan> DecompiledFunction::FindSpanAt(int line, int column) const
    {
        std::optional<DecompiledSpan> best;
        for (const auto& span : Spans)
        {
            if (span.Line != line || column < span.Start || column >= span.End)
                continue;

            if (!best || (span.End - span.Start) < (best->End - best->Start))
                best = span;
        }
        return best;
    }

    std::optional<uint32_t> DecompiledFunction::FindLinkAt(int line, int column) const
    {
        for (const auto& link : Links)
        {
            if (link.Line == line && column >= link.Start && column < link.End)
                return link.Target;
        }
        return std::nullopt;
    }

    std::optional<uint32_t> DecompiledFunction::GetAddressAt(int line, int column) const
    {
        if (auto span = FindSpanAt(line, column))
            return span->Address;
        return GetLineAddress(line);
    }

    std::optional<uint32_t> DecompiledFunction::GetLineAddress(int line) const
    {
        if (line < 0 || line >= static_cast<int>(LineAddresses.size()) || LineAddresses[line].empty())
            return std::nullopt;

        for (uint32_t address : LineAddresses[line]) // sorted ascending
        {
            if (FindLine(address) == line)
                return address;
        }

        return LineAddresses[line].front();
    }

    void DecompiledFunction::PlaceUnclaimedInstructions(std::vector<uint32_t> instructionAddresses, int fallbackLine)
    {
        std::sort(instructionAddresses.begin(), instructionAddresses.end());

        // Next claimed line, scanning backwards.
        std::vector<int> nextLine(instructionAddresses.size(), -1);
        int next = -1;
        for (size_t i = instructionAddresses.size(); i-- > 0;)
        {
            int line = FindLine(instructionAddresses[i]);
            if (line >= 0)
                next = line;
            nextLine[i] = next;
        }

        int previous = -1;
        for (size_t i = 0; i < instructionAddresses.size(); i++)
        {
            uint32_t address = instructionAddresses[i];
            int line = FindLine(address);
            if (line >= 0)
            {
                previous = line;
                continue;
            }

            int target = nextLine[i] >= 0 ? nextLine[i] : (previous >= 0 ? previous : fallbackLine);
            if (target >= 0)
                AddressToLine.emplace(address, target);
        }
    }
}