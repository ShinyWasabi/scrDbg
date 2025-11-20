#pragma once
#include "Process.hpp"

namespace scrDbg
{
    class Pointer
    {
    public:
        Pointer(uint64_t address = 0)
            : m_Address(address)
        {
        }

        Pointer Add(size_t offset) const
        {
            return Pointer(m_Address + offset);
        }

        Pointer Sub(size_t offset) const
        {
            return Pointer(m_Address - offset);
        }

        Pointer Rip() const
        {
            return Add(Process::Read<int>(m_Address)).Add(4);
        }

        template <typename T>
        T Read() const
        {
            return Process::Read<T>(m_Address);
        }

        template <typename T>
        void Write(const T& value) const
        {
            Process::Write<T>(m_Address, value);
        }

        operator uint64_t() const
        {
            return m_Address;
        }

    private:
        uint64_t m_Address;
    };
}