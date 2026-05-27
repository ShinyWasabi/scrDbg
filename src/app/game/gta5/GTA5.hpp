#pragma once

namespace scrDbgApp
{
    class GTA5 : public Game
    {
    public:
        std::unique_ptr<Disassembler> CreateDisassembly(std::unique_ptr<ScriptProgram> program) const override;

        std::string GetGameBuild() const override;
        int32_t GetGlobal(uint32_t index) const override;
        void SetGlobal(uint32_t index, int32_t value) const override;
        uint32_t GetGlobalBlockCount(uint32_t block) const override;
        std::unique_ptr<ScriptProgram> GetProgram(uint32_t hash) const override;
        std::string_view GetNativeNameByHash(uint64_t hash) const override;
        uint64_t GetNativeHashByHandler(uintptr_t handler) const override;
        std::unordered_map<uint64_t, uintptr_t> GetAllNatives() const override;
        std::string GetTextLabel(uint32_t hash) const override;

    protected:
        struct Pointers
        {
            Pointer GameBuild;
            Pointer OnlineBuild;
            Pointer ScriptThreads;
            Pointer ScriptPrograms;
            Pointer ScriptGlobals;
            Pointer ScriptGlobalBlockCounts;
            Pointer NativeRegistrationTable;
            Pointer TextLabels;
        };

        Pointers m_Pointers;
    };

    class GTA5_GEN9 : public GTA5
    {
    public:
        bool Init() override;

        GameType GetType() const override
        {
            return GameType::GTA5_GEN9;
        }

        const char* GetTargetBuild() const override
        {
            return "1013.34-1.72";
        }

        bool Is64Bit() const override
        {
            return true;
        }

        std::vector<std::unique_ptr<ScriptThread>> GetThreads() const override;
        std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const override;
    };

    class GTA5_GEN8 : public GTA5
    {
    public:
        bool Init() override;

        GameType GetType() const override
        {
            return GameType::GTA5_GEN8;
        }

        const char* GetTargetBuild() const override
        {
            return "3788.0-1.72";
        }

        bool Is64Bit() const override
        {
            return true;
        }

        std::vector<std::unique_ptr<ScriptThread>> GetThreads() const override;
        std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const override;
    };
}