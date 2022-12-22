// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "..\utils\crypt_str.h"

template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

void Memory::initialize() noexcept
{
    auto temp = reinterpret_cast<std::uintptr_t*>(findPattern(crypt_str("client"), "\xB9????\xE8????\x8B\x5D\x08") + 1);
    hud = *temp;
    findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<uintptr_t>(temp) + 5);
    clearHudWeapon = relativeToAbsolute<decltype(clearHudWeapon)>(findPattern(crypt_str("client"), "\xE8????\x8B\xF0\xC6\x44\x24??\xC6\x44\x24") + 1);
    itemSchema = relativeToAbsolute<decltype(itemSchema)>(findPattern(crypt_str("client"), "\xE8????\x0F\xB7\x0F") + 1);
    equipWearable = reinterpret_cast<decltype(equipWearable)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9"));
}