#pragma once

namespace SleekConfigIntegration
{
    // Connect to SleekConfig if it is installed, without requiring it.
    void TryAttach();

    // Disconnect when CleanHUD shuts down.
    void Detach();
}
