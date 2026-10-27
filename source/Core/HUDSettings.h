#pragma once

#include <cstdint>

#include "../Layout/HUDLayout.h"

enum class HUDMode
{
    Stack,
    Minimal
};

class HUDSettings
{
public:
    static HUDSettings& Get();

    // Loads only at plugin startup. Missing files are created with defaults.
    void Load();

    // Persists live editor settings to the existing INI.
    bool Save() const;

    bool IsElementEnabled(HUDComponentType type) const;

    HUDMode Mode = HUDMode::Stack;
    bool AlwaysShowClock = true;

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
    bool ShowOnThrowableAim = false;

    // Gap below the Clock, based on the first visible stack group.
    float ClockToMoneySpacing = 25.0f;
    float ClockToWantedSpacing = 30.0f;
    float ClockToStatusBarSpacing = 22.5f;
    float ClockToWeaponSpacing = 30.0f;
    float MoneyToWantedSpacing = 35.0f;
    float MoneyToWeaponSpacing = 35.0f;
    float MoneyToStatusBarSpacing = 25.0f;
    float WantedToWeaponSpacing = 35.0f;
    float WantedToStatusBarSpacing = 25.0f;
    float StatusBarToWeaponSpacing = 10.0f;
    float WeaponSpacing = 4.0f;
    float AmmoSpacing = 4.0f;
    float ArmourSpacing = 4.0f;
    float BreathSpacing = 4.0f;
    float HealthSpacing = 4.0f;

private:
    HUDSettings() = default;
};
