#include <cmath>
#include <cstdint>

#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>

#include "WeaponRenderHook.h"
#include "HUDManager.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gWeaponHook;

    static void HookedWeaponPosition(
        SafetyHookContext& ctx)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
            return;

        const auto pos =
            HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
            return;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        constexpr float WeaponNativeWidth = 47.0f;

        const float weaponWidth =
            WeaponNativeWidth *
            (screenHeight / 480.0f);

        const float layoutRightX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const int layoutX =
            static_cast<int>(std::lround(
                layoutRightX - weaponWidth));

        const int layoutY =
            static_cast<int>(std::lround(
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f)));

        ctx.eax =
            static_cast<std::uintptr_t>(layoutX);

        auto* stack =
            reinterpret_cast<std::uint32_t*>(ctx.esp);

        stack[0] =
            static_cast<std::uint32_t>(layoutY);
    }
}

void WeaponRenderHook::Install()
{
    gWeaponHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58F93E),
            HookedWeaponPosition);
}

void WeaponRenderHook::Remove()
{
    gWeaponHook.reset();
}