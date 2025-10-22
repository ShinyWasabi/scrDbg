#include "GUI/GUI.hpp"
#include "Pointers.hpp"
#include "PipeClient.hpp"
#include "PipeCommands.hpp"

namespace scrDbg
{
    static std::string GetGameBuild()
    {
        if (!g_Pointers.GameBuild || !g_Pointers.OnlineBuild)
            return {};

        char gameBuild[64]{};
        Process::ReadRaw(g_Pointers.GameBuild, gameBuild, sizeof(gameBuild));

        char onlineBuild[64]{};
        Process::ReadRaw(g_Pointers.OnlineBuild, onlineBuild, sizeof(onlineBuild));

        char result[128]{};
        std::snprintf(result, sizeof(result), "%s-%s", gameBuild, onlineBuild);
        return result;
    }
}

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
        std::string message = "Failed to initialize pointers. Please make sure BattlEye is disabled.";

        const auto target = g_IsEnhanced ? ENHANCED_TARGET_BUILD : LEGACY_TARGET_BUILD;
        const auto build = GetGameBuild();
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

    GUIWidget gui;
    gui.setWindowIcon(QIcon(":/resources/scrDbg.ico"));
    gui.show();
    int ret = app.exec();

    Process::Destroy();
    PipeCommands::RemoveAllBreakpoints();
    PipeClient::Destroy();

    return ret;
}