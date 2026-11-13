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

    // One switch restores normally hidden equipment/model icons.
    // A separate jetpack placement option is useful when carrying a gun.
    bool ShowHiddenWeaponIcons = false;
    bool ShowJetpackBesideWeapon = false;
    // Gap below the Clock, based on the first visible stack group.
    float ClockToMoneySpacing = 25.0f;
    float ClockToWantedSpacing = 25.0f;
    float ClockToStatusBarSpacing = 22.5f;
    float ClockToWeaponSpacing = 30.0f;
    float MoneyToWantedSpacing = 25.0f;
    float MoneyToWeaponSpacing = 35.0f;
    float MoneyToStatusBarSpacing = 22.5f;
    float WantedToWeaponSpacing = 32.5f;
    float WantedToStatusBarSpacing = 22.5f;
    float StatusBarToWeaponSpacing = 17.5f;
    float ArmourSpacing = 4.0f;
    float BreathSpacing = 4.0f;

    // Stack mode only. Offsets apply to Weapon and Ammo together;
    // the vertical distance controls the Ammo position beneath Weapon.
    float WeaponAmmoOffsetX = 12.0f;
    float WeaponAmmoOffsetY = -16.0f;
    float WeaponToAmmoVerticalOffset = 87.0f;

private:
    HUDSettings() = default;
};
