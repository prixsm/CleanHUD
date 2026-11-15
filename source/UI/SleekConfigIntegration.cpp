#include "SleekConfigIntegration.h"

#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>

#include <windows.h>
#include <sleekconfig.hpp>

#include "../Core/HUDSettings.h"
#include "../Core/HUDManager.h"

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
        int Step;
    };

    struct FloatSetting
    {
        const char* Id;
        const char* Label;
        const char* Group;
        float HUDSettings::* Field;
        float Default;
        HUDSpacingPreviewGroup First;
        HUDSpacingPreviewGroup Second;
    };

    // Simple controls live directly under their page; no extra
    // collapsible headings for General, Elements or Visibility.
    constexpr BoolSetting BoolSettings[] = {
        { "always_show_clock", "Always show Clock", "General", "", &HUDSettings::AlwaysShowClock, true },
        { "slide_stack", "Slide stack on entry", "General", "", &HUDSettings::SlideStack, false },
        { "show_on_foot_aim", "Show on foot aim", "General", "", &HUDSettings::ShowOnFootAim, true },
        { "show_on_throwable_aim", "Show on throwable aim", "General", "", &HUDSettings::ShowOnThrowableAim, false },
        { "show_on_vehicle_look", "Show on vehicle look", "General", "", &HUDSettings::ShowOnVehicleLook, false },
        { "show_hidden_weapon_icons", "Show hidden/custom weapon icons", "General", "", &HUDSettings::ShowHiddenWeaponIcons, true },
        { "show_skateboard_riding_icon", "Skateboard mod support", "Skateboard", "", &HUDSettings::ShowSkateboardRidingIcon, false },
        { "clock", "Clock", "Elements", "", &HUDSettings::Clock, true },
        { "money", "Money", "Elements", "", &HUDSettings::Money, true },
        { "wanted", "Wanted stars", "Elements", "", &HUDSettings::Wanted, true },
        { "health", "Health", "Elements", "", &HUDSettings::Health, true },
        { "armour", "Armour", "Elements", "", &HUDSettings::Armour, true },
        { "breath", "Breath", "Elements", "", &HUDSettings::Breath, true },
        { "weapon", "Weapon icon", "Elements", "", &HUDSettings::Weapon, true },
        { "ammo", "Ammo", "Elements", "", &HUDSettings::Ammo, true }
    };

    constexpr IntSetting IntSettings[] = {
        { "fade_in_ms", "Fade in (ms)", "Visibility", "", &HUDSettings::FadeInMs, 200, 2000, 50 },
        { "fade_out_ms", "Fade out (ms)", "Visibility", "", &HUDSettings::FadeOutMs, 200, 2000, 50 },
        { "duration_ms", "Visibility duration (ms)", "Visibility", "", &HUDSettings::VisibilityMs, 3000, 10000, 250 },
        { "skateboard_vehicle_model_id", "Vehicle model ID", "Skateboard", "", &HUDSettings::SkateboardVehicleModelId, 12500, 32767, 1 },
        { "skateboard_icon_model_id", "Icon model ID", "Skateboard", "", &HUDSettings::SkateboardIconModelId, 12400, 32767, 1 }
    };

    constexpr FloatSetting FloatSettings[] = {
        { "clock_to_money", "Clock to Money", "Clock", &HUDSettings::ClockToMoneySpacing, 25.0f,
            HUDSpacingPreviewGroup::Clock, HUDSpacingPreviewGroup::Money },
        { "clock_to_wanted", "Clock to Wanted", "Clock", &HUDSettings::ClockToWantedSpacing, 25.0f,
            HUDSpacingPreviewGroup::Clock, HUDSpacingPreviewGroup::Wanted },
        { "clock_to_status", "Clock to Status Bar", "Clock", &HUDSettings::ClockToStatusBarSpacing, 22.0f,
            HUDSpacingPreviewGroup::Clock, HUDSpacingPreviewGroup::StatusBars },
        { "clock_to_weapon", "Clock to Weapon", "Clock", &HUDSettings::ClockToWeaponSpacing, 30.0f,
            HUDSpacingPreviewGroup::Clock, HUDSpacingPreviewGroup::Weapon },
        { "money_to_wanted", "Money to Wanted", "Money", &HUDSettings::MoneyToWantedSpacing, 25.0f,
            HUDSpacingPreviewGroup::Money, HUDSpacingPreviewGroup::Wanted },
        { "money_to_status", "Money to Status Bar", "Money", &HUDSettings::MoneyToStatusBarSpacing, 22.0f,
            HUDSpacingPreviewGroup::Money, HUDSpacingPreviewGroup::StatusBars },
        { "money_to_weapon", "Money to Weapon", "Money", &HUDSettings::MoneyToWeaponSpacing, 35.0f,
            HUDSpacingPreviewGroup::Money, HUDSpacingPreviewGroup::Weapon },
        { "wanted_to_status", "Wanted to Status Bar", "Wanted", &HUDSettings::WantedToStatusBarSpacing, 22.0f,
            HUDSpacingPreviewGroup::Wanted, HUDSpacingPreviewGroup::StatusBars },
        { "wanted_to_weapon", "Wanted to Weapon", "Wanted", &HUDSettings::WantedToWeaponSpacing, 32.0f,
            HUDSpacingPreviewGroup::Wanted, HUDSpacingPreviewGroup::Weapon },
        { "status_to_weapon", "Status Bar to Weapon", "Status Bars", &HUDSettings::StatusBarToWeaponSpacing, 17.0f,
            HUDSpacingPreviewGroup::StatusBars, HUDSpacingPreviewGroup::Weapon },
        { "armour_spacing", "Armour spacing", "Individual", &HUDSettings::ArmourSpacing, 4.0f,
            HUDSpacingPreviewGroup::Armour, HUDSpacingPreviewGroup::StatusBars },
        { "breath_spacing", "Breath spacing", "Individual", &HUDSettings::BreathSpacing, 4.0f,
            HUDSpacingPreviewGroup::Breath, HUDSpacingPreviewGroup::Health },
        { "weapon_ammo_offset_x", "Weapon/Ammo offset X", "Weapon & Ammo Position", &HUDSettings::WeaponAmmoOffsetX, 12.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Ammo },
        { "weapon_ammo_offset_y", "Weapon/Ammo offset Y", "Weapon & Ammo Position", &HUDSettings::WeaponAmmoOffsetY, -16.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Ammo },
        { "weapon_to_ammo_vertical_offset", "Weapon to Ammo vertical offset", "Weapon & Ammo Position", &HUDSettings::WeaponToAmmoVerticalOffset, 83.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Ammo },
        { "jetpack_icon_offset_x", "Jetpack icon offset X", "Jetpack Position", &HUDSettings::JetpackIconOffsetX, 0.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Weapon },
        { "jetpack_icon_offset_y", "Jetpack icon offset Y", "Jetpack Position", &HUDSettings::JetpackIconOffsetY, 0.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Weapon },
        { "skateboard_icon_offset_x", "Skateboard icon offset X", "Skateboard Position", &HUDSettings::SkateboardIconOffsetX, 0.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Weapon },
        { "skateboard_icon_offset_y", "Skateboard icon offset Y", "Skateboard Position", &HUDSettings::SkateboardIconOffsetY, 0.0f,
            HUDSpacingPreviewGroup::Weapon, HUDSpacingPreviewGroup::Weapon }
    };


    // SleekConfig's SC_ENUM supplies a scrollable, labelled list.
    // Values in CleanHUD.ini remain raw Win32 virtual-key codes.
    // Use a fresh setting ID for this enum: the old SC_KEY setting
    // saved raw VK numbers, which must not be mistaken for enum indices.
    struct PreviewKeyChoice
    {
        int VirtualKey;
        const char* Label;
    };

    constexpr PreviewKeyChoice PreviewKeyChoices[] = {
        { 0, "Off" },
        { 8, "Backspace" },
        { 9, "Tab" },
        { 13, "Enter" },
        { 16, "Shift" },
        { 17, "Ctrl" },
        { 18, "Alt" },
        { 19, "Pause" },
        { 20, "Caps Lock" },
        { 27, "Escape" },
        { 32, "Space" },
        { 33, "Page Up" },
        { 34, "Page Down" },
        { 35, "End" },
        { 36, "Home" },
        { 37, "Left Arrow" },
        { 38, "Up Arrow" },
        { 39, "Right Arrow" },
        { 40, "Down Arrow" },
        { 45, "Insert" },
        { 46, "Delete" },
        { 48, "0" },
        { 49, "1" },
        { 50, "2" },
        { 51, "3" },
        { 52, "4" },
        { 53, "5" },
        { 54, "6" },
        { 55, "7" },
        { 56, "8" },
        { 57, "9" },
        { 65, "A" },
        { 66, "B" },
        { 67, "C" },
        { 68, "D" },
        { 69, "E" },
        { 70, "F" },
        { 71, "G" },
        { 72, "H" },
        { 73, "I" },
        { 74, "J" },
        { 75, "K" },
        { 76, "L" },
        { 77, "M" },
        { 78, "N" },
        { 79, "O" },
        { 80, "P" },
        { 81, "Q" },
        { 82, "R" },
        { 83, "S" },
        { 84, "T" },
        { 85, "U" },
        { 86, "V" },
        { 87, "W" },
        { 88, "X" },
        { 89, "Y" },
        { 90, "Z" },
        { 96, "Numpad 0" },
        { 97, "Numpad 1" },
        { 98, "Numpad 2" },
        { 99, "Numpad 3" },
        { 100, "Numpad 4" },
        { 101, "Numpad 5" },
        { 102, "Numpad 6" },
        { 103, "Numpad 7" },
        { 104, "Numpad 8" },
        { 105, "Numpad 9" },
        { 106, "Numpad *" },
        { 107, "Numpad +" },
        { 109, "Numpad -" },
        { 110, "Numpad ." },
        { 111, "Numpad /" },
        { 112, "F1" },
        { 113, "F2" },
        { 114, "F3" },
        { 115, "F4" },
        { 116, "F5" },
        { 117, "F6" },
        { 118, "F7" },
        { 119, "F8" },
        { 120, "F9" },
        { 121, "F10" },
        { 122, "F11" },
        { 123, "F12" },
        { 144, "Num Lock" },
        { 145, "Scroll Lock" },
        { 160, "Left Shift" },
        { 161, "Right Shift" },
        { 162, "Left Ctrl" },
        { 163, "Right Ctrl" },
        { 164, "Left Alt" },
        { 165, "Right Alt" },
        { 186, "Semicolon" },
        { 187, "Plus/Equals" },
        { 188, "Comma" },
        { 189, "Minus" },
        { 190, "Period" },
        { 191, "Slash" },
        { 192, "Grave" },
        { 219, "Left Bracket" },
        { 220, "Backslash" },
        { 221, "Right Bracket" },
        { 222, "Apostrophe" },
    };

    constexpr int PreviewKeyCount =
        static_cast<int>(sizeof(PreviewKeyChoices) /
            sizeof(PreviewKeyChoices[0]));
    static_assert(PreviewKeyCount < 255,
        "Keyboard VK choices must fit SleekConfig's enum limit");

    int PreviewKeyIndex(std::uint32_t vk)
    {
        for (int i = 0; i < PreviewKeyCount; ++i)
        {
            if (PreviewKeyChoices[i].VirtualKey ==
                static_cast<int>(vk))
                return i;
        }

        // Retain uncommon custom keys loaded from an existing INI.
        // This last menu entry does not modify their raw VK value.
        return PreviewKeyCount;
    }

    const std::string& PreviewKeyOptions()
    {
        static const std::string options = []()
        {
            std::string output;
            for (const auto& choice : PreviewKeyChoices)
            {
                if (!output.empty())
                    output += '|';
                output += std::to_string(choice.VirtualKey);
                output += " - ";
                output += choice.Label;
            }
            output += "|Custom VK (configured in INI)";
            return output;
        }();
        return options;
    }

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

        // Shown in SleekConfig's setting tooltip, not a new group.
        if (std::strcmp(id, "vanilla_hud_key_choice") == 0)
            setting.description =
                "Select a labelled Windows virtual-key (VK) code. "
                "Choose Off to disable; Tab is the default. "
                "For any other VK code (0..255), edit CleanHUD.ini.";
        else if (std::strcmp(id, "vanilla_hud_pad_button") == 0)
            setting.description =
                "Select a GTA gamepad action, with Xbox and PlayStation "
                "labels. Off is the default. GTA/GInput can also map "
                "keyboard keys to these same pad actions (e.g. "
                "Ctrl may activate LB/L1). Not physical-pad-only.";
        else if (std::strcmp(id, "hud_mode") == 0)
            setting.description =
                "Stack arranges temporary HUD elements vertically; "
                "Minimal fades elements at GTA's original positions.";
        else if (std::strcmp(id, "always_show_clock") == 0)
            setting.description =
                "Keep the clock visible above the Stack. When off, "
                "the managed clock fades with CleanHUD visibility rules.";
        else if (std::strcmp(id, "slide_stack") == 0)
            setting.description =
                "In Stack mode, smooth the position changes as a newly "
                "visible element makes room above other HUD elements.";
        else if (std::strcmp(id, "show_on_foot_aim") == 0)
            setting.description =
                "Reveal weapon/ammo while aiming on foot. While using "
                "a jetpack, an enabled weapon or actual shot is required.";
        else if (std::strcmp(id, "show_on_throwable_aim") == 0)
            setting.description =
                "Allow aim-button reveal for grenades, tear gas, "
                "Molotovs and satchel charges (for aiming mods).";
        else if (std::strcmp(id, "show_on_vehicle_look") == 0)
            setting.description =
                "Reveal weapon/ammo when looking left or right in a "
                "vehicle; drive-by firing reveals them regardless.";
        else if (std::strcmp(id, "show_skateboard_riding_icon") == 0)
            setting.description =
                "Enable skateboard vehicle-icon support for mods such "
                "as Skate Mod Remake. Requires Show hidden/custom "
                "weapon icons enabled.";
        else if (std::strcmp(id, "skateboard_vehicle_model_id") == 0)
            setting.description =
                "Matches the vehicle CJ rides (Skate Mod Remake: 12500). "
                "Ctrl+click or double-click the number to type an ID.";
        else if (std::strcmp(id, "skateboard_icon_model_id") == 0)
            setting.description =
                "Model whose ICON texture depicts the board "
                "(added Skate Mod Remake: 12400). Ctrl+click or "
                "double-click the number to type an ID.";
        return setting;
    }

    bool PublishCurrentIniValues()
    {
        if (!gApi || !gMod)
            return false;

        // register_setting uses its declared default, not the loaded INI.
        // Publish INI values before the first SleekConfig registry tick.
        // Saved SleekConfig DAT overrides are applied after these values.
        HUDSettings& settings = HUDSettings::Get();
        ScValue value = {};

        value.type = SC_ENUM;
        value.integer = PreviewKeyIndex(settings.VanillaHUDKey);
        if (gApi->publish_value(gMod, "vanilla_hud_key_choice",
            &value) != SC_OK)
            return false;

        value = {};
        value.type = SC_ENUM;
        value.integer = static_cast<int>(
            settings.VanillaHUDPadButton);
        if (gApi->publish_value(gMod, "vanilla_hud_pad_button",
            &value) != SC_OK)
            return false;

        value = {};
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
            const int original = static_cast<int>(
                settings.*(entry.Field));
            const int minimum = entry.Step == 1 ? 1 : 0;
            value.integer = std::clamp(
                ((original + entry.Step / 2) / entry.Step) *
                    entry.Step,
                minimum, entry.Maximum);

            if (gApi->publish_value(gMod, entry.Id, &value) != SC_OK)
                return false;

            // Legacy INI values may not land exactly on the menu step.
            settings.*(entry.Field) =
                static_cast<std::uint32_t>(value.integer);
        }

        for (const auto& entry : FloatSettings)
        {
            value = {};
            value.type = SC_FLOAT;
            // INI values from older releases may use fractional steps.
            // Publish whole units so the registry accepts step=1.0.
            value.real = std::round(std::clamp(
                settings.*(entry.Field), -1500.0f, 1500.0f));

            if (gApi->publish_value(gMod, entry.Id, &value) != SC_OK)
                return false;

            // Keep the active layout in sync with the rounded values
            // displayed by SleekConfig.
            settings.*(entry.Field) = value.real;
        }

        return true;
    }

    int SC_CALL OnChanged(
        void*, const char* id, const ScValue* proposed)
    {
        if (!id || !proposed)
            return 0;

        HUDSettings& settings = HUDSettings::Get();

        if (std::strcmp(id, "vanilla_hud_key_choice") == 0)
        {
            if (proposed->type != SC_ENUM ||
                proposed->integer < 0 ||
                proposed->integer > PreviewKeyCount)
                return 0;
            if (proposed->integer < PreviewKeyCount)
            {
                settings.VanillaHUDKey =
                    static_cast<std::uint32_t>(
                        PreviewKeyChoices[
                            proposed->integer].VirtualKey);
            }
            // Custom VK option preserves the already configured INI VK.
            return 1;
        }

        if (std::strcmp(id, "vanilla_hud_pad_button") == 0)
        {
            if (proposed->type != SC_ENUM ||
                proposed->integer < 0 || proposed->integer > 16)
                return 0;
            settings.VanillaHUDPadButton =
                static_cast<std::uint32_t>(proposed->integer);
            return 1;
        }

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

            if (entry.Field == &HUDSettings::SkateboardVehicleModelId ||
                entry.Field == &HUDSettings::SkateboardIconModelId)
            {
                HUDManager::Get().PreviewSkateboardPosition();
            }
            return 1;
        }

        for (const auto& entry : FloatSettings)
        {
            if (std::strcmp(id, entry.Id) != 0)
                continue;

            if (proposed->type != SC_FLOAT)
                return 0;

            const float previous = settings.*(entry.Field);
            settings.*(entry.Field) = proposed->real;

            // Special jetpack X/Y preview requires a real equipped
            // jetpack; unlike stack-spacing previews, it must also
            // work in Minimal mode. Other sliders keep their normal
            // adjacent-group previews.
            if (previous != proposed->real)
            {
                if (entry.Field == &HUDSettings::JetpackIconOffsetX ||
                    entry.Field == &HUDSettings::JetpackIconOffsetY)
                {
                    HUDManager::Get().PreviewJetpackPosition();
                }
                else if (entry.Field == &HUDSettings::SkateboardIconOffsetX ||
                         entry.Field == &HUDSettings::SkateboardIconOffsetY)
                {
                    HUDManager::Get().PreviewSkateboardPosition();
                }
                else
                {
                    HUDManager::Get().PreviewSpacing(
                        entry.First, entry.Second);
                }
            }

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
            "hud_mode", "HUD layout", "General", "", SC_ENUM);

        mode.options = "Stack | Minimal";
        mode.minimum = 0;
        mode.maximum = 1;
        mode.step = 1;
        mode.default_value.integer = 0;

        if (gApi->register_setting(gMod, &mode) != SC_OK)
            return false;

        // One page-level explanation replaces eight repeated tooltips.
        // This is static help text, not a saved or user-editable option.
        ScSettingDesc elementsHelp = Describe(
            "elements_behavior_help", "Elements behavior",
            "Elements", "", SC_DESCRIPTION);
        elementsHelp.description =
            "On: CleanHUD manages this element's visibility and layout. "
            "Off: leave its rendering to GTA or another HUD mod.";

        if (gApi->register_setting(gMod, &elementsHelp) != SC_OK)
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

            setting.minimum = entry.Step == 1 ? 1 : 0;
            setting.maximum = entry.Maximum;
            setting.step = entry.Step;
            setting.default_value.integer = entry.Default;

            if (gApi->register_setting(gMod, &setting) != SC_OK)
                return false;
        }

        for (const auto& entry : FloatSettings)
        {
            const bool skateboardOffset =
                entry.Field == &HUDSettings::SkateboardIconOffsetX ||
                entry.Field == &HUDSettings::SkateboardIconOffsetY;
            auto setting = Describe(
                entry.Id, entry.Label,
                skateboardOffset ? "Skateboard" : "Advanced Layout",
                skateboardOffset ? "" : entry.Group, SC_FLOAT);

            // The rendered layout moves at whole-unit increments.
            // SleekConfig 1.1 determines numeric formatting from
            // step (1.0 is displayed as an integer in its UI).
            setting.minimum = -1500.0;
            setting.maximum = 1500.0;
            setting.step = 1.0;
            setting.default_value.real = entry.Default;

            if (std::strcmp(entry.Id, "weapon_to_ammo_vertical_offset") == 0)
            {
                setting.description =
                    "Set this to 87.0 if you have ProportionalWeaponIcon = 1 "
                    "on your WidescreenFix.ini.";
            }

            if (gApi->register_setting(gMod, &setting) != SC_OK)
                return false;
        }


        // HUD Preview is registered last: its controls form the final
        // SleekConfig page, with their own complete choice lists.
        ScSettingDesc help = Describe(
            "hud_preview_help", "Hold to show vanilla positions",
            "HUD Preview", "", SC_DESCRIPTION);
        help.description =
            "Hold your chosen key or gamepad button to preview GTA's "
            "original HUD positions. Release to resume CleanHUD. "
            "Both dropdowns include the numeric INI codes; avoid "
            "buttons already used by the game.";
        if (gApi->register_setting(gMod, &help) != SC_OK)
            return false;

        ScSettingDesc keyboard = Describe(
            "vanilla_hud_key_choice", "Keyboard virtual key (VK)",
            "HUD Preview", "", SC_ENUM);
        keyboard.options = PreviewKeyOptions().c_str();
        keyboard.minimum = 0;
        keyboard.maximum = PreviewKeyCount;
        keyboard.step = 1;
        keyboard.default_value.integer = PreviewKeyIndex(9); // Tab
        if (gApi->register_setting(gMod, &keyboard) != SC_OK)
            return false;

        ScSettingDesc padButton = Describe(
            "vanilla_hud_pad_button", "Gamepad / mapped GTA button",
            "HUD Preview", "", SC_ENUM);
        padButton.options =
            "0 - Off|1 - LB/L1|2 - LT/L2|3 - RB/R1|4 - RT/R2|5 - D-pad Up|"
            "6 - D-pad Down|7 - D-pad Left|8 - D-pad Right|9 - Back/Select|"
            "10 - Start|11 - X/Square|12 - Y/Triangle|13 - A/Cross|"
            "14 - B/Circle|15 - Left stick click/L3|16 - Right stick click/R3";
        padButton.minimum = 0;
        padButton.maximum = 16;
        padButton.step = 1;
        padButton.default_value.integer = 0; // Off
        if (gApi->register_setting(gMod, &padButton) != SC_OK)
            return false;

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
