#pragma once

namespace rage
{
    template <typename T /*, int DefaultSize*/>
    class scrHash
    {
    public:
        struct Entry
        {
            uint32_t m_Hash;
            T m_Data;
        };

        Entry* m_Entries;
        uint32_t m_Size;
        uint32_t m_DefaultSize;
        uint32_t m_Used;

    public:
        // void Init(int size = DefaultSize);

        bool Insert(uint32_t hash, T value)
        {
            if (m_Used == m_Size)
                return false;

            uint32_t index = hash % m_Size;
            uint32_t probe = hash;

            if (m_Entries[index].m_Hash > 1)
            {
                while (m_Entries[index].m_Hash != hash)
                {
                    probe = (probe >> 1) + 1;
                    index = (index + probe) % m_Size;

                    if (m_Entries[index].m_Hash <= 1)
                        break;
                }

                if (m_Entries[index].m_Hash == hash)
                    return false;
            }

            m_Entries[index].m_Hash = hash;
            m_Entries[index].m_Data = value;
            ++m_Used;

            return true;
        }

        T Lookup(uint32_t hash)
        {
            if (!m_Size)
                return {};

            uint32_t index = hash % m_Size;
            uint32_t probe = hash;
            uint32_t current = m_Entries[index].m_Hash;

            if (current != hash)
            {
                while (current)
                {
                    probe = (probe >> 1) + 1;
                    index = (index + probe) % m_Size;
                    current = m_Entries[index].m_Hash;

                    if (current == hash)
                        break;
                }

                if (!current)
                    return {};
            }

            return m_Entries[index].m_Data;
        }

        bool Remove(uint32_t hash)
        {
            uint32_t index = hash % m_Size;
            uint32_t probe = hash;

            if (!m_Entries[index].m_Hash)
                return false;

            while (m_Entries[index].m_Hash != hash)
            {
                probe = (probe >> 1) + 1;
                index = (index + probe) % m_Size;

                if (!m_Entries[index].m_Hash)
                    return false;
            }

            m_Entries[index].m_Hash = 1;
            m_Entries[index].m_Data = {};
            --m_Used;

            return true;
        }
    };
}