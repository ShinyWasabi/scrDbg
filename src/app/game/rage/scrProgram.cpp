#include "scrProgram.hpp"
#include "Pointers.hpp"

namespace rage
{
    uint32_t scrProgram::GetGlobalVersion() const
    {
        return m_Base.Add(GLOBAL_VERSION).Get<uint32_t>();
    }

    uint32_t scrProgram::GetCodeSize() const
    {
        return m_Base.Add(CODE_SIZE).Get<uint32_t>();
    }

    uint32_t scrProgram::GetArgCount() const
    {
        return m_Base.Add(ARG_COUNT).Get<uint32_t>();
    }

    uint32_t scrProgram::GetStaticCount() const
    {
        return m_Base.Add(STATIC_COUNT).Get<uint32_t>();
    }

    uint32_t scrProgram::GetGlobalCount() const
    {
        return m_Base.Add(GLOBAL_COUNT_AND_BLOCK).Get<uint32_t>() & 0x3FFFF;
    }

    uint32_t scrProgram::GetGlobalBlock() const
    {
        return m_Base.Add(GLOBAL_COUNT_AND_BLOCK).Get<uint32_t>() >> 0x12;
    }

    uint32_t scrProgram::GetNativeCount() const
    {
        return m_Base.Add(NATIVE_COUNT).Get<uint32_t>();
    }

    uint32_t scrProgram::GetNameHash() const
    {
        return m_Base.Add(NAME_HASH).Get<uint32_t>();
    }

    uint32_t scrProgram::GetStringsSize() const
    {
        return m_Base.Add(STRINGS_SIZE).Get<uint32_t>();
    }

    uint32_t scrProgram::GetRefCount() const
    {
        return m_Base.Add(REF_COUNT).Get<uint32_t>();
    }

    std::vector<uint8_t> scrProgram::GetFullCode() const
    {
        uint32_t codeSize = GetCodeSize();
        if (!codeSize)
            return {};

        std::vector<uint8_t> code(codeSize);
        uint32_t pageCount = (codeSize + 0x3FFF) >> 14;

        std::vector<uintptr_t> pages(pageCount);
        m_Base.Add(CODE_PAGES).Deref().GetBuffer(pages.data(), pageCount * sizeof(uintptr_t));
        for (uint32_t i = 0, offset = 0; i < pageCount; i++)
        {
            size_t pageSize = std::min<size_t>(codeSize - offset, 0x4000);
            Pointer(pages[i]).GetBuffer(code.data() + offset, pageSize);
            offset += pageSize;
        }

        return code;
    }

    void scrProgram::SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
    {
        if (index >= GetCodeSize())
            return;

        uintptr_t page = m_Base.Add(CODE_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        Pointer(page).Add(index & 0x3FFF).SetBuffer(bytes.data(), bytes.size());
    }

    uint64_t scrProgram::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return 0;

        return m_Base.Add(STATICS).Deref().GetArray<uint64_t>(index);
    }

    uint64_t scrProgram::GetProgramGlobal(uint32_t index) const
    {
        if (index >= GetGlobalCount())
            return 0;

        uintptr_t page = m_Base.Add(GLOBAL_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        return Pointer(page).GetArray<uint64_t>(index & 0x3FFF);
    }

    uint64_t scrProgram::GetNative(uint32_t index) const
    {
        if (index >= GetNativeCount())
            return 0;

        return m_Base.Add(NATIVES).Deref().GetArray<uint64_t>(index);
    }

    std::vector<std::string> scrProgram::GetAllStrings() const
    {
        uint32_t stringsSize = GetStringsSize();
        if (!stringsSize)
            return {};

        std::vector<std::string> strings;
        uint32_t pageCount = (stringsSize + 0x3FFF) >> 14;

        std::vector<uintptr_t> pages(pageCount);
        m_Base.Add(STRING_PAGES).Deref().GetBuffer(pages.data(), pageCount * sizeof(uintptr_t));
        for (uint32_t i = 0; i < pageCount; i++)
        {
            uint32_t pageSize = std::min<uint32_t>(stringsSize - i * 0x4000, 0x4000);
            std::vector<char> page(pageSize);
            Pointer(pages[i]).GetBuffer(page.data(), page.size());

            size_t index = 0;
            while (index < page.size())
            {
                strings.push_back(&page[index]);
                index += strings.back().size() + 1;
            }
        }

        return strings;
    }

    std::string scrProgram::GetString(uint32_t index) const
    {
        if (index >= GetStringsSize())
            return {};

        uintptr_t page = m_Base.Add(STRING_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        return Pointer(page).Add(index & 0x3FFF).GetString(255); // Max STRING length is 255 in RAGE scripts, at least for GTA V
    }

    std::vector<uint32_t> scrProgram::FindStringIndices(const std::string& string) const
    {
        std::vector<uint32_t> result;

        uint32_t size = GetStringsSize();

        uint32_t index = 0;
        while (index < size)
        {
            std::string str = GetString(index);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);

            if (str.find(string) != std::string::npos)
                result.push_back(index);

            index += static_cast<uint32_t>(str.size()) + 1;
        }

        return result;
    }

    uint64_t scrProgram::GetGlobal(uint32_t index)
    {
        uintptr_t block = scrDbgApp::g_Pointers.ScriptGlobals.GetArray<uintptr_t>((index >> 0x12) & 0x3F);
        return Pointer(block).GetArray<uint64_t>(index & 0x3FFFF);
    }

    void scrProgram::SetGlobal(uint32_t index, uint64_t value)
    {
        uintptr_t block = scrDbgApp::g_Pointers.ScriptGlobals.GetArray<uintptr_t>((index >> 0x12) & 0x3F);
        Pointer(block).SetArray<uint64_t>(index & 0x3FFFF, value);
    }

    uint32_t scrProgram::GetGlobalBlockCount(uint32_t block)
    {
        return scrDbgApp::g_Pointers.ScriptGlobalBlockCounts.GetArray<uint32_t>(block);
    }

    scrProgram scrProgram::GetProgram(uint32_t hash)
    {
        if (!hash)
            return {};

        for (uint32_t i = 0; i < 176; i++)
        {
            scrProgram program(scrDbgApp::g_Pointers.ScriptPrograms.GetArray<uintptr_t>(i));
            if (program.GetNameHash() == hash)
                return program;
        }

        return {};
    }
}