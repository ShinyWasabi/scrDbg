#include "ResourceLoader.hpp"
#include "core/PipeClient.hpp"
#include "game/gta4/GTA4.hpp"
#include "game/gta5/GTA5.hpp"
#include "gui/widgets/GUIWidget.hpp"

int main(int argc, char* argv[])
{
    using namespace scrDbgApp;

    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("WindowsVista"));

    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Global\\scrDbg");
    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        QMessageBox::warning(nullptr, "scrDbg", "scrDbg is already running.");
        return 1;
    }

    for (const auto& game : g_Games)
    {
        if (Process::Init(game.second))
        {
            switch (game.first)
            {
            case GameType::GTA4:
                g_Game = std::make_unique<GTA4>();
                break;
            case GameType::GTA5_GEN8:
                g_Game = std::make_unique<GTA5_GEN8>();
                break;
            case GameType::GTA5_GEN9:
                g_Game = std::make_unique<GTA5_GEN9>();
                break;
            }
            break;
        }
    }

    if (!g_Game)
    {
        QMessageBox::critical(nullptr, "Process", "Failed to attach to a supported game process. Please make sure the game is running.");
        return 1;
    }

    if (!g_Game->Init())
    {
        std::string message = "Failed to initialize pointers.";

        const auto target = g_Game->GetTargetBuild();
        const auto build = g_Game->GetGameBuild();
        if (!build.empty() && build != target)
        {
            char note[256]{};
            std::snprintf(note, sizeof(note), "\nNote: Expected game build %s but got %s.", target, build.c_str());
            message += note;
        }

        QMessageBox::critical(nullptr, "Pointers", message.c_str());
        return 1;
    }

    if (!Process::IsModuleLoaded(g_Game->Is64Bit() ? L"scrDbg-x64.dll" : L"scrDbg-x86.dll"))
    {
        if (!Process::InjectModule(g_Game->Is64Bit() ? "scrDbg-x64.dll" : "scrDbg-x86.dll"))
        {
            QMessageBox::critical(nullptr, "Injection Failed", "Failed to inject scrDbg.dll.");
            return 1;
        }
    }

    if (!PipeClient::Init("scrDbg"))
    {
        QMessageBox::critical(nullptr, "Pipe", "Failed to initialize scrDbg pipe client.");
        return 1;
    }

    if (!scrDbgShared::NativesBin::Load(GetModuleHandle(0), g_Game->GetResourceId()))
        QMessageBox::warning(nullptr, "Resources", "Failed to load natives database.");

    GUIWidget gui;
    gui.show();
    int ret = app.exec();

    PipeClient::Destroy();
    Process::Destroy();
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    g_Game.reset();

    return ret;
}