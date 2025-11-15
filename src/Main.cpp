#include "GUI/GUI.hpp"
#include "Pointers.hpp"
#include "core/PipeClient.hpp"
#include "game/gta/Natives.hpp"
#include "pipe/PipeCommands.hpp"
#include "util/Misc.hpp"

int main(int argc, char* argv[])
{
    using namespace scrDbg;

    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("WindowsVista"));

    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Global\\scrDbg");
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        QMessageBox::warning(nullptr, "scrDbg", "scrDbg is already running.");
        return 1;
    }

    if (!Process::Init(L"GTA5.exe"))
    {
        if (!Process::Init(L"GTA5_Enhanced.exe"))
        {
            QMessageBox::critical(nullptr, "Process", "Failed to initialize process. Please make sure BattlEye is disabled and the game is running.");
            return 1;
        }
        g_IsEnhanced = true;
    }

    if (!g_Pointers.Init())
    {
        std::string message = "Failed to initialize pointers.";

        const auto target = g_IsEnhanced ? ENHANCED_TARGET_BUILD : LEGACY_TARGET_BUILD;
        const auto build = Misc::GetGameBuild();
        if (!build.empty() && build != target)
        {
            char note[256]{};
            std::snprintf(note, sizeof(note), "\nNote: Expected game build %s but got %s.", target, build.c_str());
            message += note;
        }

        QMessageBox::critical(nullptr, "Pointers", message.c_str());
        return 1;
    }

    if (Process::IsModuleLoaded(L"ScriptVM.asi"))
    {
        if (PipeClient::Init("scrDbg"))
        {
            g_BreakpointsSupported = true;
        }
        else
        {
            QMessageBox::warning(nullptr, "Pipe", "Failed to initialize scrDbg pipe.");
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Module Not Found", "ScriptVM.asi not found. Some features will be disabled.");
    }

    if (!gta::Natives::Init())
        QMessageBox::warning(nullptr, "Natives", "Failed to load native names.");

    GUIWidget gui;
    gui.show();
    int ret = app.exec();

    Process::Destroy();
    PipeCommands::RemoveAllBreakpoints();
    PipeCommands::SetBreakpointPauseGame(false);
    PipeClient::Destroy();

    return ret;
}