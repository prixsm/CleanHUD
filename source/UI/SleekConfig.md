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

Controls use **General**, **Elements**, **Visibility**, **Skateboard**
and **Advanced Layout** pages. General contains HUD layout, clock and
aiming behavior. Elements starts with a page-wide explanation:
**On** means CleanHUD controls visibility/layout; **Off** means GTA
or another HUD mod retains control of that element. Skateboard
contains its opt-in toggle, both model IDs and both position offsets.
Advanced Layout keeps the general Stack/Weapon/Jetpack groups.

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

## Hold to show GTA's original HUD positions

The first two controls on SleekConfig → **General** are
**Show vanilla HUD: keyboard key** (a key-binding picker, default Tab)
and **Show vanilla HUD: gamepad button** (dropdown, default Off).
Holding either temporarily displays GTA's native HUD positions and
reveals enabled elements using the existing preview behavior.
Releasing the control restores CleanHUD's selected layout after
its normal visibility/fade transition; this does not change
`HUDMode` or modify game input.

In `[General]` in CleanHUD.ini, the equivalent keys are
`VanillaHUDKey=9` (Windows VK_TAB) and
`VanillaHUDPadButton=0` (Off). Set `VanillaHUDKey=0`
to disable the keyboard binding. The controller setting accepts
0..16: 1..4 are LB/LT/RB/RT, 5..8 D-pad directions, 9 Back,
10 Start, 11..14 X/Y/A/B, and 15..16 stick clicks, with
PlayStation equivalents. The gamepad binding uses GTA's
`CPad::NewState`, which controller mods such as GInput also update.
Choose an action that doesn't conflict with the game's own controls;
a mapped keyboard action may affect the same CPad button.

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
enter a number. The 17 Advanced Layout sliders and two Skateboard
offset sliders use **1.0** steps and **-1500 to +1500** bounds.

**Model IDs are still native SleekConfig integer controls, not true
spinners**: the current SleekConfig API has no spinner widget selection.
To type a model ID directly instead of dragging over 1..32767,
**Ctrl+click or double-click** its number. Upstream must add a
spinner-compatible control before CleanHUD can request one while
retaining existing typed setting IDs/presets.

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
  under Visuals with 42 editable controls and one Elements help row.
- On General, verify the **first** two settings are the native HUD
  keyboard key and gamepad button. Tab remains default on keyboard;
  controller defaults Off. Bind an alternate key/button, hold it,
  release it, and verify native positioning then Stack return.
  Also check `VanillaHUDKey=0`, key capture, INI/DAT save/reload,
  and GInput-mapped controller actions.
- Change each group of sliders/checkboxes, especially Stack/Minimal, the
  four Clock gaps and Status Bar to Weapon. Layout updates during play.
- Save Mod, exit/restart, verify the modified CleanHUD.ini and
  SleekConfig/prixsm.cleanhud.dat. Verify saved values on restart.
- Reload/Restore Saved and Defaults behavior, including the DAT precedence
  documented above.
- Verify Advanced Layout and Skateboard offset slider steps are 1.0
  with -1500.0..+1500.0 bounds. Save Mod writes 25.0 (one decimal).
- Confirm Elements has a single help row above all eight toggles,
  and Skateboard has its own page with five controls; model IDs
  support direct numeric input with Ctrl+click/double-click.
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

For `WeaponToAmmoVerticalOffset`, use **83.0** as the default,
or try **87.0** with WidescreenFix `ProportionalWeaponIcon`.
This is a manual layout adjustment, also described in SleekConfig's
tooltip.

### Extended-weapon ammo positioning

CleanHUD uses the currently equipped weapon's skill when checking
whether its ammo belongs in the Stack. This matches GTA's
`CHud::DrawAmmo` logic and supports expanded-weapon loaders that
register weapon data only for **Standard** skill (including
throwables, sniper/country rifles, heavy weapons, camera, spray can
and fire extinguisher), alongside ordinary multiple-skill guns.
The same native ammo-render hook then applies the usual Stack
position without a second font hook or weapon-specific exceptions.

