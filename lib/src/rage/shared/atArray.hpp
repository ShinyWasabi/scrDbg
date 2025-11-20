#pragma once

namespace rage::shared
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

        std::uint16_t size() const
        {
            return m_Size;
        }

        std::uint16_t capacity() const
        {
            return m_Capacity;
        }

        T& operator[](std::uint16_t index) const
        {
            return m_Data[index];
        }

        bool contains(T comparator)
        {
            for (auto iter_value : this)
            {
                if (iter_value == comparator)
                {
                    return true;
                }
            }
            return false;
        }

    public:
        T* m_Data;
        std::uint16_t m_Size;
        std::uint16_t m_Capacity;
    };
    static_assert(sizeof(atArray<std::uint32_t>) == 0x10);
}