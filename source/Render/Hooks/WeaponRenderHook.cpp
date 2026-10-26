#include <cmath>
#include <cstdint>

#include <safetyhook/safetyhook.hpp>

#include <extensions/Screen.h>

#include "WeaponRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookMid gWeaponHook;
    SafetyHookMid gWeaponSpriteAlphaHook;

    constexpr std::uintptr_t WeaponIconStart = 0x58D7D0;

    constexpr std::uintptr_t WeaponIconEnd = 0x58D9A0;

    static void HookedWeaponPosition(SafetyHookContext& ctx)
    {
        const auto pos = HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
            return;

        auto* alpha =
            reinterpret_cast<float*>(
                ctx.esp + 4);

        *alpha *=
            static_cast<float>(
                pos.Alpha) /
            255.0f;

        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            return;
        }

        const float screenWidth = plugin::screen::GetScreenWidth();

        const float screenHeight = plugin::screen::GetScreenHeight();

        constexpr float WeaponNativeWidth = 47.0f;

        const float weaponWidth =
            WeaponNativeWidth *
            (screenHeight / 480.0f);

        const float layoutRightX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const int layoutX =
            static_cast<int>(
                std::lround(
                    layoutRightX -
                    weaponWidth));

        const int layoutY =
            static_cast<int>(
                std::lround(
                    static_cast<float>(pos.Y) *
                    (screenHeight / 1080.0f)));

        ctx.eax =
            static_cast<std::uintptr_t>(
                layoutX);

        auto* stack =
            reinterpret_cast<
            std::uint32_t*>(
                ctx.esp);

        stack[0] =
            static_cast<std::uint32_t>(
                layoutY);
    }

    static void HookedWeaponSpriteAlpha(SafetyHookContext& ctx)
    {
        auto* stack =
            reinterpret_cast<std::uint32_t*>(
                ctx.esp);

        const std::uintptr_t returnAddress =
            stack[0];

        if (returnAddress < WeaponIconStart ||
            returnAddress >= WeaponIconEnd)
        {
            return;
        }

        const auto pos = HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
            return;

        const std::uint32_t originalAlpha =
            stack[9];

        const std::uint32_t cleanAlpha =
            pos.Alpha;

        stack[9] =
            originalAlpha *
            cleanAlpha /
            255u;
    }
}

void WeaponRenderHook::Install()
{
    gWeaponHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58F93E),
            HookedWeaponPosition);

    gWeaponSpriteAlphaHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x70D000),
            HookedWeaponSpriteAlpha);
}

void WeaponRenderHook::Remove()
{
    gWeaponSpriteAlphaHook.reset();
    gWeaponHook.reset();
}