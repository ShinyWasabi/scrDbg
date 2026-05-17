#pragma once
#include "scrHash.hpp"
#include "scrValue.hpp"

namespace rage
{
    class scrProgram
    {
    public:
        const char* m_Name;
        uint32_t m_NameHash;
        uint8_t* m_Code;
        scrValue* m_Statics;
        uint32_t m_CodeSize;
        uint16_t m_StaticCount;
        uint16_t m_ArgCount;
        uint16_t m_RefCount;

        static inline scrHash<scrProgram*>* sm_Programs;
        static inline int32_t* sm_GlobalsCount;
        static inline scrValue** sm_Globals; // it's not paged
        static inline scrValue** sm_ProtectedGlobals;
        static inline uint32_t (*sm_GetNextProtectedSlot)();

        static bool Init();
        static scrProgram* GetProgram(uint32_t hash);
    };
}