#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include <windows.h>
#include <extensions/Paths.h>

#include "HUDSettings.h"
#include "../ResourceIds.h"

namespace
{
    static void CreateDefaultIniIfMissing(const char* path)
    {
        // Keep source/CleanHUD.ini as the only maintained template:
        // the resource compiler bundles its exact bytes into this ASI.
        // Fetch the resource from our DLL, not gta_sa.exe.
        HMODULE module = nullptr;

        if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&CreateDefaultIniIfMissing),
            &module))
        {
            return;
        }

        const HRSRC resource = FindResourceA(
            module,
            MAKEINTRESOURCEA(IDR_CLEANHUD_DEFAULT_INI),
            RT_RCDATA);

        if (!resource)
            return;

        const DWORD size = SizeofResource(module, resource);
        const HGLOBAL loaded = LoadResource(module, resource);

        if (!loaded || size == 0)
            return;

        const void* data = LockResource(loaded);

        if (!data)
            return;

        // CREATE_NEW never overwrites a user's existing INI.
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
        const BOOL saved = WriteFile(
            file, data, size, &written, nullptr);

        CloseHandle(file);

        // Avoid leaving a partial/corrupt configuration behind.
        if (!saved || written != size)
            DeleteFileA(path);
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

    char mode[32] = {};
    GetPrivateProfileStringA(
        "General", "HUDMode", "Stack",
        mode, sizeof(mode), path.c_str());

    Mode = _stricmp(mode, "Minimal") == 0
        ? HUDMode::Minimal
        : HUDMode::Stack;

    AlwaysShowClock = ReadBool(
        path.c_str(), "General", "AlwaysShowClock", true);

    SlideStack = ReadBool(
        path.c_str(), "Animation", "SlideStack", false);

    FadeInMs = ReadInt(
        path.c_str(), "Visibility", "FadeInMs", 200, 0, 2000);

    FadeOutMs = ReadInt(
        path.c_str(), "Visibility", "FadeOutMs", 200, 0, 2000);

    VisibilityMs = ReadInt(
        path.c_str(), "Visibility", "DurationMs", 3000, 0, 60000);

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

    ShowOnThrowableAim = ReadBool(
        path.c_str(), "WeaponAmmo", "ShowOnThrowableAim", false);

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

    StatusBarToWeaponSpacing = ReadFloat(
        path.c_str(), "Spacing", "StatusBarToWeaponSpacing", 10.0f);

    ClockToMoneySpacing = ReadFloat(
        path.c_str(), "Spacing", "ClockToMoneySpacing", 25.0f);

    ClockToWantedSpacing = ReadFloat(
        path.c_str(), "Spacing", "ClockToWantedSpacing", 30.0f);

    ClockToStatusBarSpacing = ReadFloat(
        path.c_str(), "Spacing", "ClockToStatusBarSpacing", 20.0f);

    ClockToWeaponSpacing = ReadFloat(
        path.c_str(), "Spacing", "ClockToWeaponSpacing", 27.5f);

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


bool HUDSettings::Save() const
{
    const std::string path =
        plugin::paths::GetPluginDirRelativePathA("CleanHUD.ini");

    CreateDefaultIniIfMissing(path.c_str());

    const auto put = [&path](
        const char* section, const char* key, const char* value)
    {
        return WritePrivateProfileStringA(
            section, key, value, path.c_str()) != 0;
    };

    const auto putBool = [&put](
        const char* section, const char* key, bool value)
    {
        return put(section, key, value ? "1" : "0");
    };

    const auto putInt = [&put](
        const char* section, const char* key, std::uint32_t value)
    {
        const std::string number = std::to_string(value);
        return put(section, key, number.c_str());
    };

    const auto putFloat = [&put](
        const char* section, const char* key, float value)
    {
        // INI numbers use a dot independent of Windows locale.
        char number[64] = {};
        std::snprintf(number, sizeof(number), "%.3f",
            static_cast<double>(value));
        return put(section, key, number);
    };

    bool success = true;
    success = put("General", "HUDMode",
        Mode == HUDMode::Stack ? "Stack" : "Minimal") && success;
    success = putBool("General", "AlwaysShowClock", AlwaysShowClock) && success;
    success = putBool("Animation", "SlideStack", SlideStack) && success;
    success = putBool("Elements", "Clock", Clock) && success;
    success = putBool("Elements", "Money", Money) && success;
    success = putBool("Elements", "Wanted", Wanted) && success;
    success = putBool("Elements", "Health", Health) && success;
    success = putBool("Elements", "Armour", Armour) && success;
    success = putBool("Elements", "Breath", Breath) && success;
    success = putBool("Elements", "Weapon", Weapon) && success;
    success = putBool("Elements", "Ammo", Ammo) && success;
    success = putBool("WeaponAmmo", "ShowOnFootAim", ShowOnFootAim) && success;
    success = putBool("WeaponAmmo", "ShowOnThrowableAim", ShowOnThrowableAim) && success;
    success = putBool("WeaponAmmo", "ShowOnVehicleLook", ShowOnVehicleLook) && success;
    success = putInt("Visibility", "FadeInMs", FadeInMs) && success;
    success = putInt("Visibility", "FadeOutMs", FadeOutMs) && success;
    success = putInt("Visibility", "DurationMs", VisibilityMs) && success;
    success = putFloat("Spacing", "ClockToMoneySpacing", ClockToMoneySpacing) && success;
    success = putFloat("Spacing", "ClockToWantedSpacing", ClockToWantedSpacing) && success;
    success = putFloat("Spacing", "ClockToStatusBarSpacing", ClockToStatusBarSpacing) && success;
    success = putFloat("Spacing", "ClockToWeaponSpacing", ClockToWeaponSpacing) && success;
    success = putFloat("Spacing", "MoneyToWantedSpacing", MoneyToWantedSpacing) && success;
    success = putFloat("Spacing", "MoneyToStatusBarSpacing", MoneyToStatusBarSpacing) && success;
    success = putFloat("Spacing", "MoneyToWeaponSpacing", MoneyToWeaponSpacing) && success;
    success = putFloat("Spacing", "WantedToStatusBarSpacing", WantedToStatusBarSpacing) && success;
    success = putFloat("Spacing", "WantedToWeaponSpacing", WantedToWeaponSpacing) && success;
    success = putFloat("Spacing", "StatusBarToWeaponSpacing", StatusBarToWeaponSpacing) && success;
    success = putFloat("Spacing", "WeaponSpacing", WeaponSpacing) && success;
    success = putFloat("Spacing", "AmmoSpacing", AmmoSpacing) && success;
    success = putFloat("Spacing", "ArmourSpacing", ArmourSpacing) && success;
    success = putFloat("Spacing", "BreathSpacing", BreathSpacing) && success;
    success = putFloat("Spacing", "HealthSpacing", HealthSpacing) && success;

    // Flush Win32 profile cache after the explicit Save click.
    return WritePrivateProfileStringA(
        nullptr, nullptr, nullptr, path.c_str()) != 0 && success;
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
