#include "GUI/GUI.hpp"
#include "Pointers.hpp"
#include "ResourceLoader.hpp"
#include "core/PipeClient.hpp"
#include "util/Misc.hpp"

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

    if (!Process::IsModuleLoaded(L"scrDbg.dll"))
    {
        if (!Process::InjectModule("scrDbg.dll"))
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

    if (!scrDbgShared::NativesBin::Load(GetModuleHandle(0)))
        QMessageBox::warning(nullptr, "Resources", "Failed to load native names.");

    GUIWidget gui;
    gui.show();
    int ret = app.exec();

    PipeClient::Destroy();
    Process::Destroy();
    ReleaseMutex(mutex);
    CloseHandle(mutex);

    return ret;
}