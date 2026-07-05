#pragma once

namespace scrDbgApp
{
    class RDR3 : public Game
    {
    public:
        bool Init() override;

        GameType GetType() const override
        {
            return GameType::RDR3;
        }

        const char* GetTargetBuild() const override
        {
            return "1491.50_dev_live_tu";
        }

        int GetResourceId() const override
        {
            return NATIVES_RDR3_BIN;
        }

        bool Is64Bit() const override
        {
            return true;
        }

        std::unique_ptr<Disassembler> CreateDisassembly(std::unique_ptr<ScriptProgram> program) const override;

        std::string GetGameBuild() const override;
        Pointer GetGlobal(uint32_t index) const override;
        uint32_t GetGlobalBlockCount(uint32_t block) const override;
        std::unique_ptr<ScriptProgram> GetProgram(uint32_t hash) const override;
        std::vector<std::unique_ptr<ScriptThread>> GetThreads() const override;
        std::unique_ptr<ScriptThread> GetThread(uint32_t hash) const override;
        std::string_view GetNativeNameByHash(uint64_t hash) const override;
        uint64_t GetNativeHashByHandler(uintptr_t handler) const override;
        std::unordered_map<uint64_t, uintptr_t> GetAllNatives() const override;
        std::string GetTextLabel(uint32_t hash) const override;

    private:
        static uint64_t RotL64(uint64_t value, int shift)
        {
            shift &= 63;
            return (value << shift) | (value >> (64 - shift));
        }

        static uint64_t RotR64(uint64_t value, int shift)
        {
            shift &= 63;
            return (value >> shift) | (value << (64 - shift));
        }

        static uint64_t UnscrambleNativeKey(uint64_t key, uint64_t seed)
        {
            uint64_t v2 = RotL64(RotR64(seed, 5), 32);
            uint64_t rotated = RotL64(key ^ v2, (v2 & 0x1F) + 1);
            return ~RotR64(rotated, 32);
        }

    protected:
        struct Pointers
        {
            Pointer GameBuild;
            Pointer ScriptThreads;
            Pointer ScriptPrograms;
            Pointer ScriptGlobals;
            Pointer ScriptGlobalBlockCounts;
            Pointer NativeRegistrationSeed;
            Pointer NativeRegistrationTable;
            Pointer TextLabels;
        };

        Pointers m_Pointers;
    };
}