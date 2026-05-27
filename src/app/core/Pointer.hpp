#pragma once
#include "Process.hpp"

namespace scrDbgApp
{
    class Pointer
    {
    public:
        Pointer(uintptr_t address = 0)
            : m_Address(address)
        {
        }

        Pointer Deref() const
        {
            return Pointer(Process::Read<uintptr_t>(m_Address));
        }

        Pointer Deref32() const
        {
            return Pointer(Process::Read<uint32_t>(m_Address));
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
            return Add(Process::Read<int32_t>(m_Address)).Add(4);
        }

        template <typename T>
        T Get() const
        {
            return Process::Read<T>(m_Address);
        }

        template <typename T>
        void Set(const T& value) const
        {
            Process::Write<T>(m_Address, value);
        }

        template <typename T>
        T GetArray(size_t index) const
        {
            return Process::Read<T>(m_Address + index * sizeof(T));
        }

        template <typename T>
        void SetArray(size_t index, const T& value) const
        {
            Process::Write<T>(m_Address + index * sizeof(T), value);
        }

        template <typename T>
        bool GetBuffer(T* out, size_t size) const
        {
            return Process::ReadRaw(m_Address, out, size);
        }

        template <typename T>
        bool SetBuffer(const T* data, size_t size) const
        {
            return Process::WriteRaw(m_Address, data, size);
        }

        std::string GetString(size_t maxLength = 1024) const
        {
            std::vector<char> buffer(maxLength);

            if (!Process::ReadRaw(m_Address, buffer.data(), buffer.size()))
                return {};

            size_t len = 0;
            while (len < buffer.size() && buffer[len] != '\0')
                len++;

            return std::string(buffer.data(), len);
        }

        operator uintptr_t() const
        {
            return m_Address;
        }

    private:
        uintptr_t m_Address;
    };
}