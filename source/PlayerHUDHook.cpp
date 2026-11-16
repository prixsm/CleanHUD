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
        // Avoid a HUD flash while loading a game.
        // If CleanHUD does not manage Health, let GTA draw it normally.
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