#include <algorithm>
#include <cmath>

#include <safetyhook/safetyhook.hpp>
#include <extensions/Screen.h>

#include "AmmoRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookInline gAmmoHook;

    static void __cdecl HookedRenderAmmo(int player, int x, int y, float alpha)
    {
        const auto pos = HUDRenderer::Get().GetAmmoPosition();

        if (!pos.Valid)
        {
            gAmmoHook.ccall(player, x, y, alpha);

            return;
        }

        const float cleanAlpha =
            static_cast<float>(
                pos.Alpha) /
            255.0f;

        alpha =
            std::clamp(
                alpha * cleanAlpha,
                0.0f,
                255.0f);

        if (!HUDManager::Get().IsUsingVanillaLayout())
        {
            const float screenWidth = plugin::screen::GetScreenWidth();

            const float screenHeight = plugin::screen::GetScreenHeight();

            constexpr float WeaponNativeWidth = 47.0f;

            const float weaponWidth =
                WeaponNativeWidth *
                (screenHeight / 480.0f);

            const float layoutRightX =
                static_cast<float>(pos.X) *
                (screenWidth / 1920.0f);

            constexpr float AmmoNativeCenterOffset = 3.4f;

            const float ammoCenterOffset =
                AmmoNativeCenterOffset *
                (screenHeight / 480.0f);

            const float ammoCenterX =
                layoutRightX -
                (weaponWidth * 0.5f) -
                ammoCenterOffset;

            const float layoutY =
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f);

            x = static_cast<int>(std::lround(ammoCenterX));

            y = static_cast<int>(std::lround(layoutY));
        }

        gAmmoHook.ccall(player, x, y, alpha);
    }
}

void AmmoRenderHook::Install()
{
    gAmmoHook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(0x5893B0),
            reinterpret_cast<void*>(HookedRenderAmmo)
        );
}

void AmmoRenderHook::Remove()
{
    gAmmoHook.reset();
}