With fastman92's weapon type loader enabled, verify country rifle,
sniper, grenades/tear gas/Molotovs/satchels, camera, extinguisher,
spray can, minigun, flamethrower and both launchers, plus ordinary
pistols/SMGs/shotguns. Minimal mode and unmanaged ammo retain GTA's
original positions.

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

When using the jetpack, aim-button visibility follows GTA's
current weapon capability: instant-hit firing plus the
`AimWithArm` weapon-info flag. These values are inspected live
rather than maintaining a fixed list of weapon names, so mods
that adjust weapon data are accommodated. A weapon observed
in the actual firing state is also allowed to reveal its HUD
when a script implements shooting independently of weapon data.
Simply pressing Aim on an unusable weapon does not force the
weapon/ammo icons to appear. Outside jetpack use, existing
on-foot and vehicle aim behavior is unchanged.

CleanHUD's cellphone, jetpack and custom
icons use the **most recently measured native sprite bounds**, so they
follow whichever scaling mod the player uses without injecting another
scaling correction. Measurements persist across transient missing sprite
draws and are discarded on resolution changes.

An external weapon-icon scale adjustment may change the apparent
distance between the weapon icon and the ammo readout. CleanHUD
does not override the external scaling; use
`WeaponToAmmoVerticalOffset` to tune their relative positions.

Advanced Layout keeps Weapon/Ammo offsets together and jetpack X/Y
offsets in a separate `Jetpack Position` group.

### Optional skateboard riding icon

CleanHUD can show a separate, **opt-in skateboard icon while the
player rides a skateboard vehicle**, without confusing it with the
skateboard weapon held on foot. Enable `[Skateboard]
ShowWhileRiding=1` (or **Skateboard mod support** under
SleekConfig → Skateboard) and leave `ShowHiddenWeaponIcons=1`.

For the added-weapon Skate Mod Remake installation, the uploaded
`cleo/skate_remake.ini` contains `VehicleID=12500`,
`WeaponModel=12400`, and `WeaponType=70`. The corresponding
`Weapons.ide` registers model `12400` as `SKATE`.
CleanHUD therefore defaults to `VehicleModelId=12500` and
`IconModelId=12400`. Older replacement configurations may use
weapon model `326` (Cane), but that produces a Cane icon for
the added-model installation.

**Existing CleanHUD.ini values are preserved on upgrade.**
If `IconModelId=326` has already been saved, explicitly change it
to `12400` in CleanHUD.ini or SleekConfig and save. For another
skateboard mod, match its actual vehicle and weapon model IDs.
The integration does not automatically read external mod INIs.

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
`IconOffsetX`/`IconOffsetY` (SleekConfig → Skateboard)
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

## Manual Driveby Refixed: aim and BMX hop behavior

The supplied `ManualDrivebyRefixed.txt` script reads its own
`DrivebyAimButton` setting (which need not correspond to GTA's
`GetTarget`/side-look controls). On aim it starts GTA's
`task_drive_by` task. While an active
`TASK_SIMPLE_GANG_DRIVEBY` (ID 1022, which the script checks) or
`TASK_COMPLEX_DO_DRIVEBY` task exists and the selected weapon has
an ammo readout, CleanHUD displays Weapon/Ammo **without requiring
`ShowOnVehicleLook`**. The optional look-only preview still
follows the existing rules when no scripted drive-by task exists.

For BMX bunny hops, the script temporarily hides the carried
weapon using `hide_char_weapon_for_scripted_cutscene`. GTA may
switch to fists, saving the previously equipped weapon. CleanHUD
detects that saved-weapon/unarmed state (or render-weapon hidden
flag) **only in the bicycle class** and suppresses its managed
Weapon/Ammo components for the hide interval plus **250 ms**
afterwards. The temporary fist/knuckle slot and supplemental
weapon icons do not appear during this period. A later genuine
drive-by shot or aim task can reveal them again normally; if
Weapon/Ammo are disabled in `[Elements]`, their drawing remains
under vanilla/other-mod ownership.

