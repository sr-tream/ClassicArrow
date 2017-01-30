#ifndef MEMSAFE_H
#define MEMSAFE_H

#include <stdint.h>
#include <windows.h>

namespace memsafe
{
    void *memcpy_safe(void *dest, const void *src, size_t stLen);
    int memcmp_safe(const void *_s1, const void *_s2, uint32_t len);
    int memset_safe(void *_dest, int c, DWORD len);
};

#endif // MEMSAFE_H
