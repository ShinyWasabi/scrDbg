#pragma once

namespace rage
{
    template <typename K, typename V>
    class atMap
    {
    public:
        struct Entry
        {
            K m_Key;
            V m_Data;
            Entry* m_Next;
        };

        Entry** m_Entries;
        uint16_t m_Size;
        uint16_t m_Capacity;
        char m_Pad[0x04];
    };
}