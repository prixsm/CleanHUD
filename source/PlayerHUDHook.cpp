#include <../safetyhook/safetyhook.hpp>
#include "PlayerHUDHook.h"
#include "Core/HUDManager.h"
#include "Core/HUDSettings.h"

namespace
{
    SafetyHookInline& PlayerHUDHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    void __cdecl HookedDrawPlayerInfo()
    {
        // The original game drawing calls are restored while
        // a save/new game loads. Do not let that fallback flash
        // the full vanilla HUD before CleanHUD's first layout.
        //
        // This hook is for the player HUD, not CFont menu text.
        // When Health=0, preserve the requested vanilla Health
        // startup behavior rather than gating the whole HUD.
        if (HUDSettings::Get().Health &&
            !HUDManager::Get().IsPlayerHUDInitialized())
        {
            return;
        }

        PlayerHUDHookStorage().ccall();
    }
}

void PlayerHUDHook::Install()
{
    SafetyHookInline& hook = PlayerHUDHookStorage();
    hook = safetyhook::create_inline(
        reinterpret_cast<void*>(0x58EAF0),
        reinterpret_cast<void*>(HookedDrawPlayerInfo)
    );
}