#pragma once

#if defined(_M_X64)

namespace rage
{
    template <typename T /*, int DefaultSize*/>
    class scrHash;
    template <typename T>
    class atArray;
    class scrNativeContext;
    union scrValue;

    namespace rdr2
    {
        class scrProgram;
        class scrThread;
    }
}

namespace scrDbgLib
{
    class RDR2 : public Game
    {
    public:
        struct Pointers;

        explicit RDR2();

        bool InitPointers() const override;
        bool InitHooks() const override;

        GameType GetType() const override
        {
            return GameType::RDR2;
        }

        int GetResourceId() const override
        {
            return NATIVES_RDR2_BIN;
        }

        static const Pointers& GetPointers()
        {
            return m_Pointers;
        }

    private:
        struct Pointers
        {
            rage::scrHash<rage::rdr2::scrProgram*>* ScriptPrograms;
            rage::scrValue** ScriptGlobals; // addressed in 4-byte slots although scrValue is 8 bytes
            rage::rdr2::scrThread** CurrentScriptThread;
            const char** CurrentScriptThreadName;
            rage::atArray<rage::rdr2::scrThread*>* ScriptThreads;
            rage::scrHash<void (*)(rage::scrNativeContext*)>* CommandHandlers;
            void* RunScriptThread;
            uint32_t* PauseGameFlags;
        };

        static inline Pointers m_Pointers;
    };
}

#endif