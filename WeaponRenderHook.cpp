#include <cmath>
#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "WeaponRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookInline gWeaponHook;

    static void __cdecl HookedRenderWeapon(
        int player,
        int x,
        int y,
        float alpha)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            gWeaponHook.ccall(player, x, y, alpha);
            return;
        }

        const auto pos =
            HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
        {
            gWeaponHook.ccall(player, x, y, alpha);
            return;
        }

        constexpr float WeaponNativeX = 525.2f;
        constexpr float WeaponNativeY = 38.0f;

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float weaponX =
            HUDMetrics::ScaleNativeRightX(
                WeaponNativeX);

        const float weaponY =
            WeaponNativeY *
            (screenHeight / 1080.0f);

        x = static_cast<int>(
            std::lround(weaponX));

        y = static_cast<int>(
            std::lround(weaponY));

        gWeaponHook.ccall(player, x, y, alpha);
    }
}

void WeaponRenderHook::Install()
{
    gWeaponHook = safetyhook::create_inline(
        reinterpret_cast<void*>(0x58D7D0),
        reinterpret_cast<void*>(HookedRenderWeapon)
    );
}

void WeaponRenderHook::Remove()
{
    gWeaponHook.reset();
}