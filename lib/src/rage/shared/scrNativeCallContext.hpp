#pragma once
#include "Vector3.hpp"
#include "scrValue.hpp"

namespace rage::shared
{
    class scrNativeCallContext
    {
    public:
        scrNativeCallContext(scrValue* retVal, std::int32_t argCount, scrValue* args)
            : m_ReturnValue(retVal),
              m_ArgCount(argCount),
              m_Args(args),
              m_NumVectorRefs(0)
        {
        }

        void FixVectors()
        {
            for (std::int32_t i = 0; i < m_NumVectorRefs; i++)
            {
                m_VectorRefTargets[i][0].Float = m_VectorRefSources[i].x;
                m_VectorRefTargets[i][1].Float = m_VectorRefSources[i].y;
                m_VectorRefTargets[i][2].Float = m_VectorRefSources[i].z;
            }
            m_NumVectorRefs = 0;
        }

    protected:
        scrValue* m_ReturnValue;
        std::uint32_t m_ArgCount;
        scrValue* m_Args;
        std::int32_t m_NumVectorRefs;
        scrValue* m_VectorRefTargets[4];
        Vector3 m_VectorRefSources[4];
    };
    static_assert(sizeof(scrNativeCallContext) == 0x80);

    using scrNativeHandler = void (*)(scrNativeCallContext*);
}