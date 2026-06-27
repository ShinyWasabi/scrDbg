#pragma once

namespace scrDbgApp
{
    class Payne : public Game
    {
    public:
        bool Init() override;

        GameType GetType() const override
        {
            return GameType::PAYNE;
        }

        const char* GetTargetBuild() const override
        {
            return "216";
        }

        int GetResourceId() const override
        {
            return NATIVES_PAYNE_BIN;
        }

        bool Is64Bit() const override
        {
            return false;
        }

        std::unique_ptr<Disassembler> CreateDisassembly(std::unique_ptr<ScriptProgram> program) const override;

        std::string GetGameBuild() const override;
        Pointer GetGlobal(uint32_t index) const override;
        uint32_t GetGlobalCount() const override;
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
            Pointer PTScriptThreads;
            Pointer CommandHandlers;
            Pointer ScriptPrograms;
            Pointer ScriptGlobals;
            Pointer ScriptGlobalsCount;
            Pointer TextLabels;
        };

        Pointers m_Pointers;
    };
}