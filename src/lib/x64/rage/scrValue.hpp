#pragma once

namespace rage
{
    union scrValue {
        std::int32_t Int;
        std::uint32_t Uns;
        float Float;
        const char* String;
        scrValue* Reference;
        std::uint64_t Any;
    };
    static_assert(sizeof(scrValue) == 0x08);
}