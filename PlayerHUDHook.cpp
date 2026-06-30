#include <../safetyhook/safetyhook.hpp>
#include "PlayerHUDHook.h"
#include "HUDManager.h"

namespace
{
    SafetyHookInline gPlayerHUDHook;

    void __cdecl HookedDrawPlayerInfo()
    {
        if (!HUDManager::Get().IsPlayerHUDInitialized())
            return;

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