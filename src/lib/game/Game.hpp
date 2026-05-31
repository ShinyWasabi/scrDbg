#pragma once
#include "debugger/Debugger.hpp"

namespace scrDbgLib
{
    enum class GameType
    {
        GTA4,
        GTA5_GEN8,
        GTA5_GEN9,
    };

    class Game
    {
    public:
        virtual ~Game() = default;

        virtual bool InitPointers() const = 0;
        virtual bool InitHooks() const = 0;
        virtual Debugger* GetDebugger() const = 0;
        virtual GameType GetType() const = 0;
        virtual int GetResourceId() const = 0;

        static void Cleanup(const std::string& message = {});

    protected:
        std::unique_ptr<Debugger> m_Debugger;
    };

    inline std::unique_ptr<Game> g_Game;
}