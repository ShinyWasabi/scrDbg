#pragma once
#include "Vector3.hpp"
#include "atArray.hpp"
#include "scrHash.hpp"
#include "scrOpcode.hpp"
#include "scrValue.hpp"

namespace rage
{
    class scrThread
    {
    public:
        enum class State : uint32_t
        {
            RUNNING = 0U,
            WAITING = 1U,
            KILLED = 2U,
            PAUSED = 3U,
            REFRESH = 4U
        };

        struct Context
        {
            uint32_t m_Id;
            uint32_t m_ProgramHash;
            State m_State;
            uint32_t m_Pc;
            uint32_t m_Fp;
            uint32_t m_Sp;
            uint32_t m_TimerA;
            uint32_t m_TimerB;
            uint32_t m_TimerC;
            uint32_t m_WaitTime;
            uint32_t m_MinPc;
            uint32_t m_MaxPc;
            scrValue m_Tls[4];
            uint32_t m_StackSize;
            uint32_t m_CatchPc;
            uint32_t m_CatchFp;
            uint32_t m_CatchSp;
            int32_t m_ProtectedSp;
        };

        struct ThreadStack
        {
            scrThread* m_Thread;
            uint32_t m_StackSize;
            scrValue* m_Stack;
        };

        struct NativeContext
        {
            using Handler = void(__cdecl*)(NativeContext*);

            scrValue* m_Rets;
            uint32_t m_ArgCount;
            scrValue* m_Args;
            int32_t m_VectorRefCount;
            scrValue* m_VectorRefOutput[4];
            Vector3 m_VectorRefInput[4];

            void CopyVectorsOut()
            {
                for (int i = 0; i < m_VectorRefCount; i++)
                {
                    m_VectorRefOutput[i][0].Float = m_VectorRefInput[i].x;
                    m_VectorRefOutput[i][1].Float = m_VectorRefInput[i].y;
                    m_VectorRefOutput[i][2].Float = m_VectorRefInput[i].z;
                }
                m_VectorRefCount = 0;
            }
        };

        virtual ~scrThread() = default;
        virtual void Reset(uint32_t programHash, void* args, uint32_t argsSize) = 0;
        virtual State Run(int32_t insnCount) = 0;
        virtual State Update(int32_t insnCount) = 0;
        virtual void Kill() = 0;

        Context m_Context;
        scrValue* m_Stack;
        uint32_t m_InsnCount;
        scrValue* m_ProtectedStack;
        int32_t m_ProtectedSlotIndex;
        scrValue* m_ProtectedTempStack;
        const char* m_KillReason;
        char m_ScriptName[24]; // This actually belongs to GtaThread in GTA IV

        static inline scrThread** sm_CurrentThread;
        static inline atArray<scrThread*>* sm_Threads;
        static inline atArray<ThreadStack*>* sm_Stacks;
        static inline scrHash<NativeContext::Handler>* sm_CommandHandlers;
        static inline int32_t sm_NullContainer;

        State OnException(uint32_t pc, scrOpcode op, const char* fmt, ...);

        static bool Init();
        static scrThread* GetThreadByHash(uint32_t hash);
        static scrThread* GetThreadById(uint32_t id);
        static ThreadStack* GetThreadStack(uint32_t stackSize);
        static NativeContext::Handler GetCommandHandler(uint32_t hash);
        static State __fastcall RunThread(scrThread* _this, void* edx, int32_t insnCount);
    };
}