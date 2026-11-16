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

    // Load settings at startup, creating the INI if needed.
    void Load();

    // Save the current settings to the INI.
    bool Save() const;

    bool IsElementEnabled(HUDComponentType type) const;

    // Hold a key to show GTA's original HUD positions.
    // Tab is the keyboard default; the mapped gamepad action is off.
    std::uint32_t VanillaHUDKey = 9;
    std::uint32_t VanillaHUDPadButton = 0;
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

    // Show available phone, jetpack and custom weapon icons.
    bool ShowHiddenWeaponIcons = true;
    // Move the jetpack icon from its normal or side-by-side position.
    float JetpackIconOffsetX = 0.0f;
    float JetpackIconOffsetY = 0.0f;

    // The vehicle ID detects riding; the icon model provides the image.
    // A carried skateboard still uses the regular weapon icon.
    bool ShowSkateboardRidingIcon = false;
    std::uint32_t SkateboardVehicleModelId = 12500;
    std::uint32_t SkateboardIconModelId = 12400;
    float SkateboardIconOffsetX = 0.0f;
    float SkateboardIconOffsetY = 0.0f;

    // Choose the clock gap based on the next visible HUD item.
    float ClockToMoneySpacing = 25.0f;
    float ClockToWantedSpacing = 25.0f;
    float ClockToStatusBarSpacing = 22.0f;
    float ClockToWeaponSpacing = 30.0f;
    float MoneyToWantedSpacing = 25.0f;
    float MoneyToWeaponSpacing = 35.0f;
    float MoneyToStatusBarSpacing = 22.0f;
    float WantedToWeaponSpacing = 32.0f;
    float WantedToStatusBarSpacing = 22.0f;
    float StatusBarToWeaponSpacing = 17.0f;
    float ArmourSpacing = 4.0f;
    float BreathSpacing = 4.0f;

    // In Stack mode, move weapon and ammo together and adjust
    // the distance between them.
    float WeaponAmmoOffsetX = 12.0f;
    float WeaponAmmoOffsetY = -16.0f;
    float WeaponToAmmoVerticalOffset = 83.0f;

private:
    HUDSettings() = default;
};
