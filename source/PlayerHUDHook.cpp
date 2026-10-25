#include <../safetyhook/safetyhook.hpp>
#include "PlayerHUDHook.h"
#include "HUDManager.h"
#include "HUDSettings.h"

namespace
{
    SafetyHookInline gPlayerHUDHook;

    void __cdecl HookedDrawPlayerInfo()
    {
        // Vanilla Health must draw during game startup.
        // Only gate the HUD when CleanHUD controls Health.
        if (HUDSettings::Get().Health &&
            !HUDManager::Get().IsPlayerHUDInitialized())
        {
            return;
        }

        gPlayerHUDHook.ccall();
    }
}

void PlayerHUDHook::Install()
{
    gPlayerHUDHook = safetyhook::create_inline(
        reinterpret_cast<void*>(0x58EAF0),
        reinterpret_cast<void*>(HookedDrawPlayerInfo)
    );
}