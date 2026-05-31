#pragma once

namespace rage
{
    template <typename T>
    class atArray
    {
    public:
        atArray()
            : m_Data(nullptr),
              m_Size(0),
              m_Capacity(0)
        {
        }

        T* begin() const
        {
            return &m_Data[0];
        }

        T* end() const
        {
            return &m_Data[m_Size];
        }

        T* data() const
        {
            return m_Data;
        }

        uint16_t size() const
        {
            return m_Size;
        }

        uint16_t capacity() const
        {
            return m_Capacity;
        }

        T& operator[](uint16_t index) const
        {
            return m_Data[index];
        }

        T* m_Data;
        uint16_t m_Size;
        uint16_t m_Capacity;
#if defined(_M_X64)
        char m_Pad1[0x04];
#endif
    };
}