#include "cshortasm.h"

CShortAsm::CShortAsm(uint pages)
{
    _size = pages * PAGE_SIZE;
    _code = (byte*)mmap(NULL, _size, PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
}

CShortAsm::~CShortAsm()
{
    munmap(_code, _size);
}

void CShortAsm::insert(byte* array, uint size)
{
    for (int i = 0; i < size; ++i)
        write(array[i]);
}

void CShortAsm::insert(QByteArray array)
{
    insert((byte*)array.data(), array.size());
}

void CShortAsm::push(byte value)
{
    write(0x6a);
    write(value);
}

void CShortAsm::push(int value)
{
    write(0x68);
    byteValue<uint> v;
    v.value = value;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::push(r86 r)
{
    write(0x50 + (byte)r);
}

void CShortAsm::pushad()
{
    write(0x60);
}

void CShortAsm::pushfd()
{
    write(0x9C);
}

void CShortAsm::pop(r86 r)
{
    write(0x58 + (byte)r);
}

void CShortAsm::popad()
{
    write(0x61);
}

void CShortAsm::popfd()
{
    write(0x9D);
}

void CShortAsm::label(QString label)
{
    _labels[label] = (uint)_code + _offset;
}

void CShortAsm::jmp(int addr)
{
    byteValue<uint> v;
    v.value = getRelativeAddress(addr);
    write(0xE9);
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jmp(QString label)
{
    jmp(_labels[label]);
}

void CShortAsm::jmp(r86 r)
{
    write(0xff);
    write(0xE0 + (byte)r);
}

void CShortAsm::ret(int pops)
{
    if (!pops)
        write(0xC3);
    else{
        byteValue<ushort> v;
        v.value = pops * 4;
        write(0xC2);
        write(v.bytes[0]);
        write(v.bytes[1]);
    }
}

void CShortAsm::nop()
{
    write(0x90);
}

void CShortAsm::call(int addr)
{
    byteValue<uint> v;
    v.value = getRelativeAddress(addr);
    write(0xE8);
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::call(QString label)
{
    call(_labels[label]);
}

void CShortAsm::call(r86 r)
{
    write(0xff);
    write(0xD0 + (byte)r);
}

void CShortAsm::mov(int &var, r86 r)
{
    byteValue<uint> v;
    v.value = (uint)&var;
    if (r != EAX)
        write(0x89);
    switch (r) {
    case EAX:
        write(0xa3);
        break;
    case ECX:
        write(0x0d);
        break;
    case EDX:
        write(0x15);
        break;
    case EBX:
        write(0x1d);
        break;
    case ESP:
        write(0x25);
        break;
    case EBP:
        write(0x2D);
        break;
    case ESI:
        write(0x35);
        break;
    case EDI:
        write(0x3d);
        break;
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::mov(r86 r, int &var)
{
    byteValue<uint> v;
    v.value = (uint)&var;
    if (r != EAX)
        write(0x8b);
    switch (r) {
    case EAX:
        write(0xa1);
        break;
    case ECX:
        write(0x0d);
        break;
    case EDX:
        write(0x15);
        break;
    case EBX:
        write(0x1d);
        break;
    case ESP:
        write(0x25);
        break;
    case EBP:
        write(0x2D);
        break;
    case ESI:
        write(0x35);
        break;
    case EDI:
        write(0x3d);
        break;
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::mov(r86 r, const int value)
{
    write(0xB8 + (byte)r);
    byteValue<uint> v;
    v.value = value;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::mov(r86 r1, r86 r2, byte offset)
{
    write(0x8B);
    write(0x08 * (byte)r1 + (byte)r2 +
                       (offset == 0 ? 0x00 : 0x40));
    if (offset != 0)
        write(offset);
}

void CShortAsm::mov(r86 r1, byte offset, r86 r2)
{
    write(0x89);
    if (r1 != ESP && r1 != EBP){
        write((byte)r1 + 0x08 * (byte)r2 +
                           (offset == 0 ? 0x00 : 0x40));
    }
    else if (r1 == ESP){
        write(0x04 + 0x08 * (byte)r2 +
                           (offset == 0 ? 0x00 : 0x40));
    }
    else if (r1 == EBP){
        write(0x45 + (byte)r2);
    }
    if (offset != 0 || r1 == EBP)
        write(offset);
}

void CShortAsm::xchg(r86 r1, r86 r2, byte offset)
{
    if (r2 == EAX)
        std::swap(r1, r2);
    if (!offset){
        if (r1 == EAX)
            write(0x90 + (byte)r2);
        else{
            write(0x87);
            write(0xC8 + (byte)r1 + 0x08 * ((byte)r2 - 1));
        }
    }
    else{
        write(0x87);
        write(0x40 + (byte)r2 + 0x08 * (byte)r1);
        write(offset);
    }
}

void CShortAsm::xchg(r86 r, int &var)
{
    write(0x87);
    write(0x05 + (byte)r);
    byteValue<uint> v;
    v.value = (uint)&var;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::add(r86 r, byte value)
{
    write(0x83);
    write(0xC0 + (byte)r);
    write(value);
}

void CShortAsm::add(r86 r, const int value)
{
    byteValue<uint> v;
    v.value = value;
    if (r == EAX){
        write(0x05);
    }
    else{
        write(0x81);
        write(0xC0 + (byte)r);
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::add(r86 r, int &var)
{
    byteValue<uint> v;
    v.value = (uint)&var;
    write(0x03);
    write(0x05 + 0x08 * (byte)r);
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::sub(r86 r, byte value)
{
    write(0x83);
    write(0xE8 + (byte)r);
    write(value);
}

void CShortAsm::sub(r86 r, const int value)
{
    byteValue<uint> v;
    v.value = value;
    if (r == EAX){
        write(0x2D);
    }
    else{
        write(0x81);
        write(0xE8 + (byte)r);
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::sub(r86 r, int &var)
{
    byteValue<uint> v;
    v.value = (uint)&var;
    write(0x2B);
    write(0x05 + 0x08 * (byte)r);
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::mul(r86 r)
{
    write(0xF7);
    write(0xE0 + (byte)r);
}

void CShortAsm::imul(r86 r)
{
    write(0xF7);
    write(0xE8 + (byte)r);
}

void CShortAsm::imul(r86 r1, r86 r2)
{
    write(0x0F);
    write(0xAF);
    write(0xC0 + (byte)r2 + 0x08 * r1);
}

void CShortAsm::imul(r86 r, byte value)
{
    write(0x6B);
    write(0xC0 + 0x09 * r);
    write(value);
}

void CShortAsm::imul(r86 r, const int value)
{
    write(0x69);
    write(0xC0 + 0x09 * r);
    write(value);
}

void CShortAsm::imul(r86 r1, r86 r2, byte value)
{
    write(0x6B);
    write(0xC0 + (byte)r2 + 0x08 * r1);
    write(value);
}

void CShortAsm::imul(r86 r1, r86 r2, const int value)
{
    write(0x69);
    write(0xC0 + (byte)r2 + 0x08 * r1);
    byteValue<uint> v;
    v.value = value;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::div(r86 r)
{
    write(0xF7);
    write(0xF0 + (byte)r);
}

void CShortAsm::idiv(r86 r)
{
    write(0xF7);
    write(0xF8 + (byte)r);
}

void CShortAsm::cmp(r86 r1, r86 r2)
{
    write(0x39);
    write(0xC0 + (byte)r1 + 0x08 * (byte)r2);
}

void CShortAsm::cmp(r86 r, byte value)
{
    write(0x83);
    write(0xF8 + (byte)r);
    write(value);
}

void CShortAsm::cmp(r86 r, const int value)
{
    byteValue<uint> v;
    v.value = value;
    if (r == EAX)
        write(0x3D);
    else {
        write(0x81);
        write(0xf8 + (byte)r);
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::XOR(r86 r1, r86 r2)
{
    write(0x31);
    write(0xC0 + (byte)r1 + 0x08 * (byte)r2);
}

void CShortAsm::XOR(r86 r, byte value)
{
    write(0x83);
    write(0xF0 + (byte)r);
    write(value);
}

void CShortAsm::XOR(r86 r, const int value)
{
    byteValue<uint> v;
    v.value = value;
    if (r == EAX)
        write(0x35);
    else {
        write(0x81);
        write(0xf0 + (byte)r);
    }
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::je(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x84);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::je(QString label)
{
    je(_labels[label]);
}

void CShortAsm::jne(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x85);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jne(QString label)
{
    jne(_labels[label]);
}

void CShortAsm::jl(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x8C);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jl(QString label)
{
    jl(_labels[label]);
}

void CShortAsm::jle(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x8E);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jle(QString label)
{
    jle(_labels[label]);
}

void CShortAsm::jg(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x8F);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jg(QString label)
{
    jg(_labels[label]);
}

void CShortAsm::jge(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x8D);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jge(QString label)
{
    jge(_labels[label]);
}

void CShortAsm::jb(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x82);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jb(QString label)
{
    jb(_labels[label]);
}

void CShortAsm::jbe(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x86);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jbe(QString label)
{
    jbe(_labels[label]);
}

void CShortAsm::ja(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x87);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::ja(QString label)
{
    ja(_labels[label]);
}

void CShortAsm::jae(const int addr)
{
    write(0x0F);
    uint relative = getRelativeAddress(addr);
    write(0x83);
    byteValue<uint> v;
    v.value = relative;
    write(v.bytes[0]);
    write(v.bytes[1]);
    write(v.bytes[2]);
    write(v.bytes[3]);
}

void CShortAsm::jae(QString label)
{
    jae(_labels[label]);
}












void CShortAsm::resetWriteOffset()
{
    _offset = 0;
}

uint CShortAsm::getWriteOffset()
{
    return _offset;
}

void CShortAsm::setWriteOffset(uint offset)
{
    _offset = offset;
}

uint CShortAsm::arrayToPages(size_t size_array)
{
    return (size_array / PAGE_SIZE) + 1;
}

const byte* CShortAsm::getAddr()
{
    return _code;
}

uint CShortAsm::getSize()
{
    return _peak;
}

void CShortAsm::resize(uint pages)
{
    byte* new_code = (byte*)mmap(NULL, pages * PAGE_SIZE,
                                 PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    memcpy(new_code, _code, _size);
    _size = pages * PAGE_SIZE;
    munmap(_code, _size);
    _code = new_code;
}

uint CShortAsm::getRelativeAddress(uint addr)
{
    return addr - (((uint)_code + _offset) + 5);
}

void CShortAsm::write(byte v)
{
    if (_offset + 1 >= _size)
        resize(arrayToPages(_offset * 2));
    _code[_offset++] = v;
    if (_offset > _peak)
        _peak = _offset;
}
