# CleanHUD live settings editor

This optional in-game editor is included in the ASI build and requires no
separate Dear ImGui installation.

- Press **F10** in GTA San Andreas to open/close the editor.
- Drag or Ctrl+click the numeric controls for precise values.
- Changes take effect in the running game on the next HUD update.
- **Save to CleanHUD.ini** persists current values beside CleanHUD.SA.asi.
- **Reload from disk** discards unsaved live edits and reloads the on-disk INI.
- Player controls are temporarily disabled while the editor is open; the
  previous control-disable state is restored when it closes.
- The **[Spacing]** values affect Stack mode only. Minimal mode keeps original
  GTA positions.
- Opening the editor does not force all HUD elements to become visible.
  Trigger the corresponding HUD elements in-game to observe spacing effects.
  TAB still invokes the existing vanilla-position preview.

The editor uses Plugin-SDK drawing / D3D lost / reset / RenderWare shutdown
events, without adding game-memory hooks. It initializes Dear ImGui lazily on
first F10 use. If the Direct3D device or focus window is unavailable, the
editor simply does not open.

Third-party dependency: [Dear ImGui](https://github.com/ocornut/imgui)
version **1.91.9b**, vendored in third_party/imgui. License:
third_party/imgui/LICENSE.txt (MIT).

## Game verification checklist

Test on GTA SA PC 1.0 US in your actual graphics environment:

1. Main menu, New Game, and Load Game should remain visible with no HUD flash.
2. F10 opens a movable panel in-game; clicks, typing, Ctrl+click, and scrolling
   operate without shooting or moving the player.
3. Switching Stack / Minimal updates on the next frame.
4. With an active HUD event, modifying any Clock or status gap shifts Stack
   elements immediately; in Minimal their native positions remain unchanged.
5. Fade durations, display timer, Clock ownership, and element toggles update
   while the game is running.
6. Save, exit, restart and verify persisted INI values; Reload restores values
   stored on disk.
7. Close with F10, the window's X, or Close: player controls resume.
8. Alt+Tab, change game resolution and fullscreen/windowed mode; verify D3D
   lost/reset does not crash or leave the cursor/player controls stuck.
9. Check compatibility with your Widescreen Fix, SilentPatch, and other ASIs
   that may hook Direct3D or the game window.
