#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <string>

#include <windows.h>
#include <extensions/Paths.h>

#include "HUDSettings.h"

namespace
{
    constexpr char DefaultIni[] =
        "; CleanHUD\r\n"
        "; Place this file beside CleanHUD.SA.asi. Restart after editing.\r\n"
        "; 1 = enabled, 0 = disabled.\r\n"
        "\r\n[Animation]\r\n"
        "; Sliding is optional and off by default.\r\n"
        "SlideStack = 0\r\n"
        "FadeInMs = 200\r\n"
        "FadeOutMs = 200\r\n"
        "\r\n[Visibility]\r\n"
        "DurationMs = 3000\r\n"
        "\r\n[Elements]\r\n"
        "; 0 restores vanilla behavior for the selected element.\r\n"
        "Clock = 1\r\n"
        "Money = 1\r\n"
        "Wanted = 1\r\n"
        "Weapon = 1\r\n"
        "Ammo = 1\r\n"
        "Armour = 1\r\n"
        "Breath = 1\r\n"
        "Health = 1\r\n"
        "\r\n[WeaponAmmo]\r\n"
        "ShowOnFootAim = 1\r\n"
        "\r\n; Side-looking alone does not reveal weapons.\r\n"
        "ShowOnVehicleLook = 0\r\n"
        "\r\n[Spacing]\r\n"
        "MoneyToWantedSpacing = 35.0\r\n"
        "MoneyToWeaponSpacing = 35.0\r\n"
        "MoneyToStatusBarSpacing = 25.0\r\n"
        "\r\nWantedToWeaponSpacing = 35.0\r\n"
        "WantedToStatusBarSpacing = 25.0\r\n"
        "WeaponToStatusBarSpacingNoAmmo = 10.0\r\n"
        "\r\nClockSpacing = 40.0\r\n"
        "WeaponSpacing = 4.0\r\n"
        "AmmoSpacing = 4.0\r\n"
        "ArmourSpacing = 4.0\r\n"
        "BreathSpacing = 4.0\r\n"
        "HealthSpacing = 4.0";

