#include <algorithm>
#include <string>

#include <windows.h>
#include <extensions/Paths.h>

#include "HUDSettings.h"

namespace
{
    constexpr char DefaultIni[] =
        "; CleanHUD - GTA San Andreas (1.0 US)\r\n"
        "; Place this file beside CleanHUD.SA.asi. Restart after editing.\r\n"
        "; 1 = enabled, 0 = disabled.\r\n"
        "\r\n[Animation]\r\n"
        "; Sliding is optional and off by default.\r\n"
        "SlideStack=0\r\n"
        "FadeInMs=200\r\n"
        "FadeOutMs=200\r\n"
        "\r\n[Visibility]\r\n"
        "DurationMs=4000\r\n"
        "\r\n[Elements]\r\n"
        "; 0 restores vanilla behavior for the selected element.\r\n"
        "Clock=1\r\n"
        "Money=1\r\n"
        "Wanted=1\r\n"
        "Weapon=1\r\n"
        "Ammo=1\r\n"
        "Armour=1\r\n"
        "Breath=1\r\n"
        "Health=1\r\n"
        "\r\n[WeaponAmmo]\r\n"
        "ShowOnFootAim=1\r\n"
        "ShowOnVehicleLook=1\r\n"
        "ShowOnVehicleFire=1\r\n"
        "; TAB always allows a HUD preview.\r\n"
        "AmmoOnlyWhileFiring=0\r\n";

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
            "CleanHUD.SA.ini");

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
        path.c_str(), "WeaponAmmo", "ShowOnVehicleLook", true);

    ShowOnVehicleFire = ReadBool(
        path.c_str(), "WeaponAmmo", "ShowOnVehicleFire", true);

    AmmoOnlyWhileFiring = ReadBool(
        path.c_str(), "WeaponAmmo", "AmmoOnlyWhileFiring", false);
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