Regression tests: start a scripted drive-by without shooting and
confirm the firearm HUD appears during Aim; release Aim and
verify it fades as expected. On BMX with a firearm available,
bunny hop without shooting and ensure neither the fist/knuckle
icon nor the selected firearm briefly flashes during/just after
the hop. Test a genuine shot right after the cooldown, normal
bike shooting, controller button 4, GInput, and the absence of
false changes on motorcycles and cars.

## Gameplay compatibility and lifetime audit

CleanHUD reads **GTA's CPad state**, not keyboard scan codes,
for drive-by shooting or look-direction input. The uploaded GInput
1.11 API describes optional `DrivebyWithAnalog` and
`HotkeyToDriveby` pad settings, but CleanHUD does not need to
link or load `GInputSA.asi`, obtain an `IGInputPad` pointer, or
register a GInput callback. Confirmed held-weapon `FIRING` state
or ammunition consumption while in a vehicle can reveal Weapon/Ammo
regardless of weapon type, model, vehicle class, or the original
`GetCarGunFired` mapping. A fire-button press alone cannot do so.
Consumable ammo edits from trainers can also count as a decrease.

`ShowOnVehicleLook` is intentionally conservative. Default
SMGs can preview immediately; other weapon types become eligible
for look-only preview once **actual fire or ammo consumption** is
observed with that weapon in the current vehicle. Switching
vehicles/loading a game clears this learned state. The optional
look preview still obeys the known vanilla vehicle-direction checks,
so custom vehicle classes may not offer speculative look previews
even though their actual shots reveal the HUD.

The source-level memory audit found no per-frame `new`, `malloc`,
RenderWare texture creation, or retained ownership of GTA TXD
textures. Supplemental icons only borrow a texture during drawing;
plugin-sdk's `CSprite2d` has no automatic destructor that frees
the borrowed texture. SafetyHook hook objects have static lifetime
and SleekConfig is unregistered on RenderWare shutdown. CleanHUD
does register plugin-sdk game/draw events for the game process
lifetime: **unloading/reloading the ASI mid-game is not a supported
or leak-tested operation**. A code review alone cannot prove the
absence of leaks; long-session heap/device-reload testing is still
required for release readiness.

### Pre-release GInput / drive-by test matrix

1. Without GInput, test vanilla Tec-9, Micro SMG and MP5 drive-by;
   weapon and ammo reveal only on confirmed shooting, not bike
   bunny hop or car honking.
2. With GInput pad, test both analog drive-by and hotkey mappings.
   Check cars, motorcycles, bicycles and Skate Mod Remake, including
   XInput disconnect/reconnect and mouse/keyboard switching.
3. Enable an expanded-weapon drive-by mod and actually shoot with
   an additional weapon (pistol, rifle or another mod-enabled type).
   Both `WEAPONSTATE_FIRING` and ammo-decrement-only scripts should
   work without a hardcoded SMG list.
4. With `ShowOnVehicleLook=1`, verify an unfamiliar weapon does
   not reveal on sideways look before it is observed firing; after
   a confirmed shot, it may preview in supported vehicle classes.
   Verify weapon switching and exiting/re-entering reset this state.
5. Play a long session with frequent reloads, resolutions, jetpack,
   phone and skateboard use, SleekConfig saves, and repeated HUD
   fading. Compare private bytes/committed memory across stationary
   baselines rather than claiming a leak from a one-time loading peak.

## Layout range, formatting and visibility cap

All 19 spacing/offset controls accept **-1500 to +1500** virtual
layout units in both SleekConfig and the INI reader. The 17 general
layout/jetpack controls remain under Advanced Layout; the two
skateboard X/Y offsets are in Skateboard. Values round to whole
virtual units and save with one decimal place. Model IDs are integer
options (step 1) and affect only the optional rider icon.

`DurationMs` is limited to 0..10000 milliseconds (10 seconds) in
both SleekConfig and INI loading. Existing longer values are clamped
on load. FadeInMs/FadeOutMs retain their 0..2000 ranges.

SleekConfig registers a **1.0** increment for all 19 float
sliders across Advanced Layout and Skateboard, avoiding float validation issues.
Direct INI values are normalized to the same whole-unit increments.
The editor displays integer values because its current formatting
follows the step size;
