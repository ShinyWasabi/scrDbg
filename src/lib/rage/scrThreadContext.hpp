#pragma once

namespace rage
{
    enum class scrThreadState : uint32_t
    {
        RUNNING,
        IDLE,
        KILLED,
        PAUSED
    };

    enum class scrThreadPriority : uint32_t
    {
        HIGHEST,
        NORMAL,
        LOWEST,
        MANUAL_UPDATE = 100
    };

    class scrThreadContext
    {
    public:
        uint32_t* Id()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_Id : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_Id;
        }

        uint32_t* ProgramHash()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_ProgramHash : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_ProgramHash;
        }

        scrThreadState* State()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_State : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_State;
        }

        uint32_t* ProgramCounter()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_ProgramCounter : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_ProgramCounter;
        }

        uint32_t* FramePointer()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_FramePointer : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_FramePointer;
        }

        uint32_t* StackPointer()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_StackPointer : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_StackPointer;
        }

        float* TimerA()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_TimerA : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_TimerA;
        }

        float* TimerB()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_TimerB : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_TimerB;
        }

        float* WaitTimer()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_WaitTimer : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_WaitTimer;
        }

        uint32_t* StackSize()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_StackSize : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_StackSize;
        }

        uint32_t* CatchProgramCounter()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_CatchProgramCounter : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_CatchProgramCounter;
        }

        uint32_t* CatchFramePointer()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_CatchFramePointer : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_CatchFramePointer;
        }

        uint32_t* CatchStackPointer()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_CatchStackPointer : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_CatchStackPointer;
        }

        scrThreadPriority* Priority()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_Priority : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_Priority;
        }

        uint8_t* CallDepth()
        {
            return scrDbgLib::g_IsEnhanced ? &reinterpret_cast<scrThreadContext_GEN9*>(this)->m_CallDepth : &reinterpret_cast<scrThreadContext_GEN8*>(this)->m_CallDepth;
        }

        uint32_t* Callstack()
        {
            return scrDbgLib::g_IsEnhanced ? reinterpret_cast<scrThreadContext_GEN9*>(this)->m_Callstack : reinterpret_cast<scrThreadContext_GEN8*>(this)->m_Callstack;
        }

    private:
        struct scrThreadContext_GEN8
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            scrThreadState m_State;
            uint32_t m_ProgramCounter;
            uint32_t m_FramePointer;
            uint32_t m_StackPointer;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTimer;
            char m_Pad1[0x2C];
            uint32_t m_StackSize;
            uint32_t m_CatchProgramCounter;
            uint32_t m_CatchFramePointer;
            uint32_t m_CatchStackPointer;
            scrThreadPriority m_Priority;
            uint8_t m_CallDepth;
            uint32_t m_Callstack[16];
        };
        static_assert(sizeof(scrThreadContext_GEN8) == 0xA8);

        struct scrThreadContext_GEN9
        {
            uint32_t m_Id;
            char m_Pad1[0x04];
            uint32_t m_ProgramHash;
            char m_Pad2[0x04];
            scrThreadState m_State;
            uint32_t m_ProgramCounter;
            uint32_t m_FramePointer;
            uint32_t m_StackPointer;
            float m_TimerA;
            float m_TimerB;
            float m_WaitTimer;
            char m_Pad3[0x2C];
            uint32_t m_StackSize;
            uint32_t m_CatchProgramCounter;
            uint32_t m_CatchFramePointer;
            uint32_t m_CatchStackPointer;
            scrThreadPriority m_Priority;
            uint8_t m_CallDepth;
            uint32_t m_Callstack[16];
        };
        static_assert(sizeof(scrThreadContext_GEN9) == 0xB0);
    };
}