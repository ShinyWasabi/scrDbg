#pragma once

namespace scrDbgApp
{
    class GTA4 : public Game
    {
    public:
        bool Init() override;

        GameType GetType() const override
        {
            return GameType::GTA4;
        }

        const char* GetTargetBuild() const override
        {
            return "121";
        }

        bool Is64Bit() const override
        {
            return false;
        }

        std::string GetGameBuild() const override;
        int32_t GetGlobal(uint32_t index) const override;
        void SetGlobal(uint32_t index, int32_t value) const override;
        uint32_t GetGlobalBlockCount(uint32_t block) const override;
        std::unique_ptr<ScriptProgram> GetProgram(uint32_t hash) const override;
        std::vector<std::unique_ptr<ScriptThread>> GetThreads() const override;
        std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const override;
        std::string_view GetNativeNameByHash(uint64_t hash) const override;
        uint64_t GetNativeHashByHandler(uintptr_t handler) const override;
        std::unordered_map<uint64_t, uintptr_t> GetAllNatives() const override;
        std::string GetTextLabel(uint32_t hash) const override;

    protected:
        struct Pointers
        {
            Pointer GameBuild;
            Pointer ScriptThreads;
            Pointer CommandHandlers;
            Pointer ScriptPrograms;
            Pointer ScriptGlobals;
            Pointer ScriptGlobalsCount;
            Pointer TextLabels;
        };

        Pointers m_Pointers;
    };
}