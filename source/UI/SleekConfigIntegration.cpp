#include "SleekConfigIntegration.h"

#include <cstdint>
#include <cstring>

#include <windows.h>
#include <sleekconfig.hpp>

#include "../Core/HUDSettings.h"

namespace
{
    // This integration only registers settings. SleekConfig itself owns the
    // F10 menu, D3D9 rendering, game input, and UI device lifecycle.
    constexpr const char* ModId = "prixsm.cleanhud";

    const ScApi* gApi = nullptr;
    ScMod gMod = 0;

    struct BoolSetting
    {
        const char* Id;
        const char* Label;
        const char* Page;
        const char* Group;
        bool HUDSettings::* Field;
        bool Default;
    };

    struct IntSetting
    {
        const char* Id;
        const char* Label;
        const char* Page;
        const char* Group;
        std::uint32_t HUDSettings::* Field;
        int Default;
        int Maximum;
    };

    struct FloatSetting
    {
        const char* Id;
        const char* Label;
        const char* Group;
        float HUDSettings::* Field;
        float Default;
    };

    constexpr BoolSetting BoolSettings[] = {
        { "always_show_clock", "Always show Clock", "General", "Clock", &HUDSettings::AlwaysShowClock, true },
        { "slide_stack", "Slide stack on entry", "General", "Animation", &HUDSettings::SlideStack, false },
        { "clock", "Clock", "Elements", "Managed elements", &HUDSettings::Clock, true },
        { "money", "Money", "Elements", "Managed elements", &HUDSettings::Money, true },
        { "wanted", "Wanted stars", "Elements", "Managed elements", &HUDSettings::Wanted, true },
        { "health", "Health", "Elements", "Managed elements", &HUDSettings::Health, true },
        { "armour", "Armour", "Elements", "Managed elements", &HUDSettings::Armour, true },
        { "breath", "Breath", "Elements", "Managed elements", &HUDSettings::Breath, true },
        { "weapon", "Weapon icon", "Elements", "Managed elements", &HUDSettings::Weapon, true },
        { "ammo", "Ammo", "Elements", "Managed elements", &HUDSettings::Ammo, true },
        { "show_on_foot_aim", "Show on foot aim", "Weapon & Ammo", "Aim controls", &HUDSettings::ShowOnFootAim, true },
        { "show_on_throwable_aim", "Show on throwable aim", "Weapon & Ammo", "Aim controls", &HUDSettings::ShowOnThrowableAim, false },
        { "show_on_vehicle_look", "Show on vehicle look", "Weapon & Ammo", "Aim controls", &HUDSettings::ShowOnVehicleLook, false }
    };

    constexpr IntSetting IntSettings[] = {
        { "fade_in_ms", "Fade in (ms)", "Visibility", "Animation", &HUDSettings::FadeInMs, 200, 2000 },
        { "fade_out_ms", "Fade out (ms)", "Visibility", "Animation", &HUDSettings::FadeOutMs, 200, 2000 },
        { "duration_ms", "Visibility duration (ms)", "Visibility", "Animation", &HUDSettings::VisibilityMs, 3000, 60000 }
    };

