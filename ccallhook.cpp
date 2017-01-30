#include "ccallhook.h"

CCallHook::CCallHook(void *addr, eSafeCall save, uint size, eCodePos pos)
{
    hook_addr = addr;
    _size = size;
    hook_pos = pos;
    _asm = new CShortAsm();

    orig_bytes = new byte[size + 1];
    memsafe::memcpy_safe(orig_bytes, addr, size);

    if (pos == cp_before){
        if (orig_bytes[0] == 0xE9)
            memsafe::memset_safe(orig_bytes, 0x90, 5);
        else ModOriginalBytes((uint)_asm->getAddr() + _asm->getWriteOffset());
        _asm->insert(orig_bytes, size);
    }
    if (checkFlag(save, sc_registers))
        _asm->pushad();
    if (checkFlag(save, sc_flags))
        _asm->pushfd();

    hook_offset = _asm->getWriteOffset();
    if (pos == cp_skip)
        ModOriginalBytes((uint)_asm->getAddr() + _asm->getWriteOffset());
    disable();

    if (checkFlag(save, sc_flags))
        _asm->popfd();
    if (checkFlag(save, sc_registers))
        _asm->popad();
    if (pos == cp_after){
        ModOriginalBytes((uint)_asm->getAddr() + _asm->getWriteOffset());
        _asm->insert(orig_bytes, size);
    }

    _asm->jmp((uint)addr + size);

    memsafe::memset_safe(addr, 0x90, size);
    byteValue<uint> v;
    v.value = (uint)_asm->getAddr() - ((uint)addr + 5);
    memsafe::memset_safe(addr, 0xE9, 1);
    memsafe::memset_safe((void*)((uint)addr + 1), v.bytes[0], 1);
    memsafe::memset_safe((void*)((uint)addr + 2), v.bytes[1], 1);
    memsafe::memset_safe((void*)((uint)addr + 3), v.bytes[2], 1);
    memsafe::memset_safe((void*)((uint)addr + 4), v.bytes[3], 1);
}

CCallHook::~CCallHook()
{
    disable();
    //delete _asm;
}

void CCallHook::enable(void(__stdcall*func)())
{
    setNops();
    if (func == nullptr)
        return;
    _asm->setWriteOffset(hook_offset);
    _asm->call((uint)func);
}

void CCallHook::disable()
{
    _asm->setWriteOffset(hook_offset);
    if (hook_pos == cp_skip)
        _asm->insert(orig_bytes, _size);
    else setNops();
}

template<typename T> inline
bool CCallHook::checkFlag(T value, T flag )
{
    return (value & flag);
}

void CCallHook::setNops()
{
    _asm->setWriteOffset(hook_offset);
    for (int i = 0; i < _size; ++i)
        _asm->nop();
}

void CCallHook::ModOriginalBytes(uint offset)
{
    // call and jmp (long)
    if (orig_bytes[0] == 0xE9 || orig_bytes[0] == 0xE8){
        byteValue<uint> v;
        v.bytes[0] = orig_bytes[1];
        v.bytes[1] = orig_bytes[2];
        v.bytes[2] = orig_bytes[3];
        v.bytes[3] = orig_bytes[4];
        uint o_addr = v.value + ((uint)hook_addr + 5);
        v.value = o_addr - (offset + 5);
        orig_bytes[1] = v.bytes[0];
        orig_bytes[2] = v.bytes[1];
        orig_bytes[3] = v.bytes[2];
        orig_bytes[4] = v.bytes[3];
    }
    // conditionals jmp's (long)
    else if (orig_bytes[0] == 0x0F) {
        if (orig_bytes[1] >= 0x81 && orig_bytes[1] <= 0x8F){
            byteValue<uint> v;
            v.bytes[0] = orig_bytes[2];
            v.bytes[1] = orig_bytes[3];
            v.bytes[2] = orig_bytes[4];
            v.bytes[3] = orig_bytes[5];
            uint o_addr = v.value + ((uint)hook_addr + 6);
            v.value = o_addr - (offset + 6);
            orig_bytes[2] = v.bytes[0];
            orig_bytes[3] = v.bytes[1];
            orig_bytes[4] = v.bytes[2];
            orig_bytes[5] = v.bytes[3];
        }
    }
}
