#pragma once

#include <cstdint>

#include "HUDLayout.h"

class HUDSettings
{
public:
    static HUDSettings& Get();

    // Loads only at plugin startup. Missing files are created with defaults.
    void Load();

    bool IsElementEnabled(HUDComponentType type) const;

    bool SlideStack = false;
    std::uint32_t FadeInMs = 200;
    std::uint32_t FadeOutMs = 200;
    std::uint32_t VisibilityMs = 4000;

    bool Clock = true;
    bool Money = true;
    bool Wanted = true;
    bool Weapon = true;
    bool Ammo = true;
    bool Armour = true;
    bool Breath = true;
    bool Health = true;

    bool ShowOnFootAim = true;
    bool ShowOnVehicleLook = true;
    bool ShowOnVehicleFire = true;
    bool AmmoOnlyWhileFiring = false;

private:
    HUDSettings() = default;
};
