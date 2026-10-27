#pragma once

namespace HUDConfigEditor
{
    // Initialized through Plugin-SDK render events (not a global hook).
    void Install();

    // Called during gameProcessEvent to prevent gameplay input while
    // the interactive editor owns keyboard/mouse focus.
    void UpdateInput();
}
