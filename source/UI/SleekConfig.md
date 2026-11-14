# CleanHUD + SleekConfig

CleanHUD's in-game settings are registered with the separate
[SleekConfig](https://github.com/pedrolecio/SleekConfig) GTA SA settings menu.

## Installing

1. Install CleanHUD as usual (GTA SA PC 1.0 US, Win32 ASI loader).
2. Install the separate `SleekConfig.asi` from the SleekConfig release,
   following SleekConfig's Mod Loader installation instructions.
3. Launch GTA SA, press **F10**, choose **CleanHUD** under Visuals.

If SleekConfig is not installed, CleanHUD runs normally using CleanHUD.ini,
without an F10 configuration panel or error.

## Live editing

Controls are organized under **General**, **Elements**, **Visibility**
and **Advanced Layout**. General contains HUD layout, clock behavior,
stack animation, weapon/aim behavior, and special-icon visibility.
Elements and Visibility have no additional groups. Advanced Layout
keeps its named spacing and positioning groups.

Edits run on the game main thread through SleekConfig's change callback.
CleanHUD's HUDSettings instance is changed directly, so ordinary HUD updates
apply the value immediately; **no ASI/game restart** is needed.

`SlideStack=1` animates the space created when temporary elements
enter above the visible stack. Health, armour and breath bars also
slide into their own slots as they fade in. The stack does not slide
backward during fade-out, and Minimal mode keeps native positions.

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
gaps). Jetpack X/Y and skateboard X/Y edits preview the actually
equipped/ridden equipment in both Stack and Minimal. All previews are disabled during
startup/loading, preventing HUD flashes. They don't force a Save:
use **Save Mod** or a SleekConfig preset when satisfied with the layout.

## SleekConfig 1.1 features

SleekConfig **v1.1.0** adds a compact menu
layout, improved fine-dragging and named presets. These features come from
the separately installed `SleekConfig.asi`; CleanHUD does not need to
duplicate their rendering or add its own preset storage.

Update SleekConfig to **1.1.0 or newer** and keep only one copy of its
ASI active. In F10, choose CleanHUD and use **Presets** to save a named
configuration or load it during gameplay.

For exact spacing edits: SleekConfig's sliders follow the mouse,
Shift offers fine adjustments, and Ctrl+click or double-click lets you
enter a number. The 19 Advanced Layout sliders use **1.0** steps
and a more practical **-1500 to +1500** range.

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

Settings retain their CleanHUD.ini keys across reloads and saves.

## Build

CleanHUD vendors SleekConfig's API headers and MIT license under
`third_party/sleekconfig/`. SleekConfig handles Dear ImGui and its
backends independently. No `SleekConfig.lib` or additional linker
dependency is needed.
The `SleekConfig_GetApi` entry point is resolved dynamically from the
separately installed SleekConfig.asi. Registration retries on the main thread
if SleekConfig becomes available later.

The vendored SleekConfig API headers match the supported integration.
Target: GTA San Andreas PC 1.0 US, Win32.

## Regression tests to perform in game

- Start without SleekConfig. Game/menu text, loading, TAB, both HUD modes,
  firing and fading behave as before. CleanHUD.ini is loaded.
- Start with SleekConfig. F10 opens **only** SleekConfig; CleanHUD is listed
  under Visuals with 40 controls across its pages.
- Change each group of sliders/checkboxes, especially Stack/Minimal, the
  four Clock gaps and Status Bar to Weapon. Layout updates during play.
- Save Mod, exit/restart, verify the modified CleanHUD.ini and
  SleekConfig/prixsm.cleanhud.dat. Verify saved values on restart.
- Reload/Restore Saved and Defaults behavior, including the DAT precedence
  documented above.
- Verify Advanced Layout slider steps are 1.0 and the range is
  -1500.0 to +1500.0. Save Mod formats values like 25.0 (one decimal).
- Verify Fade In and Fade Out use 50 ms steps, and Visibility Duration
  uses 250 ms steps, within the existing 0..2000/0..10000 bounds.
- Test with other SleekConfig-enabled mods: there should be one shared F10
  panel, no duplicate D3D9 hook and no stuck mouse or player controls.
- Verify New Game/Load Game startup, main-menu labels,
  existing vehicle drive-by/vision-goggle behavior and TAB preview.

## Weapon/Ammo fine positioning

Stack mode exposes `WeaponAmmoOffsetX`, `WeaponAmmoOffsetY`, and
`WeaponToAmmoVerticalOffset` in **Advanced Layout → Weapon & Ammo Position**.
Editing these values previews the currently drawable weapon and ammo.
Minimal mode keeps GTA's native positions.

For `WeaponToAmmoVerticalOffset`, start with **83.0** for unmodified GTA
weapon-icon scaling (the default), **79.0** with MixSets
`WeaponIconScaleFix`, or **87.0** with WidescreenFix
`ProportionalWeaponIcon`. These are suggested manual adjustments,
also displayed in SleekConfig's tooltip.

## Optional hidden and custom weapon icons

The `[SpecialIcons]` section has one master toggle, **on by
default**: `ShowHiddenWeaponIcons=1` enables cellphone, jetpack and
custom/unused weapon icons. Existing saved INI/DAT settings that
explicitly disabled it remain respected. Both special equipment icons
use the same visibility
fading settings as the rest of CleanHUD; jetpack visibility is triggered
once on equip and is not extended indefinitely while flying.

