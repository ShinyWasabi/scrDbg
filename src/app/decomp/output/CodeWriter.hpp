#pragma once
#include "DecompiledFunction.hpp"

namespace scrDbgApp::Decomp
{
    class CodeWriter
    {
    public:
        void Line(int indent, const std::string& markedText, const std::vector<uint32_t>& extraAddresses = {});

        void Blank();
        void Append(CodeWriter&& other);

        int GetLineCount() const
        {
            return static_cast<int>(m_Result.Lines.size());
        }

        DecompiledFunction& GetResult()
        {
            return m_Result;
        }

    private:
        DecompiledFunction m_Result;
    };
}