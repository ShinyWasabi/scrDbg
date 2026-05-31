#include "Game.hpp"
#include "core/Hooking.hpp"
#include "core/PipeServer.hpp"
#include "debugger/VMLogger.hpp"

namespace scrDbgLib
{
    void Game::Cleanup(const std::string& message)
    {
        if (!message.empty())
            MessageBoxA(0, message.c_str(), "Error", MB_ICONERROR);

        if (g_Game && g_Game->m_Debugger)
        {
            g_Game->m_Debugger->RemoveAllBreakpoints();
            g_Game->m_Debugger->SetPauseGameOnBreakpoint(false);
        }

        PipeServer::Destroy();
        VMLogger::Destroy();
        Hooking::Destroy();

        CloseHandle(g_MainThread);
        FreeLibraryAndExitThread(g_DllInstance, 0);
    }
}