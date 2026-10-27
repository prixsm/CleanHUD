#pragma once

namespace SleekConfigIntegration
{
    // No hard dependency: does nothing until SleekConfig.asi is loaded.
    // Call from the existing game-process event on the main thread.
    void TryAttach();

    // Unregister before CleanHUD's process lifetime ends.
    void Detach();
}
