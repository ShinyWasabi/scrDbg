#include "GTA4.hpp"
#include "core/Scanner.hpp"

namespace scrDbgApp
{
    bool GTA4::Init()
    {
        Scanner scanner;

        scanner.Add("GameBuild", "68 ? ? ? ? 68 ? ? ? ? 50 E8 ? ? ? ? 83 C4 ? B8", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(1).Get<uint32_t>()).Deref();
        });

        scanner.Add("ScriptThreads", "8B 15 ? ? ? ? 0F B7 C7", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(2).Get<uint32_t>()).Deref();
        });

        scanner.Add("CommandHandlers", "C7 05 ? ? ? ? 00 00 00 00 66 3B C2", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(2).Get<uint32_t>()).Deref();
        });

        scanner.Add("ScriptPrograms", "8B 3D ? ? ? ? 8B CE ? ? ? 85 C0", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(2).Get<uint32_t>()).Deref();
        });

        scanner.Add("ScriptGlobals", "8B 15 ? ? ? ? 57 64 8B 3D", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(2).Get<uint32_t>()).Deref();
        });

        scanner.Add("ScriptGlobalsCount", "A3 ? ? ? ? 33 DB BF", [this](Pointer ptr) {
            m_Pointers.ScriptThreads = Pointer(ptr.Add(1).Get<uint32_t>()).Deref();
        });

        scanner.Add("TextLabels", "B9 ? ? ? ? 4F E8", [this](Pointer ptr) {
            m_Pointers.TextLabels = Pointer(ptr.Add(1).Get<uint32_t>()).Deref();
        });

        return scanner.Scan();
    }

    std::string GTA4::GetGameBuild() const
    {
        return "";
    }

    int32_t GTA4::GetGlobal(uint32_t index) const
    {
        return 0;
    }

    void GTA4::SetGlobal(uint32_t index, int32_t value) const
    {
    }

    uint32_t GTA4::GetGlobalBlockCount(uint32_t block) const
    {
        return 0;
    }

    std::unique_ptr<ScriptProgram> GTA4::GetProgram(uint32_t hash) const
    {
        return nullptr;
    }

    std::vector<std::unique_ptr<ScriptThread>> GTA4::GetThreads() const
    {
        return {};
    }

    std::unique_ptr<ScriptThread> GTA4::GetThread(uint32_t hash) const
    {
        return nullptr;
    }

    std::string_view GTA4::GetNativeNameByHash(uint64_t hash) const
    {
        return "";
    }

    uint64_t GTA4::GetNativeHashByHandler(uintptr_t handler) const
    {
        return 0;
    }

    std::unordered_map<uint64_t, uintptr_t> GTA4::GetAllNatives() const
    {
        return {};
    }

    std::string GTA4::GetTextLabel(uint32_t hash) const
    {
        return "";
    }
}