#pragma once
#include "rage/shared/Vector3.hpp"
#include "rage/shared/scrValue.hpp"

#if defined(_M_IX86)

namespace rage::payne
{
    class scrThread;

    class scrCommand
    {
    public:
        struct Context
        {
            using Handler = void (*)(Context*);

            scrThread* m_Thread;
            scrValue* m_Rets;
            uint32_t m_ArgCount;
            scrValue* m_Args;
            scrCommand* m_Command;
            int32_t m_VectorRefCount;
            scrValue* m_VectorRefOutput[4];
            char m_Pad1[8]; // IDK what this is
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

        // Known values: 0, 1, 2, 3, 6, 7, 11, 16
        // bit 2: requires synchronized execution via InvokeSynchronizedCommand
        // only relevant when thread m_Context.m_TypedFlags bits 0-1 == 2 (flags 6 and 7 are the only values with this bit set)
        uint32_t m_Flags;
        Context::Handler m_Handler;
    };
}

#endif