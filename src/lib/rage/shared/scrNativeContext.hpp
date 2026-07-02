#pragma once
#include "Vector3.hpp"
#include "scrValue.hpp"

namespace rage
{
    struct scrNativeContext
    {
        using Handler = void (*)(scrNativeContext*);

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

        // RDR1 specific
        void CopyVectorsOut2()
        {
            for (int i = 0; i < m_VectorRefCount; i++)
            {
                uint8_t* base = reinterpret_cast<uint8_t*>(m_VectorRefOutput[i]);
                *reinterpret_cast<float*>(base + 0) = m_VectorRefInput[i].x;
                *reinterpret_cast<float*>(base + 4) = m_VectorRefInput[i].y;
                *reinterpret_cast<float*>(base + 8) = m_VectorRefInput[i].z;
            }
            m_VectorRefCount = 0;
        }
    };
}