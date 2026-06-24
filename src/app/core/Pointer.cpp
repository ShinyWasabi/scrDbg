#include "Pointer.hpp"

namespace scrDbgApp
{
    Pointer Pointer::Deref() const
    {
        return Pointer(g_Game->Is64Bit() ? Process::Read<uintptr_t>(m_Address) : Process::Read<uint32_t>(m_Address));
    }
}