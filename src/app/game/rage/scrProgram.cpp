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

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, CodeBlocks));
        uint32_t count = (codeSize + 0x3FFF) >> 14;

        std::vector<uint64_t> ptrs(count);
        scrDbgApp::Process::ReadRaw(base, ptrs.data(), count * sizeof(uint64_t));

        for (uint32_t i = 0, offset = 0; i < count; ++i)
        {
            size_t chunk = std::min<size_t>(codeSize - offset, 0x4000);
            scrDbgApp::Process::ReadRaw(ptrs[i], code.data() + offset, chunk);
            offset += chunk;
        }

        return code;
    }

    uint8_t scrProgram::GetCode(uint32_t index) const
    {
        if (index < GetCodeSize())
        {
            uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, CodeBlocks));
            return scrDbgApp::Process::Read<uint8_t>(scrDbgApp::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t)) + (index & 0x3FFF));
        }

        return 0;
    }

    void scrProgram::SetCode(uint32_t index, uint8_t byte) const
    {
        if (index < GetCodeSize())
        {
            uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, CodeBlocks));
            scrDbgApp::Process::Write<uint8_t>(scrDbgApp::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t)) + (index & 0x3FFF), byte);
        }
    }

    uint32_t scrProgram::GetGlobalVersion() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, GlobalVersion));
    }

    uint32_t scrProgram::GetCodeSize() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, CodeSize));
    }

    uint32_t scrProgram::GetArgCount() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, ArgCount));
    }

    uint32_t scrProgram::GetStaticCount() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, StaticCount));
    }

    uint32_t scrProgram::GetGlobalCount() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, GlobalCountAndBlock)) & 0x3FFFF;
    }

    uint32_t scrProgram::GetGlobalBlockIndex() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, GlobalCountAndBlock)) >> 0x12;
    }

    uint32_t scrProgram::GetNativeCount() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, NativeCount));
    }

    uint64_t scrProgram::GetStatic(uint32_t index) const
    {
        if (index >= GetStaticCount())
            return 0;

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, Statics));
        return scrDbgApp::Process::Read<uint64_t>(base + index * sizeof(uint64_t));
    }

    uint64_t scrProgram::GetProgramGlobal(uint32_t index) const
    {
        if (index >= GetGlobalCount())
            return 0;

        int blockIndex = (index >> 0x12) & 0x3F;
        int _offset = index & 0x3FFFF;

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, Globals));
        uint64_t block = scrDbgApp::Process::Read<uint64_t>(base + blockIndex * sizeof(uint64_t));
        return scrDbgApp::Process::Read<uint64_t>(block + _offset * sizeof(uint64_t));
    }

    uint64_t scrProgram::GetNative(uint32_t index) const
    {
        if (index >= GetNativeCount())
            return 0;

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, Natives));
        return scrDbgApp::Process::Read<uint64_t>(base + index * sizeof(uint64_t));
    }

    uint32_t scrProgram::GetHash() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, NameHash));
    }

    uint32_t scrProgram::GetRefCount() const
    {
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, RefCount));
    }

    std::vector<std::string> scrProgram::GetAllStrings() const
    {
        uint32_t stringCount = GetStringCount();
        if (!stringCount)
            return {};

        std::vector<std::string> strings;
        strings.reserve(stringCount);

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, Strings));
        uint32_t count = (stringCount + 0x3FFF) >> 14;

        std::vector<uint64_t> ptrs(count);
        scrDbgApp::Process::ReadRaw(base, ptrs.data(), count * sizeof(uint64_t));

        for (uint32_t i = 0; i < count; ++i)
        {
            uint32_t blockSize = std::min<uint32_t>(stringCount - i * 0x4000, 0x4000);

            std::vector<char> block(blockSize);
            scrDbgApp::Process::ReadRaw(ptrs[i], block.data(), block.size());

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

        uint64_t base = scrDbgApp::Process::Read<uint64_t>(m_Address + offsetof(_scrProgram, Strings));
        uint64_t page = scrDbgApp::Process::Read<uint64_t>(base + (index >> 14) * sizeof(uint64_t));
        uint64_t addr = page + (index & 0x3FFF);

        std::string str;
        for (int i = 0; i < 255; ++i) // Max STRING length is 255 in RAGE scripts, at least for GTA V
        {
            char c = scrDbgApp::Process::Read<char>(addr + i);
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
        return scrDbgApp::Process::Read<uint32_t>(m_Address + offsetof(_scrProgram, StringCount));
    }

    uint64_t scrProgram::GetGlobal(uint32_t index)
    {
        int blockIndex = (index >> 0x12) & 0x3F;
        int offset = index & 0x3FFFF;

        uint64_t base = scrDbgApp::g_Pointers.ScriptGlobals.Add(blockIndex * sizeof(uint64_t)).Read<uint64_t>();
        return scrDbgApp::Process::Read<uint64_t>(base + offset * sizeof(uint64_t));
    }

    void scrProgram::SetGlobal(uint32_t index, uint64_t value)
    {
        int blockIndex = (index >> 0x12) & 0x3F;
        int offset = index & 0x3FFFF;

        uint64_t base = scrDbgApp::g_Pointers.ScriptGlobals.Add(blockIndex * sizeof(uint64_t)).Read<uint64_t>();
        scrDbgApp::Process::Write<uint64_t>(base + offset * sizeof(uint64_t), value);
    }

    int scrProgram::GetGlobalBlockCount(uint32_t block)
    {
        return scrDbgApp::g_Pointers.ScriptGlobalBlockCounts.Add(block * sizeof(int)).Read<int>();
    }

    scrProgram scrProgram::GetProgram(uint32_t hash)
    {
        if (!hash)
            return scrProgram();

        for (int i = 0; i < 176; i++)
        {
            scrProgram program(scrDbgApp::Process::Read<uint64_t>(scrDbgApp::g_Pointers.ScriptPrograms + i * sizeof(uint64_t)));
            if (program.GetHash() == hash)
                return program;
        }

        return scrProgram();
    }
}