    constexpr FloatSetting FloatSettings[] = {
        { "clock_to_money", "Clock to Money", "Clock", &HUDSettings::ClockToMoneySpacing, 25.0f },
        { "clock_to_wanted", "Clock to Wanted", "Clock", &HUDSettings::ClockToWantedSpacing, 30.0f },
        { "clock_to_status", "Clock to Status Bar", "Clock", &HUDSettings::ClockToStatusBarSpacing, 20.0f },
        { "clock_to_weapon", "Clock to Weapon", "Clock", &HUDSettings::ClockToWeaponSpacing, 27.5f },
        { "money_to_wanted", "Money to Wanted", "Money", &HUDSettings::MoneyToWantedSpacing, 35.0f },
        { "money_to_status", "Money to Status Bar", "Money", &HUDSettings::MoneyToStatusBarSpacing, 25.0f },
        { "money_to_weapon", "Money to Weapon", "Money", &HUDSettings::MoneyToWeaponSpacing, 35.0f },
        { "wanted_to_status", "Wanted to Status Bar", "Wanted", &HUDSettings::WantedToStatusBarSpacing, 25.0f },
        { "wanted_to_weapon", "Wanted to Weapon", "Wanted", &HUDSettings::WantedToWeaponSpacing, 35.0f },
        { "status_to_weapon", "Status Bar to Weapon", "Status Bars", &HUDSettings::StatusBarToWeaponSpacing, 10.0f },
        { "weapon_spacing", "Weapon spacing", "Individual", &HUDSettings::WeaponSpacing, 4.0f },
        { "ammo_spacing", "Ammo spacing", "Individual", &HUDSettings::AmmoSpacing, 4.0f },
        { "armour_spacing", "Armour spacing", "Individual", &HUDSettings::ArmourSpacing, 4.0f },
        { "breath_spacing", "Breath spacing", "Individual", &HUDSettings::BreathSpacing, 4.0f },
        { "health_spacing", "Health spacing", "Individual", &HUDSettings::HealthSpacing, 4.0f }
    };

    ScSettingDesc Describe(
        const char* id, const char* label,
        const char* page, const char* group, ScType type)
    {
        ScSettingDesc setting = {};
        setting.size = sizeof(setting);
        setting.id = id;
        setting.label = label;
        setting.page = page;
        setting.group = group;
        setting.type = type;
        setting.default_value.type = type;
        return setting;
    }

    bool PublishCurrentIniValues()
    {
        if (!gApi || !gMod)
            return false;

        // register_setting uses its declared default, not the loaded INI.
        // Publish INI values before the first SleekConfig registry tick.
        // Saved SleekConfig DAT overrides are applied after these values.
        const HUDSettings& settings = HUDSettings::Get();
        ScValue value = {};

        value.type = SC_ENUM;
        value.integer = settings.Mode == HUDMode::Stack ? 0 : 1;

        if (gApi->publish_value(gMod, "hud_mode", &value) != SC_OK)
            return false;

        for (const auto& entry : BoolSettings)
        {
            value = {};
            value.type = SC_BOOL;
            value.integer = settings.*(entry.Field) ? 1 : 0;

            if (gApi->publish_value(gMod, entry.Id, &value) != SC_OK)
                return false;
        }

        for (const auto& entry : IntSettings)
        {
            value = {};
            value.type = SC_INT;
            value.integer = static_cast<std::int32_t>(
                settings.*(entry.Field));

            if (gApi->publish_value(gMod, entry.Id, &value) != SC_OK)
                return false;
        }

        for (const auto& entry : FloatSettings)
        {
            value = {};
            value.type = SC_FLOAT;
            value.real = settings.*(entry.Field);

            if (gApi->publish_value(gMod, entry.Id, &value) != SC_OK)
                return false;
        }

        return true;
    }

    int SC_CALL OnChanged(
        void*, const char* id, const ScValue* proposed)
    {
        if (!id || !proposed)
            return 0;

        HUDSettings& settings = HUDSettings::Get();

        if (std::strcmp(id, "hud_mode") == 0)
        {
            if (proposed->type != SC_ENUM)
                return 0;

            settings.Mode = proposed->integer == 0
                ? HUDMode::Stack : HUDMode::Minimal;
            return 1;
        }

        for (const auto& entry : BoolSettings)
        {
            if (std::strcmp(id, entry.Id) != 0)
                continue;

            if (proposed->type != SC_BOOL)
                return 0;

            settings.*(entry.Field) = proposed->integer != 0;
            return 1;
        }

        for (const auto& entry : IntSettings)
        {
            if (std::strcmp(id, entry.Id) != 0)
                continue;

            if (proposed->type != SC_INT)
                return 0;

            settings.*(entry.Field) =
                static_cast<std::uint32_t>(proposed->integer);
            return 1;
        }

        for (const auto& entry : FloatSettings)
        {
            if (std::strcmp(id, entry.Id) != 0)
                continue;

            if (proposed->type != SC_FLOAT)
                return 0;

            settings.*(entry.Field) = proposed->real;
            return 1;
        }

        return 0;
    }