    static void CreateDefaultIniIfMissing(const char* path)
    {
        // CREATE_NEW never overwrites a user's existing settings.
        HANDLE file = CreateFileA(
            path,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            nullptr,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
            return;

        DWORD written = 0;
        WriteFile(
            file,
            DefaultIni,
            static_cast<DWORD>(sizeof(DefaultIni) - 1),
            &written,
            nullptr);
        CloseHandle(file);
    }

    static int ReadInt(
        const char* path,
        const char* section,
        const char* key,
        int fallback,
        int minimum,
        int maximum)
    {
        const int value = static_cast<int>(
            GetPrivateProfileIntA(
                section,
                key,
                fallback,
                path));

        return std::clamp(value, minimum, maximum);
    }

    static float ReadFloat(
        const char* path,
        const char* section,
        const char* key,
        float fallback)
    {
        char text[64] = {};
        GetPrivateProfileStringA(
            section, key, "", text,
            sizeof(text), path);

        if (!text[0])
            return fallback;

        char* end = nullptr;
        errno = 0;
        const float value = std::strtof(text, &end);

        if (end == text || errno == ERANGE ||
            !std::isfinite(value))
        {
            return fallback;
        }

        while (*end &&
            std::isspace(static_cast<unsigned char>(*end)))
        {
            ++end;
        }

        if (*end)
            return fallback;

        // Allow fine-tuning without accepting pathological
        // spacings that would send the HUD offscreen.
        return std::clamp(value, -200.0f, 500.0f);
    }

    static bool ReadBool(
        const char* path,
        const char* section,
        const char* key,
        bool fallback)
    {
        return ReadInt(
            path, section, key,
            fallback ? 1 : 0,
            0, 1) != 0;
    }
}

HUDSettings& HUDSettings::Get()
{
    static HUDSettings settings;
    return settings;
}

void HUDSettings::Load()
{
    // Plugin-SDK resolves the directory of the ASI itself,
    // including when installed in a subfolder of scripts.
    const std::string path =
        plugin::paths::GetPluginDirRelativePathA(
            "CleanHUD.ini");

    CreateDefaultIniIfMissing(path.c_str());

    SlideStack = ReadBool(
        path.c_str(), "Animation", "SlideStack", SlideStack);

    FadeInMs = ReadInt(
        path.c_str(), "Animation", "FadeInMs", 200, 0, 2000);

    FadeOutMs = ReadInt(
        path.c_str(), "Animation", "FadeOutMs", 200, 0, 2000);

    VisibilityMs = ReadInt(
        path.c_str(), "Visibility", "DurationMs", 4000, 0, 60000);

    Clock = ReadBool(path.c_str(), "Elements", "Clock", true);
    Money = ReadBool(path.c_str(), "Elements", "Money", true);
    Wanted = ReadBool(path.c_str(), "Elements", "Wanted", true);
    Weapon = ReadBool(path.c_str(), "Elements", "Weapon", true);
    Ammo = ReadBool(path.c_str(), "Elements", "Ammo", true);
    Armour = ReadBool(path.c_str(), "Elements", "Armour", true);
    Breath = ReadBool(path.c_str(), "Elements", "Breath", true);
    Health = ReadBool(path.c_str(), "Elements", "Health", true);

    ShowOnFootAim = ReadBool(
        path.c_str(), "WeaponAmmo", "ShowOnFootAim", true);

    ShowOnVehicleLook = ReadBool(
        path.c_str(), "WeaponAmmo", "ShowOnVehicleLook", false);

    MoneyToWantedSpacing = ReadFloat(
        path.c_str(), "Spacing", "MoneyToWantedSpacing", 35.0f);

    MoneyToWeaponSpacing = ReadFloat(
        path.c_str(), "Spacing", "MoneyToWeaponSpacing", 35.0f);

    MoneyToStatusBarSpacing = ReadFloat(
        path.c_str(), "Spacing", "MoneyToStatusBarSpacing", 25.0f);

    WantedToWeaponSpacing = ReadFloat(
        path.c_str(), "Spacing", "WantedToWeaponSpacing", 35.0f);

    WantedToStatusBarSpacing = ReadFloat(
        path.c_str(), "Spacing", "WantedToStatusBarSpacing", 25.0f);

    WeaponToStatusBarSpacingNoAmmo = ReadFloat(
        path.c_str(), "Spacing", "WeaponToStatusBarSpacingNoAmmo", 10.0f);

    ClockSpacing = ReadFloat(
        path.c_str(), "Spacing", "ClockSpacing", 40.0f);

    WeaponSpacing = ReadFloat(
        path.c_str(), "Spacing", "WeaponSpacing", 4.0f);

    AmmoSpacing = ReadFloat(
        path.c_str(), "Spacing", "AmmoSpacing", 4.0f);

    ArmourSpacing = ReadFloat(
        path.c_str(), "Spacing", "ArmourSpacing", 4.0f);

    BreathSpacing = ReadFloat(
        path.c_str(), "Spacing", "BreathSpacing", 4.0f);

    HealthSpacing = ReadFloat(
        path.c_str(), "Spacing", "HealthSpacing", 4.0f);
}

bool HUDSettings::IsElementEnabled(HUDComponentType type) const
{
    switch (type)
    {
    case HUDComponentType::Clock: return Clock;
    case HUDComponentType::Money: return Money;
    case HUDComponentType::Wanted: return Wanted;
    case HUDComponentType::Weapon: return Weapon;
    case HUDComponentType::Ammo: return Ammo;
    case HUDComponentType::Armour: return Armour;
    case HUDComponentType::Breath: return Breath;
    case HUDComponentType::Health: return Health;
    default: return true;
    }
}
