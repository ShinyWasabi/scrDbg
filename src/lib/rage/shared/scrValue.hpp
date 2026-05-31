#pragma once

namespace rage
{
    union scrValue {
        int32_t Int;
        uint32_t Uns;
        float Float;
        const char* String;
        scrValue* Reference;
#if defined(_M_X64)
        uint64_t Any;
#endif
    };
}