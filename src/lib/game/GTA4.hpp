#pragma once

#if defined(_M_IX86)

namespace rage
{
    template <typename T /*, int DefaultSize*/>
    class scrHash;
    class scrProgram;
    union scrValue;
    class scrThread;
    template <typename T>
    class atArray;
    class scrNativeContext;
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

        Debugger* GetDebugger() const override
        {
            return m_Debugger.get();
        }

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
            rage::scrHash<rage::scrProgram*>* ScriptPrograms;
            int32_t* ScriptGlobalsCount;
            rage::scrValue** ScriptGlobals; // it's not paged
            uint8_t** ProtectedScriptGlobals;
            uint32_t (*GetNextProtectedScriptSlot)();
            void* ScriptProgramCtor;
            void* ScriptProgramDtor;
            rage::scrThread** CurrentScriptThread;
            rage::atArray<rage::scrThread*>* ScriptThreads;
            rage::scrHash<void (*)(rage::scrNativeContext*)>* CommandHandlers;
            void* RunScriptThread;
            bool* TimerUserPause;
            bool* TimerScriptPause;
        };

        static inline Pointers m_Pointers;
    };
}

#endif