    int SC_CALL OnSave(void*)
    {
        // SleekConfig saves a DAT snapshot too. Keep CleanHUD.ini in sync
        // with user-committed changes, preserving manual-INI workflows.
        return HUDSettings::Get().Save() ? 1 : 0;
    }

    int SC_CALL OnLoad(void*)
    {
        // SleekConfig's Reload action: INI provides the baseline,
        // then its stored DAT settings take precedence.
        HUDSettings::Get().Load();
        return PublishCurrentIniValues() ? 1 : 0;
    }

    bool RegisterSettings()
    {
        ScSettingDesc mode = Describe(
            "hud_mode", "HUD layout", "General", "Mode", SC_ENUM);

        mode.options = "Stack|Minimal";
        mode.minimum = 0;
        mode.maximum = 1;
        mode.step = 1;
        mode.default_value.integer = 0;

        if (gApi->register_setting(gMod, &mode) != SC_OK)
            return false;

        for (const auto& entry : BoolSettings)
        {
            auto setting = Describe(
                entry.Id, entry.Label, entry.Page,
                entry.Group, SC_BOOL);

            setting.minimum = 0;
            setting.maximum = 1;
            setting.step = 1;
            setting.default_value.integer = entry.Default ? 1 : 0;

            if (gApi->register_setting(gMod, &setting) != SC_OK)
                return false;
        }

        for (const auto& entry : IntSettings)
        {
            auto setting = Describe(
                entry.Id, entry.Label, entry.Page,
                entry.Group, SC_INT);

            setting.minimum = 0;
            setting.maximum = entry.Maximum;
            setting.step = 1;
            setting.default_value.integer = entry.Default;

            if (gApi->register_setting(gMod, &setting) != SC_OK)
                return false;
        }

        for (const auto& entry : FloatSettings)
        {
            auto setting = Describe(
                entry.Id, entry.Label, "Advanced Layout",
                entry.Group, SC_FLOAT);

            // Matches HUDSettings::ReadFloat; zero step allows decimals.
            setting.minimum = -200.0;
            setting.maximum = 500.0;
            setting.step = 0;
            setting.default_value.real = entry.Default;

            if (gApi->register_setting(gMod, &setting) != SC_OK)
                return false;
        }

        return PublishCurrentIniValues();
    }
}

namespace SleekConfigIntegration
{
    void TryAttach()
    {
        if (gMod != 0)
        {
            if (GetModuleHandleA("SleekConfig.asi"))
                return;

            // A mod loader may unload/reload the shared menu while the game
            // stays running. Avoid reusing pointers into its old DLL.
            gApi = nullptr;
            gMod = 0;
        }

        // Do not load SleekConfig ourselves: independent installation and
        // game startup order must not affect CleanHUD's normal operation.
        const ScApi* api = sleekconfig::FindApi();

        if (!api)
            return;

        if (!(api->capabilities & SC_CAP_EXTERNAL_BACKEND) ||
            !api->publish_value)
        {
            return;
        }

        ScModDesc mod = {};
        mod.size = sizeof(mod);
        mod.required_api = SC_API_VERSION;
        mod.id = ModId;
        mod.name = "CleanHUD";
        mod.author = "prixsm";
        mod.description = "HUD visibility, modes and live Stack spacing";
        mod.version = "1.0";
        mod.category = "Visuals";
        mod.persistence = SC_EXTERNAL;
        mod.on_change = OnChanged;
        mod.on_load = OnLoad;
        mod.on_save = OnSave;

        ScMod registered = 0;
        if (api->register_mod(&mod, &registered) != SC_OK)
            return;

        gApi = api;
        gMod = registered;

        if (!RegisterSettings())
        {
            api->unregister_mod(registered);
            gApi = nullptr;
            gMod = 0;
        }
    }

    void Detach()
    {
        // Do not dereference a stale API pointer if another loader has
        // already unloaded SleekConfig. Its registry also tracks owners.
        if (gMod && GetModuleHandleA("SleekConfig.asi"))
            gApi->unregister_mod(gMod);

        gApi = nullptr;
        gMod = 0;
    }
}
