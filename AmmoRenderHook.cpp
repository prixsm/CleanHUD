#include <cmath>
#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "AmmoRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookInline gAmmoHook;

    static void __cdecl HookedRenderAmmo(
        int player,
        int x,
        int y)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            gAmmoHook.ccall(player, x, y);
            return;
        }

        const auto pos =
            HUDRenderer::Get().GetArmourPosition();

        if (!pos.Valid)
        {
            gAmmoHook.ccall(player, x, y);
            return;
        }

        constexpr float AmmoNativeX = 544.2f;
        constexpr float AmmoNativeY = 121.0f;

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float ammoX =
            HUDMetrics::ScaleNativeRightX(
                AmmoNativeX);

        const float ammoY =
            AmmoNativeY *
            (screenHeight / 1080.0f);

        x = static_cast<int>(
            std::lround(ammoX));

        y = static_cast<int>(
            std::lround(ammoY));

        gAmmoHook.ccall(player, x, y);
    }
}

void AmmoRenderHook::Install()
{
    gAmmoHook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(0x5890A0),
            HookedRenderAmmo);
}

void AmmoRenderHook::Remove()
{
    gAmmoHook.reset();
}