#include <../safetyhook/safetyhook.hpp>
#include "PlayerHUDHook.h"
#include "Core/HUDManager.h"

namespace
{
    SafetyHookInline& PlayerHUDHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    void __cdecl HookedDrawPlayerInfo()
    {
        // Never swallow the entire engine HUD draw if a
        // save has not initialized, the game is loading,
        // or CleanHUD's per-frame update has not yet run.
        // The manager supplies vanilla rendering until ready.
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