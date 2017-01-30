#include "memsafe.h"

void *memsafe::memcpy_safe(void *dest, const void *src, size_t stLen)
{
    if (dest == nullptr || src == nullptr || stLen == 0)
        return nullptr;

    MEMORY_BASIC_INFORMATION	mbi;
    VirtualQuery(dest, &mbi, sizeof(mbi));
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

    void	*pvRetn = memcpy(dest, src, stLen);
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);
    FlushInstructionCache(GetCurrentProcess(), dest, stLen);
    return pvRetn;
}

int memsafe::memcmp_safe(const void *_s1, const void *_s2, uint32_t len)
{
    const uint8_t	*s1 = (const uint8_t *)_s1;
    const uint8_t	*s2 = (const uint8_t *)_s2;
    uint8_t			buf[4096];

    for (;;)
    {
        if (len > 4096)
        {
            if (!memcpy_safe(buf, s1, 4096))
                return 0;
            if (memcmp(buf, s2, 4096))
                return 0;
            s1 += 4096;
            s2 += 4096;
            len -= 4096;
        }
        else
        {
            if (!memcpy_safe(buf, s1, len))
                return 0;
            if (memcmp(buf, s2, len))
                return 0;
            break;
        }
    }

    return 1;
}

int memsafe::memset_safe(void *_dest, int c, DWORD len)
{
    byte *dest = (byte *)_dest;
    byte buf[4096];
    memset(buf, c, (len > 4096) ? 4096 : len);
    for (;;)
    {
        if (len > 4096)
        {
            if (!memcpy_safe(dest, buf, 4096))
                return 0;
            dest += 4096;
            len -= 4096;
        }
        else
        {
            if (!memcpy_safe(dest, buf, len))
                return 0;
            break;
        }
    }
    return 1;
}