The jetpack occupies the normal weapon-icon position only when CJ
has **fists or brass knuckles** selected. For any other selected
weapon—including grenades, tear gas, Molotovs, satchel charges,
melee weapons and custom held weapons—the jetpack appears **left
of the existing weapon icon**, so neither is obscured. A visible
cellphone takes priority in the normal weapon slot, with the
jetpack next to it. `JetpackIconOffsetX` and `JetpackIconOffsetY`
work in either position.

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

### Jetpack behavior

The jetpack defaults to `JetpackIconOffsetX=0.0` and
`JetpackIconOffsetY=0.0`. With fists or knuckles, its icon replaces
the native weapon-slot icon while visible. With any other held
weapon or an active cellphone icon, the jetpack appears to the left
of that icon. The renderer shifts this separate, side-by-side
position **12 virtual X units right**, independently of the user
offset. Positive X moves right and positive Y moves down.

Both offsets use virtual HUD units (screen-width/1920 and
screen-height/1080) in Stack and Minimal modes. Editing either offset
temporarily shows the jetpack for `DurationMs` when equipped, even
after its normal timer expires. This preview does not invent a jetpack
or revive one that has been dropped.

The jetpack's normal visibility begins on equip and fades out using
CleanHUD's visibility timings. A drop starts fading when GTA marks
the jetpack task finished or removes it. Pressing Exit Vehicle in
the air, when the jetpack remains equipped, does not end its HUD
visibility. Equipping or previewing the jetpack also reveals ammo
alongside the selected weapon for the same `DurationMs` timer,
when that weapon has an ammo readout.

CleanHUD's cellphone, jetpack and custom
icons use the **most recently measured native sprite bounds**, so they
follow whichever scaling mod the player uses without injecting another
scaling correction. Measurements persist across transient missing sprite
draws and are discarded on resolution changes.

Because MixSets and WidescreenFix do not use exactly the same scaling,
their relative Weapon/Ammo positions can still differ; the existing
Weapon/Ammo spacing sliders remain the adjustment mechanism. Enabling
both external scaling fixes may compound their adjustments; CleanHUD
does not attempt to override or correct either mod.

Advanced Layout keeps Weapon/Ammo offsets together and jetpack X/Y
offsets in a separate `Jetpack Position` group.

### Optional skateboard riding icon

CleanHUD can show a separate, **opt-in skateboard icon while the
player rides a skateboard vehicle**, without confusing it with the
skateboard weapon held on foot. Enable `[Skateboard]
ShowWhileRiding=1` (or the **Skateboard riding icon** checkbox under
SleekConfig → General) and leave `ShowHiddenWeaponIcons=1`.

The supplied Skate Mod Remake CLEO script uses these configuration
keys in `cleo/skate_remake.ini`: `VehicleID=12500` for the vehicle
spawned on mounting, `WeaponModel=326` for the carried skateboard,
and `WeaponType=15` (Cane) for its weapon slot. CleanHUD defaults to
`VehicleModelId=12500` and `IconModelId=326`; if the script's
values are changed, update the corresponding CleanHUD settings.
Other skateboard mods may use different vehicle/model IDs. This
integration does not read the external skate mod INI automatically.

When riding is detected via the player's **actual occupied vehicle
model**, CleanHUD starts the normal `DurationMs` visibility timer
once. The icon fades in using `FadeInMs` and fades out after that
timer expires. Dismounting immediately ends the timer and uses
`FadeOutMs`. It never appears merely because CJ carries the Cane
weapon; carried skateboard rendering remains handled by the normal
weapon/custom-model icon path.

With fists or brass knuckles selected while riding, the skateboard
occupies the primary weapon-icon slot. With a gun or any other held
weapon, the skateboard icon appears to the left and the weapon/ammo
are shown for the same initial visibility window. The configurable
`IconOffsetX`/`IconOffsetY` (Advanced Layout → Skateboard Position)
apply to either placement. Adjusting them previews the icon while
the board vehicle is actually occupied, in either HUD mode.
If a held skateboard returns immediately after dismount, the fading
riding icon avoids drawing a duplicate of that same model.

Both skateboard and jetpack effects have independent opacity timers.
Should unusual mods allow both simultaneously, the skateboard icon
uses an additional left slot to avoid covering the jetpack icon.
A visible cellphone remains the primary hand-held icon.

The selected `IconModelId` must have a model-name + `ICON` texture
available through WeaponIconsTXD or its native TXD. This mod does
not bundle a skateboard icon or infer one from a vehicle mesh. If
none is available, select an icon model whose texture exists.

The icon renderer checks the `weaponicons` TXD slot used by
[WeaponIconsTXD](https://github.com/JuniorDjjr/WeaponIconsTXD) first,
then the original model TXD. It does not patch WeaponIconsTXD's two
lookup calls. Icons appear only if their model has an `ICON` texture
in one of those dictionaries; no new image assets are bundled.

## Layout range, formatting and visibility cap

All 19 advanced spacing/offset controls accept **-1500 to +1500**
virtual layout units in both SleekConfig and the INI reader. Values
are rounded to whole virtual units and saved with one decimal place.
These controls include the jetpack and skateboard's independent X/Y
positions. Skateboard model IDs are integer options (step 1), and
only affect the optional rider icon.

`DurationMs` is limited to 0..10000 milliseconds (10 seconds) in
both SleekConfig and INI loading. Existing longer values are clamped
on load. FadeInMs/FadeOutMs retain their 0..2000 ranges.

SleekConfig registers a **1.0** increment for each of the 19 Advanced
Layout float sliders, avoiding fractional-step float validation issues.
Direct INI values are normalized to the same whole-unit increments.
The editor displays integer values because its current formatting
follows the step size;
