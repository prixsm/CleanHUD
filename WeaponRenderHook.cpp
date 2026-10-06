#include <cmath>
#include <cstdint>
#include <intrin.h>

#include "../safetyhook/safetyhook.hpp"

#include <extensions/Screen.h>

#include "WeaponRenderHook.h"
#include "HUDManager.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gWeaponHook;
    SafetyHookInline gWeaponSpriteHook;

    constexpr std::uintptr_t WeaponIconStart =
        0x58D7D0;

    constexpr std::uintptr_t WeaponIconEnd =
        0x58D9A0;

    static void HookedWeaponPosition(
        SafetyHookContext& ctx)
    {
        if (HUDManager::Get().
            IsUsingVanillaLayout())
        {
            return;
        }

        const auto pos =
            HUDRenderer::Get().
            GetWeaponPosition();

        if (!pos.Valid)
            return;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        constexpr float WeaponNativeWidth =
            47.0f;

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
            reinterpret_cast<std::uint32_t*>(
                ctx.esp);

        stack[0] =
            static_cast<std::uint32_t>(
                layoutY);

        auto* alpha =
            reinterpret_cast<float*>(
                ctx.esp +
                sizeof(std::uint32_t));

        *alpha *=
            static_cast<float>(pos.Alpha) /
            255.0f;
    }

    static void __cdecl HookedWeaponSprite(
        float x,
        float y,
        float z,
        float halfWidth,
        float halfHeight,
        unsigned char red,
        unsigned char green,
        unsigned char blue,
        short alpha,
        float rhw,
        unsigned char intensity,
        unsigned char udir,
        unsigned char vdir)
    {
        const auto returnAddress =
            reinterpret_cast<std::uintptr_t>(
                _ReturnAddress());

        const bool calledFromWeaponIcon =
            returnAddress >= WeaponIconStart &&
            returnAddress < WeaponIconEnd;

        if (calledFromWeaponIcon &&
            !HUDManager::Get().
            IsUsingVanillaLayout())
        {
            const auto pos =
                HUDRenderer::Get().
                GetWeaponPosition();

            if (pos.Valid)
            {
                alpha =
                    static_cast<short>(
                        static_cast<int>(alpha) *
                        static_cast<int>(
                            pos.Alpha) /
                        255);
            }
        }

        gWeaponSpriteHook.ccall(
            x,
            y,
            z,
            halfWidth,
            halfHeight,
            red,
            green,
            blue,
            alpha,
            rhw,
            intensity,
            udir,
            vdir);
    }
}

void WeaponRenderHook::Install()
{
    gWeaponHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58F93E),
            HookedWeaponPosition);

    gWeaponSpriteHook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(
                0x70D000),
            HookedWeaponSprite);
}

void WeaponRenderHook::Remove()
{
    gWeaponSpriteHook.reset();
    gWeaponHook.reset();
}