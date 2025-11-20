#include "scrProgram.hpp"
#include "Pointers.hpp"
#include "core/Process.hpp"

namespace rage
{
    std::vector<uint8_t> scrProgram::GetFullCode() const
    {
        uint32_t codeSize = GetCodeSize();
        if (!codeSize)
            return {};

        std::vector<uint8_t> code(codeSize);

        size_t offset = offsetof(_scrProgram, m_CodeBlocks);

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        uint32_t count = (codeSize + 0x3FFF) >> 14;

        std::vector<uint64_t> ptrs(count);
        scrDbg::Process::ReadRaw(base, ptrs.data(), count * sizeof(uint64_t));

        for (uint32_t i = 0, offset = 0; i < count; ++i)
        {
            size_t chunk = std::min<size_t>(codeSize - offset, 0x4000);
            scrDbg::Process::ReadRaw(ptrs[i], code.data() + offset, chunk);
            offset += chunk;
        }

        return code;
    }

    uint8_t scrProgram::GetCode(uint32_t index) const
    {
        if (index < GetCodeSize())
        {
            size_t offset = offsetof(_scrProgram, m_CodeBlocks);

            uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
            return scrDbg::Process::Read<uint8_t>(scrDbg::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t)) + (index & 0x3FFF));
        }

        return 0;
    }

    void scrProgram::SetCode(uint32_t index, uint8_t byte) const
    {
        if (index < GetCodeSize())
        {
            size_t offset = offsetof(_scrProgram, m_CodeBlocks);

            uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
            scrDbg::Process::Write<uint8_t>(scrDbg::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t)) + (index & 0x3FFF), byte);
        }
    }

    uint32_t scrProgram::GetGlobalVersion() const
    {
        size_t offset = offsetof(_scrProgram, m_GlobalVersion);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrProgram::GetCodeSize() const
    {
        size_t offset = offsetof(_scrProgram, m_CodeSize);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrProgram::GetArgCount() const
    {
        size_t offset = offsetof(_scrProgram, m_ArgCount);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrProgram::GetStaticCount() const
    {
        size_t offset = offsetof(_scrProgram, m_StaticCount);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrProgram::GetGlobalCount() const
    {
        size_t offset = offsetof(_scrProgram, m_GlobalCountAndBlock);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset) & 0x3FFFF;
    }

    uint32_t scrProgram::GetGlobalBlockIndex() const
    {
        size_t offset = offsetof(_scrProgram, m_GlobalCountAndBlock);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset) >> 0x12;
    }

    uint32_t scrProgram::GetNativeCount() const
    {
        size_t offset = offsetof(_scrProgram, m_NativeCount);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint64_t scrProgram::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return 0;

        size_t offset = offsetof(_scrProgram, m_Statics);

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        return scrDbg::Process::Read<uint64_t>(base + index * sizeof(uint64_t));
    }

    uint64_t scrProgram::GetProgramGlobal(uint32_t index) const
    {
        if (index >= GetGlobalCount())
            return 0;

        size_t offset = offsetof(_scrProgram, m_Globals);

        int blockIndex = (index >> 0x12) & 0x3F;
        int _offset = index & 0x3FFFF;

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        uint64_t block = scrDbg::Process::Read<uint64_t>(base + blockIndex * sizeof(uint64_t));
        return scrDbg::Process::Read<uint64_t>(block + _offset * sizeof(uint64_t));
    }

    uint64_t scrProgram::GetNative(uint32_t index) const
    {
        if (index >= GetNativeCount())
            return 0;

        size_t offset = offsetof(_scrProgram, m_Natives);

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        return scrDbg::Process::Read<uint64_t>(base + index * sizeof(uint64_t));
    }

    uint32_t scrProgram::GetHash() const
    {
        size_t offset = offsetof(_scrProgram, m_NameHash);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint32_t scrProgram::GetRefCount() const
    {
        size_t offset = offsetof(_scrProgram, m_RefCount);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    std::vector<std::string> scrProgram::GetAllStrings() const
    {
        uint32_t stringCount = GetStringCount();
        if (!stringCount)
            return {};

        std::vector<std::string> strings;
        strings.reserve(stringCount);

        size_t offset = offsetof(_scrProgram, m_Strings);

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        uint32_t count = (stringCount + 0x3FFF) >> 14;

        std::vector<uint64_t> ptrs(count);
        scrDbg::Process::ReadRaw(base, ptrs.data(), count * sizeof(uint64_t));

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t blockSize = std::min<uint32_t>(stringCount - i * 0x4000, 0x4000);

            std::vector<char> block(blockSize);
            scrDbg::Process::ReadRaw(ptrs[i], block.data(), block.size());

            size_t j = 0;
            while (j < block.size() && strings.size() < stringCount)
            {
                const char* start = &block[j];
                size_t len = strnlen(start, block.size() - j);

                if (len == 0)
                {
                    ++j;
                    continue;
                }

                strings.emplace_back(start, len);
                j += len + 1;
            }
        }

        return strings;
    }

    std::string scrProgram::GetString(uint32_t index) const
    {
        if (index >= GetStringCount())
            return {};

        size_t offset = offsetof(_scrProgram, m_Strings);

        uint64_t base = scrDbg::Process::Read<uint64_t>(m_Address + offset);
        uint64_t page = scrDbg::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t));
        uint64_t addr = page + (index & 0x3FFF);

        std::string str;
        for (int i = 0; i < 255; ++i) // Max STRING length is 255 in RAGE scripts, at least for GTA V
        {
            char c = scrDbg::Process::Read<char>(addr + i);
            if (c == '\0')
                break;
            str += c;
        }

        return str;
    }

    std::vector<uint32_t> scrProgram::FindScriptIndex(const std::string& string) const
    {
        std::vector<uint32_t> result;

        uint32_t count = GetStringCount();

        uint32_t index = 0;
        while (index < count)
        {
            std::string str = GetString(index);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);

            if (str.find(string) != std::string::npos)
                result.push_back(index);

            index += static_cast<uint32_t>(str.size()) + 1;
        }

        return result;
    }

    uint32_t scrProgram::GetStringCount() const
    {
        size_t offset = offsetof(_scrProgram, m_StringCount);
        return scrDbg::Process::Read<uint32_t>(m_Address + offset);
    }

    uint64_t scrProgram::GetGlobal(uint32_t index)
    {
        int blockIndex = (index >> 0x12) & 0x3F;
        int offset = index & 0x3FFFF;

        uint64_t base = scrDbg::g_Pointers.ScriptGlobals.Add(blockIndex * sizeof(uint64_t)).Read<uint64_t>();
        return scrDbg::Process::Read<uint64_t>(base + offset * sizeof(uint64_t));
    }

    void scrProgram::SetGlobal(uint32_t index, uint64_t value)
    {
        int blockIndex = (index >> 0x12) & 0x3F;
        int offset = index & 0x3FFFF;

        uint64_t base = scrDbg::g_Pointers.ScriptGlobals.Add(blockIndex * sizeof(uint64_t)).Read<uint64_t>();
        scrDbg::Process::Write<uint64_t>(base + offset * sizeof(uint64_t), value);
    }

    int scrProgram::GetGlobalBlockCount(uint32_t block)
    {
        return scrDbg::g_Pointers.ScriptGlobalBlockCounts.Add(block * sizeof(int)).Read<int>();
    }

    scrProgram scrProgram::GetProgram(uint32_t hash)
    {
        if (!hash)
            return scrProgram();

        for (int i = 0; i < 176; i++)
        {
            scrProgram program(scrDbg::Process::Read<uint64_t>(scrDbg::g_Pointers.ScriptPrograms + i * sizeof(uint64_t)));
            if (program.GetHash() == hash)
                return program;
        }

        return scrProgram();
    }
}