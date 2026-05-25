#include "ScriptProgramGTA5.hpp"

namespace scrDbgApp
{
    uint32_t ScriptProgramGTA5::GetGlobalVersion() const
    {
        return m_Base.Add(GLOBAL_VERSION).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetCodeSize() const
    {
        return m_Base.Add(CODE_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetArgCount() const
    {
        return m_Base.Add(ARG_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetStaticCount() const
    {
        return m_Base.Add(STATIC_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetGlobalCount() const
    {
        return m_Base.Add(GLOBAL_COUNT_AND_BLOCK).Get<uint32_t>() & 0x3FFFF;
    }

    uint32_t ScriptProgramGTA5::GetGlobalBlock() const
    {
        return m_Base.Add(GLOBAL_COUNT_AND_BLOCK).Get<uint32_t>() >> 0x12;
    }

    uint32_t ScriptProgramGTA5::GetNativeCount() const
    {
        return m_Base.Add(NATIVE_COUNT).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetNameHash() const
    {
        return m_Base.Add(NAME_HASH).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetStringsSize() const
    {
        return m_Base.Add(STRINGS_SIZE).Get<uint32_t>();
    }

    uint32_t ScriptProgramGTA5::GetRefCount() const
    {
        return m_Base.Add(REF_COUNT).Get<uint32_t>();
    }

    std::vector<uint8_t> ScriptProgramGTA5::GetCode() const
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

    void ScriptProgramGTA5::SetCode(uint32_t index, const std::vector<uint8_t>& bytes) const
    {
        if (index >= GetCodeSize())
            return;

        uintptr_t page = m_Base.Add(CODE_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        Pointer(page).Add(index & 0x3FFF).SetBuffer(bytes.data(), bytes.size());
    }

    int32_t ScriptProgramGTA5::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return 0;

        return m_Base.Add(STATICS).Deref().GetArray<int32_t>(index);
    }

    int32_t ScriptProgramGTA5::GetProgramGlobal(uint32_t index) const
    {
        if (index >= GetGlobalCount())
            return 0;

        uintptr_t page = m_Base.Add(GLOBAL_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        return Pointer(page).GetArray<int32_t>(index & 0x3FFF);
    }

    uint64_t ScriptProgramGTA5::GetNative(uint32_t index) const
    {
        if (index >= GetNativeCount())
            return 0;

        return m_Base.Add(NATIVES).Deref().GetArray<uint64_t>(index);
    }

    std::vector<std::string> ScriptProgramGTA5::GetStrings() const
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

    std::string ScriptProgramGTA5::GetString(uint32_t index) const
    {
        if (index >= GetStringsSize())
            return {};

        uintptr_t page = m_Base.Add(STRING_PAGES).Deref().GetArray<uintptr_t>(index >> 14);
        return Pointer(page).Add(index & 0x3FFF).GetString(255); // Max STRING length is 255 in RAGE scripts
    }

    std::vector<uint32_t> ScriptProgramGTA5::FindStringIndices(const std::string& string) const
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
}