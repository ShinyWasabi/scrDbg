#pragma once

namespace rage
{
    static void scrAssignString(char* dest, uint32_t size, const char* src)
    {
        if (!src || !size)
            return;

        const char* s = src;
        char* d = dest;
        uint32_t remaining = size;

        for (;;)
        {
            if (--remaining == 0 || !(*d++ = *s++))
                break;
        }

        *d = '\0';
    }

    static void scrAppendString(char* dest, uint32_t size, const char* src)
    {
        if (!size)
            return;

        char* d = dest;
        uint32_t remaining = size;

        while (remaining && *d)
        {
            ++d;
            --remaining;
        }

        if (remaining)
        {
            const char* s = src;
            while (--remaining && (*d++ = *s++))
                ;
            *d = '\0';
        }
    }

    static void scrItoa(char* dest, int32_t value)
    {
        char* d = dest;

        uint32_t uval;
        if (value < 0)
        {
            *d++ = '-';
            uval = static_cast<uint32_t>(-value);
        }
        else
            uval = static_cast<uint32_t>(value);

        uint32_t tmp = uval;
        int32_t digits = 0;
        do
        {
            ++digits;
            tmp /= 10;
        } while (tmp);

        d[digits] = '\0';

        while (digits--)
        {
            d[digits] = '0' + (uval % 10);
            uval /= 10;
        }
    }
}