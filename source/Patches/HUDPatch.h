#pragma once

#include <cstdint>

namespace HUDPatch
{
    struct Call
    {
        uintptr_t Address;
        int32_t RelativeCall;
    };
    inline constexpr Call Clock{ 0x58EC21, 0x0018BADA };
    inline constexpr Call Health{ 0x58EE9A, 0xFFFFA3D1 };
    inline constexpr Call Armour{ 0x58EF6B, 0xFFFFA130 };

    inline constexpr Call Breath1{ 0x58F131, 0xFFFFA05A };
    inline constexpr Call Breath2{ 0x58F1AD, 0xFFFF9FDE };

    inline constexpr Call Weapon{ 0x58F944, 0xFFFFDE87 };
    inline constexpr Call Ammo{ 0x58FA25, 0xFFFF9986 };
}