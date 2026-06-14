#pragma once
#include "Types.hpp"
#include "disasm/Disassembler.hpp"

namespace scrDbgApp
{
    enum class GameType
    {
        UNKNOWN = -1,
        GTA4,
        GTA5_GEN8,
        GTA5_GEN9
    };

    constexpr const std::pair<GameType, const wchar_t*> g_Games[] = {
        {GameType::GTA4, L"GTAIV.exe"},
        {GameType::GTA5_GEN8, L"GTA5.exe"},
        {GameType::GTA5_GEN9, L"GTA5_Enhanced.exe"}};

    class Game
    {
    public:
        virtual ~Game() = default;

        virtual bool Init() = 0;
        virtual GameType GetType() const = 0;
        virtual const char* GetTargetBuild() const = 0;
        virtual int GetResourceId() const = 0;
        virtual bool Is64Bit() const = 0;
        virtual std::unique_ptr<Disassembler> CreateDisassembly(std::unique_ptr<ScriptProgram> program) const = 0;

        virtual std::string GetGameBuild() const
        {
            return {};
        }

        virtual Pointer GetGlobal(uint32_t index) const
        {
            return {};
        }

        virtual uint32_t GetGlobalCount() const
        {
            return 0;
        }

        virtual uint32_t GetGlobalBlockCount(uint32_t block) const
        {
            return 0;
        }

        virtual std::unique_ptr<ScriptProgram> GetProgram(uint32_t hash) const
        {
            return nullptr;
        }

        virtual std::vector<std::unique_ptr<ScriptThread>> GetThreads() const
        {
            return {};
        }

        virtual std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const
        {
            return nullptr;
        }

        virtual std::string_view GetNativeNameByHash(uint64_t hash) const
        {
            return {};
        }

        virtual uint64_t GetNativeHashByHandler(uintptr_t handler) const
        {
            return 0;
        }

        virtual std::unordered_map<uint64_t, uintptr_t> GetAllNatives() const
        {
            return {};
        }

        virtual std::string GetTextLabel(uint32_t hash) const
        {
            return {};
        }
    };

    inline std::unique_ptr<Game> g_Game;
}