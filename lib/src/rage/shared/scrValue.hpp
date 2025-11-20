#pragma once

namespace rage::shared
{
    union scrValue {
        std::int32_t Int;
        std::uint32_t Uns;
        float Float;
        const char* String;
        scrValue* Reference;
        std::uint64_t Any;
    };
}