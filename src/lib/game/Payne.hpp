#pragma once
#include "rage/payne/scrCommand.hpp"

#if defined(_M_IX86)

namespace rage
{
    template <typename T /*, int DefaultSize*/>
    class scrHash;
    template <typename T>
    class atArray;
    union scrValue;

    namespace payne
    {
        class scrProgram;
        class scrThread;
    }
}

namespace scrDbgLib
{
    class Payne : public Game
    {
    public:
        struct Pointers;

        explicit Payne();

        bool InitPointers() const override;
        bool InitHooks() const override;

        GameType GetType() const override
        {
            return GameType::PAYNE;
        }

        int GetResourceId() const override
        {
            return NATIVES_PAYNE_BIN;
        }

        static const Pointers& GetPointers()
        {
            return m_Pointers;
        }

    private:
        struct Pointers
        {
            rage::scrHash<rage::payne::scrProgram*>* ScriptPrograms;
            int32_t* ScriptGlobalsCount;
            rage::scrValue** ScriptGlobals;
            rage::payne::scrThread** CurrentScriptThread;
            const char** CurrentScriptThreadName;
            rage::atArray<rage::payne::scrThread*>* ScriptThreads;
            rage::scrHash<rage::payne::scrCommand::Context::Handler>* CommandHandlers;
            void* RunScriptThread;
            bool* TimerUserPause;
            bool* TimerScriptPause;
        };

        static inline Pointers m_Pointers;
    };
}

#endif