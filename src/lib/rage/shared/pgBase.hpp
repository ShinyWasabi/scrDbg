#pragma once

namespace rage
{
    class pgBase
    {
    public:
        virtual ~pgBase() = default;

        void* m_Map;
    };
}