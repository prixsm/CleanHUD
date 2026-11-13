# CleanHUD + SleekConfig

CleanHUD's in-game settings are now registered with the separate
[SleekConfig](https://github.com/pedrolecio/SleekConfig) GTA SA settings menu.
CleanHUD does **not** include its own ImGui renderer or window hooks.

## Installing

1. Install CleanHUD as usual (GTA SA PC 1.0 US, Win32 ASI loader).
2. Install the separate `SleekConfig.asi` from the SleekConfig release,
   following SleekConfig's Mod Loader installation instructions.
3. Launch GTA SA, press **F10**, choose **CleanHUD** under Visuals.

If SleekConfig is not installed, CleanHUD runs normally using CleanHUD.ini,
without an F10 configuration panel or error.

## Live editing

Controls cover General (Stack/Minimal, Clock, optional slide),
Visibility (fade and show duration), Elements, Weapon & Ammo and
Advanced Layout. The Advanced Layout page is divided into Clock, Money,
Wanted, Status Bars and Individual spacing groups.

Edits run on the game main thread through SleekConfig's change callback.
CleanHUD's HUDSettings instance is changed directly, so ordinary HUD updates
apply the value immediately; **no ASI/game restart** is needed. Editing a
spacing value does not invent game state; ordinary Stack spacing sliders
preview their actual neighboring HUD groups. Jetpack position edits preview
the icon only if CJ has a jetpack equipped.

SleekConfig owns the F10 hotkey and handles player input, keyboard/mouse,
Direct3D9 render hooks and graphics-device lifecycle. CleanHUD does not
register duplicate F10/D3D9 hooks.

## Live spacing previews

Changing any of the 15 stack-spacing/Weapon/Ammo layout sliders in **Stack mode**
temporarily reveals its two neighboring HUD groups for the configured
`[Visibility] DurationMs` interval. Every new slider adjustment refreshes
that interval. For example, adjusting **Money to Wanted** previews Money
and the currently visible wanted stars, while **Wanted to Status Bar**
previews wanted stars plus Health and any currently drawable status bars.

Other examples:
- Clock-to-X previews Clock even when `AlwaysShowClock=0`. With
  `Clock=0`, the vanilla Clock is still controlled by GTA.
- Weapon and Ammo spacing previews the equipped weapon icon and its ammo,
  when relevant for the currently equipped weapon.
- Armour and Breath are only previewed if those bars can actually be
  drawn (existing armour, swimming/depleted breath). Health is the
  fallback reference bar for status spacing.
- Wanted stars are previewable **only when the player actually has a
  wanted level**. CleanHUD does not invent wanted stars or alter gameplay
  wanted levels when there are none.
- Disabled `[Elements]` are never forced by the preview; GTA/other mods
  keep control over them.

Stack spacing previews are inactive in Minimal mode (which has no stack
gaps). Jetpack X/Y edits are the exception: they preview the equipped
jetpack in both Stack and Minimal. All previews are disabled during
startup/loading, preventing HUD flashes. They don't force a Save:
use **Save Mod** or a SleekConfig preset when satisfied with the layout.

## SleekConfig 1.1 features

SleekConfig **v1.1.0** (released November 9, 2026) adds a compact menu
layout, improved fine-dragging and named presets. These features come from
the separately installed `SleekConfig.asi`; CleanHUD does not need to
duplicate their rendering or add its own preset storage.

Update SleekConfig to **1.1.0 or newer** and keep only one copy of its
ASI active. In F10, choose CleanHUD and use **Presets** to save a named
configuration or load it during gameplay. Existing registered setting IDs
that remain registered are unchanged; with the experimental scale toggle
removed, the editor returns to **35 controls**.

For exact spacing edits: SleekConfig's sliders follow the mouse,
Shift offers fine adjustments, and Ctrl+click or double-click lets you
enter a number. The 17 Advanced Layout sliders use **1.0** steps
and a more practical **-1500 to +1500** range. Old fractional values
are rounded to the nearest whole virtual unit on INI load or when
published to SleekConfig. Older DAT presets with fractional values
may need to be re-saved.

SleekConfig v1.1 chooses its number of visible decimal places from the
step. A 1.0 step displays an integer in its own slider; the API does
not currently offer per-setting formatting.

Save Mod records your selection in CleanHUD.ini and in SleekConfig's
regular DAT settings. Loading a preset changes the live settings; it
does not automatically overwrite CleanHUD.ini until you save.

## Persistence and precedence

CleanHUD loads `CleanHUD.ini` on startup, even without SleekConfig.

When SleekConfig is installed, its persisted
`SleekConfig/prixsm.cleanhud.dat` overrides matching INI settings.
That precedence is part of SleekConfig's current upstream design:

1. Registered defaults.
2. CleanHUD.ini values.
3. Saved SleekConfig DAT overrides.

Click **Save Mod** in SleekConfig to save **both** the current CleanHUD.ini
and SleekConfig's DAT snapshot. CleanHUD's existing Win32 `Save()` method
writes its named INI settings using one decimal place for spacing and
normalizes separators to `Key = Value`, while SleekConfig saves its own .dat file.
A failed INI write aborts SleekConfig's Save action.

**Manual INI edits made while an older SleekConfig DAT exists may not take
effect** because DAT takes priority. To restore INI-driven values, remove the
CleanHUD DAT override while the game is closed, then relaunch.

**Restore Saved** reloads the INI baseline, then applies saved DAT values
(where available). **Defaults** restores the registered CleanHUD defaults
as live, unsaved values until the next Save Mod.

No automatic migration of previously saved standalone-editor settings is
needed because both editors use the same CleanHUD.ini keys.

## Build

CleanHUD only vendors the two small SleekConfig API headers and its MIT
license under `third_party/sleekconfig/`; it no longer builds Dear ImGui or
its backends. No `SleekConfig.lib` or additional linker dependency is needed.
The `SleekConfig_GetApi` entry point is resolved dynamically from the
separately installed SleekConfig.asi. Registration retries on the main thread
if SleekConfig becomes available later.

The SleekConfig **v1 API** remains unchanged in v1.1.0; the vendored
headers still match upstream. Target: GTA San Andreas PC 1.0 US, Win32.

## Regression tests to perform in game

- Start without SleekConfig. Game/menu text, loading, TAB, both HUD modes,
  firing and fading behave as before. CleanHUD.ini is loaded.
- Start with SleekConfig. F10 opens **only** SleekConfig; CleanHUD is listed
  under Visuals with 35 controls across its pages.
- Change each group of sliders/checkboxes, especially Stack/Minimal, the
  four Clock gaps and Status Bar to Weapon. Layout updates during play.
- Save Mod, exit/restart, verify the modified CleanHUD.ini and
  SleekConfig/prixsm.cleanhud.dat. Verify saved values on restart.
- Reload/Restore Saved and Defaults behavior, including the DAT precedence
  documented above.
- Verify Advanced Layout slider steps are 1.0 and the range is
  -1500.0 to +1500.0. Loading a legacy 27.5 INI position rounds to
  28.0. Save Mod formats values like 25.0 (one decimal).
- Verify fade/duration ranges still accept 0 to 2000/10000.
- Test with other SleekConfig-enabled mods: there should be one shared F10
  panel, no duplicate D3D9 hook and no stuck mouse or player controls.
- Verify New Game/Load Game startup (no HUD flash), main-menu labels,
  existing vehicle drive-by/vision-goggle behavior and TAB preview.

## Weapon/Ammo fine positioning

Stack mode exposes `WeaponAmmoOffsetX`, `WeaponAmmoOffsetY`, and
`WeaponToAmmoVerticalOffset` in **Advanced Layout → Weapon & Ammo Position**.
These replace the ineffective `WeaponSpacing`, `AmmoSpacing`, and
`HealthSpacing` fields. SleekConfig changes preview the currently drawable
Weapon and Ammo. Minimal mode retains GTA's native positions.

For `WeaponToAmmoVerticalOffset`, the suggested values are **79.0**
when using MixSets `WeaponIconScaleFix`, or **87.0** when using
WidescreenFix `ProportionalWeaponIcon` (the default). These are
user-tested starting points, not automatic scale-fix detection.
SleekConfig shows this recommendation in the setting's hover description.

## Optional hidden and custom weapon icons

The `[SpecialIcons]` section has one master toggle, now **on by
default**: `ShowHiddenWeaponIcons=1` enables cellphone, jetpack and
custom/unused weapon icons. Existing saved INI/DAT settings that
explicitly disabled it remain respected. Both special equipment icons
use the same visibility
fading settings as the rest of CleanHUD; jetpack visibility is triggered
once on equip and is not extended indefinitely while flying.

The jetpack now defaults to a slot left of the weapon icon, regardless
of which weapon CJ is holding. Its X/Y position can be freely adjusted
using `JetpackIconOffsetX` and `JetpackIconOffsetY`, rather than a
side-by-side mode toggle.

The model of a custom equipped weapon is resolved automatically
using GTA's `CPed::m_nWeaponModelId`, which identifies the model
currently attached to the player's hand. CleanHUD compares it with
the active weapon's ordinary model from `CWeaponInfo`. If they
match, GTA/WeaponIconsTXD already handles its icon and CleanHUD does
not draw a duplicate. If a compatible mod attaches a different model
(such as a skateboard using a repurposed Cane/Shovel type or unused
weapon type 47), CleanHUD looks up that model's `ICON` texture.

No numeric model ID, user-maintained mapping or full model-registry
scan is needed. This detects weapon-model replacements, but **cannot
identify standalone CLEO-attached skateboard props or independent
scripted renderers** that do not update GTA's held weapon model.
Missing models/textures are ignored. Hidden weapon icons default on,
but can still be disabled in the INI or SleekConfig.

### Jetpack, phone and scaling behavior

The jetpack icon starts `DurationMs` once when CJ equips it, using
`FadeInMs` and `FadeOutMs` as normal. It may fade away even while
the jetpack remains equipped; equipping it again retriggers the timer.
Pressing **Exit Vehicle** while jetpacking starts the icon's fade-out
immediately and prevents its visibility timer from restarting until
a new equip. Its existing alpha fades out for `FadeOutMs`.

The cellphone icon is held visible during the active phone-use task.
Once the phone-out/unequip animation begins, CleanHUD ends the
visibility timer but keeps its icon drawn through **FadeOutMs**.
The same applies when dropping the jetpack with Exit Vehicle.
Both stop their visibility lifetime immediately, then fade smoothly.
The ordinary fist/knuckle icon remains suppressed while the
cellphone icon has visible alpha.

The jetpack icon is anchored to the left of the weapon slot at
`JetpackIconOffsetX=0`, `JetpackIconOffsetY=0`, with a small gap.
Positive X moves right, positive Y moves down. Both use independent
virtual HUD units (screen-width/1920 and screen-height/1080), and work
with Stack as well as Minimal HUD mode. Changing either jetpack
position slider temporarily re-shows the jetpack icon for `DurationMs`
when the jetpack is actually equipped, even if its original on-equip
visibility timer expired. No preview occurs when the jetpack has
been dropped or the icon option is disabled.

CleanHUD no longer has a unified weapon-icon scale feature. Native
fists and weapons keep their normal render geometry, including changes
made by MixSets `WeaponIconScaleFix` or WidescreenFix's
`ProportionalWeaponIcon`. CleanHUD's cellphone, jetpack and custom
icons use the **most recently measured native sprite bounds**, so they
follow whichever scaling mod the player uses without injecting another
scaling correction. Measurements persist across transient missing sprite
draws and are discarded on resolution changes.

Because MixSets and WidescreenFix do not use exactly the same scaling,
their relative Weapon/Ammo positions can still differ; the existing
Weapon/Ammo spacing sliders remain the adjustment mechanism. Enabling
both external scaling fixes may compound their adjustments; CleanHUD
does not attempt to override or correct either mod.

SleekConfig groups all three Weapon/Ammo offsets together, followed by
one `Jetpack Position` group containing both X and Y sliders.

The icon renderer checks the `weaponicons` TXD slot used by
[WeaponIconsTXD](https://github.com/JuniorDjjr/WeaponIconsTXD) first,
then the original model TXD. It does not patch WeaponIconsTXD's two
lookup calls. Icons appear only if their model has an `ICON` texture
in one of those dictionaries; no new image assets are bundled.

## Layout range, formatting and visibility cap

All 17 advanced spacing/offset controls accept **-1500 to +1500**
virtual layout units in both SleekConfig and the INI reader. Values
are rounded to whole virtual units and saved with one decimal place.
These controls include the jetpack's independent X/Y positions.

`DurationMs` is limited to 0..10000 milliseconds (10 seconds) in
both SleekConfig and INI loading. Existing longer values are clamped
on load. FadeInMs/FadeOutMs retain their 0..2000 ranges.

SleekConfig registers a **1.0** increment for each of the 17 Advanced
Layout float sliders, avoiding fractional-step float validation issues.
Direct INI values are normalized to the same whole-unit increments.
The editor displays integer values because its current formatting
follows the step size;
