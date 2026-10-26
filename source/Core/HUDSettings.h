#pragma once

#include <cstdint>

#include "../Layout/HUDLayout.h"

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
    std::uint32_t VisibilityMs = 3000;

    bool Clock = true;
    bool Money = true;
    bool Wanted = true;
    bool Weapon = true;
    bool Ammo = true;
    bool Armour = true;
    bool Breath = true;
    bool Health = true;

    bool ShowOnFootAim = true;
    bool ShowOnVehicleLook = false;

    float MoneyToWantedSpacing = 35.0f;
    float MoneyToWeaponSpacing = 35.0f;
    float MoneyToStatusBarSpacing = 25.0f;
    float WantedToWeaponSpacing = 35.0f;
    float WantedToStatusBarSpacing = 25.0f;
    float WeaponToStatusBarSpacingNoAmmo = 10.0f;
    float ClockSpacing = 40.0f;
    float WeaponSpacing = 4.0f;
    float AmmoSpacing = 4.0f;
    float ArmourSpacing = 4.0f;
    float BreathSpacing = 4.0f;
    float HealthSpacing = 4.0f;

private:
    HUDSettings() = default;
};
