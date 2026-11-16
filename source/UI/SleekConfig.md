# CleanHUD settings with SleekConfig

[SleekConfig](https://github.com/pedrolecio/SleekConfig) is an optional in-game settings menu for CleanHUD. You can also change everything in `CleanHUD.ini` without it.

## Getting started

1. Install `CleanHUD.asi` and `CleanHUD.ini` together using your GTA San Andreas ASI loader. CleanHUD supports the **PC 1.0 US** version of the game.
2. For in-game settings, install **SleekConfig 1.1.0 or newer** separately.
3. In the game, press **F10**, open **Visuals**, then choose **CleanHUD**.

SleekConfig is optional. Without it, CleanHUD uses your INI settings normally.

## Find your settings

| Page | What you can change |
| --- | --- |
| **General** | Stack or Minimal mode, clock, animation, aiming and hidden icons |
| **Elements** | Which HUD items CleanHUD controls |
| **Visibility** | How quickly items appear, stay visible and fade |
| **Skateboard** | Skateboard mod support, model IDs and icon position |
| **Advanced Layout** | Spacing and positions in Stack mode, based on GTA's original HUD |
| **HUD Preview** | Keyboard or gamepad button to show GTA's original HUD positions |

On the **Elements** page, **On** means CleanHUD controls an item. **Off** lets GTA or another HUD mod handle it.

Most changes appear right away. Use **Save Mod** to keep them after restarting the game. SleekConfig also supports named presets.

## Stack, Minimal and appearance

- **Stack:** HUD items that need attention appear together along the right side of the screen.
- **Minimal:** The same fade effects, but at GTA's original HUD positions.
- **Always show Clock:** Keeps the clock visible. If you turn the Clock element off, GTA draws it instead.
- **Slide stack on entry:** Gently moves items into their places in Stack mode.
- **Visibility:** Fade in, Fade out and Duration control the timing. A duration of 3000 ms means 3 seconds.
- **Show on foot aim:** Shows the weapon/ammo when aiming on foot.
- **Show on throwable aim:** Useful for other mods that let you aim grenades or similar weapons.
- **Show on vehicle look:** Lets looking sideways in a vehicle reveal weapon/ammo. Actual drive-by shooting can reveal them even with this off.

Turning off an element under **Elements** gives control of that element back to GTA or another HUD mod.

## Move HUD items

Open **Advanced Layout** to adjust gaps and positions. The default
values are based on GTA's original HUD positions, so the HUD stays
familiar. **Spacing and Weapon/Ammo positioning work only in Stack
mode**. Jetpack icon offsets also work in Minimal mode. Increase a
spacing value to add more room between items. Moving a slider briefly
shows the affected HUD items.

**Weapon & Ammo Position** moves the weapon icon and ammo together. The default **Weapon to Ammo vertical offset** is **83**. If you use WidescreenFix with `ProportionalWeaponIcon=1`, try **87**.

The jetpack has its own X/Y position controls. Skateboard X/Y controls are on the **Skateboard** page.

- Positive **X** moves an icon right.
- Positive **Y** moves it down.
- **Shift** helps with fine slider adjustments.
- **Ctrl+click** or **double-click** a number to type an exact value. This is particularly helpful for skateboard model IDs.

Minimal mode keeps GTA's original positions, so Stack spacing changes won't move its HUD items.

## Cellphone, jetpack and custom weapon icons

Enable **Show hidden/custom weapon icons** under General (enabled by default). CleanHUD can show the cellphone, jetpack and supported custom weapon icons when their icon textures are available.

The jetpack appears in the weapon slot when fists or brass knuckles are selected. With other weapons, it appears beside the weapon icon. You can adjust its position under Advanced Layout.

### Skate Mod Remake

The **Skateboard** page has optional support for a skateboard that acts as a vehicle. Turn on **Skateboard mod support** and check the model IDs.

For the added-model setup of Skate Mod Remake:
- **Vehicle model ID:** `12500`
- **Icon model ID:** `12400`

Other installations may use different IDs; use the IDs from your skateboard mod's settings. The icon needs an available `ICON` texture. A skateboard carried as a weapon uses the ordinary custom-weapon icon system.

If an older INI has `IconModelId=326`, change it to your skateboard's actual weapon/icon model ID; `326` is the Cane model.

## Temporarily show GTA's original HUD

Open **HUD Preview**, the last settings page.

- **Keyboard virtual key:** **Tab** by default.
- **Gamepad / mapped GTA button:** **Off** by default.

Hold the selected key/button to see GTA's usual HUD positions. Release it to return to CleanHUD. Both menus show the code and the key/button name; `CleanHUD.ini` also includes the full choice lists.

**Gamepad choices are GTA actions, not controller-only buttons.** A keyboard key mapped to the same action (for example Ctrl and LB/L1) can also trigger a preview. Leave this setting **Off** if that happens.

For uncommon keyboard keys, enter a number from **0 to 255** under `[HUDPreview]` in the INI. `0` disables the binding.

## Save your changes

- **Save Mod:** Saves your current CleanHUD settings.
- **Presets:** Save or load a named layout.
- **Defaults:** Try CleanHUD's default settings. Save if you want to keep them.
- **Restore Saved:** Load the previously saved settings.

**Edited the INI but nothing changed?** SleekConfig's saved file, `SleekConfig/prixsm.cleanhud.dat`, may override the INI. Make the change in SleekConfig and choose **Save Mod**, or close the game and remove that DAT file if you prefer to use only the INI.

When changing the INI manually, close the game first and restart it afterwards.

## Quick fixes

| Problem | What to try |
| --- | --- |
| F10 doesn't open CleanHUD's settings | Install SleekConfig 1.1.0 or newer; CleanHUD works without it |
| HUD settings change back on restart | Choose **Save Mod**; check for an older SleekConfig DAT file |

| Custom or skateboard icon is missing | Enable hidden icons; check the model IDs and available icon textures |
| Weapon and ammo look too far apart | Adjust **Weapon to Ammo vertical offset** (83 normally; try 87 with proportional weapon icons) |
| Ctrl unexpectedly shows GTA's HUD | Set the mapped gamepad preview button to **0 - Off** |
