#pragma once

#if defined(_M_IX86)

namespace rage
{
    template <typename T /*, int DefaultSize*/>
    class scrHash;
    template <typename T>
    class atArray;
    class scrNativeContext;
    union scrValue;

    namespace gta4
    {
        class scrProgram;
        class scrThread;
    }
}

namespace scrDbgLib
{
    class GTA4 : public Game
    {
    public:
        struct Pointers;

        explicit GTA4();

        bool InitPointers() const override;
        bool InitHooks() const override;
        void* GetNativeHandler(uint64_t hash) const override;
        uint64_t GetNativeHash(void* handler) const override;

        GameType GetType() const override
        {
            return GameType::GTA4;
        }

        int GetResourceId() const override
        {
            return NATIVES_GTA4_BIN;
        }

        static const Pointers& GetPointers()
        {
            return m_Pointers;
        }

    private:
        struct Pointers
        {
            rage::scrHash<rage::gta4::scrProgram*>* ScriptPrograms;
            int32_t* ScriptGlobalsCount;
            rage::scrValue** ScriptGlobals; // it's not paged
            uint8_t** ProtectedScriptGlobals;
            uint32_t (*GetNextProtectedScriptSlot)();
            rage::gta4::scrThread** CurrentScriptThread;
            rage::atArray<rage::gta4::scrThread*>* ScriptThreads;
            rage::scrHash<void (*)(rage::scrNativeContext*)>* CommandHandlers;
            void* RunScriptThread;
            bool* TimerUserPause;
            bool* TimerScriptPause;
        };

        static inline Pointers m_Pointers;
    };
}

#endif