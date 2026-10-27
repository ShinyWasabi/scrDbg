#include "CodeWriter.hpp"
#include "decomp/ast/Ast.hpp"

namespace scrDbgApp::Decomp
{
    struct Open
    {
        bool IsLink;
        uint64_t Value;
        int Start;
    };

    void CodeWriter::Line(int indent, const std::string& markedText, const std::vector<uint32_t>& extraAddresses)
    {
        const int lineIndex = static_cast<int>(m_Result.Lines.size());

        std::string text(static_cast<size_t>(std::max(indent, 0)), '\t');
        text.reserve(text.size() + markedText.size());

        std::vector<uint32_t> addresses = extraAddresses;
        std::vector<Open> open;

        for (size_t i = 0; i < markedText.size(); i++)
        {
            const char c = markedText[i];

            if (c == Marker::NODE_BEGIN || c == Marker::LINK_BEGIN)
            {
                size_t separator = markedText.find(Marker::SEPARATOR, i + 1);
                if (separator == std::string::npos)
                    break; // malformed

                uint64_t value = std::stoull(markedText.substr(i + 1, separator - i - 1), nullptr, 16);
                open.push_back({c == Marker::LINK_BEGIN, value, static_cast<int>(text.size())});
                i = separator;
                continue;
            }

            if (c == Marker::END)
            {
                if (open.empty())
                    continue;

                Open item = open.back();
                open.pop_back();
                const int end = static_cast<int>(text.size());

                if (item.IsLink)
                {
                    m_Result.Links.push_back({lineIndex, item.Start, end, static_cast<uint32_t>(item.Value)});
                }
                else
                {
                    auto address = static_cast<uint32_t>(item.Value);
                    m_Result.Spans.push_back({lineIndex, item.Start, end, address});
                    addresses.push_back(address);
                }
                continue;
            }

            text += c;
        }

        std::sort(addresses.begin(), addresses.end());
        addresses.erase(std::unique(addresses.begin(), addresses.end()), addresses.end());

        for (uint32_t address : addresses)
            m_Result.AddressToLine.emplace(address, lineIndex); // first line wins

        m_Result.Lines.push_back(std::move(text));
        m_Result.LineAddresses.push_back(std::move(addresses));
    }

    void CodeWriter::Blank()
    {
        m_Result.Lines.emplace_back();
        m_Result.LineAddresses.emplace_back();
    }

    void CodeWriter::Append(CodeWriter&& other)
    {
        const int offset = GetLineCount();
        auto& src = other.m_Result;

        for (auto& span : src.Spans)
        {
            span.Line += offset;
            m_Result.Spans.push_back(span);
        }

        for (auto& link : src.Links)
        {
            link.Line += offset;
            m_Result.Links.push_back(link);
        }

        for (const auto& [address, line] : src.AddressToLine)
            m_Result.AddressToLine.emplace(address, line + offset);

        for (auto& line : src.Lines)
            m_Result.Lines.push_back(std::move(line));

        for (auto& addresses : src.LineAddresses)
            m_Result.LineAddresses.push_back(std::move(addresses));
    }
}