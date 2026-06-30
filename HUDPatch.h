#pragma once

#include <cstdint>

namespace HUDPatch
{
    struct Call
    {
        uintptr_t Address;
        int32_t RelativeCall;
    };

    inline constexpr Call Health{ 0x58EE9A, 0xFFFFA3D1 };
    inline constexpr Call Armour{ 0x58EF6B, 0xFFFFA130 };
    inline constexpr Call Money{ 0x58F607, 0x0018B0F4 };

    inline constexpr Call Weapon{ 0x58F944, 0xFFFFDE87 };
    inline constexpr Call Ammo{ 0x58FA25, 0xFFFF9986 };
}