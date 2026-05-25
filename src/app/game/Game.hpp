#pragma once
#include "Types.hpp"

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
        virtual bool Is64Bit() const = 0;

        virtual std::string GetGameBuild() const = 0;
        virtual int32_t GetGlobal(uint32_t index) const = 0;
        virtual void SetGlobal(uint32_t index, int32_t value) const = 0;
        virtual uint32_t GetGlobalBlockCount(uint32_t block) const = 0;
        virtual std::unique_ptr<ScriptProgram> GetProgram(uint32_t hash) const = 0;
        virtual std::vector<std::unique_ptr<ScriptThread>> GetThreads() const = 0;
        virtual std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const = 0;
        virtual std::string_view GetNativeNameByHash(uint64_t hash) const = 0;
        virtual uint64_t GetNativeHashByHandler(uintptr_t handler) const = 0;
        virtual std::unordered_map<uint64_t, uintptr_t> GetAllNatives() const = 0;
        virtual std::string GetTextLabel(uint32_t hash) const = 0;
    };

    inline std::unique_ptr<Game> g_Game;
}