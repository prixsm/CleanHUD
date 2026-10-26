#include "../safetyhook/safetyhook.hpp"
#include <CWorld.h>
#include <extensions/Screen.h>

#include "../../Core/HUDManager.h"
#include "../HUDRenderer.h"
#include "WantedRenderHook.h"

namespace
{
    SafetyHookMid gWantedYHook;
    SafetyHookMid gWantedPositionHook;

    constexpr uintptr_t WantedYAddress = 0x866C5C;

    float gOriginalWantedY = 114.0f;

    static void HookedWantedY(
        SafetyHookContext& ctx)
    {
        auto* wantedY =
            reinterpret_cast<float*>(
                WantedYAddress);

        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            *wantedY = gOriginalWantedY;
            return;
        }

        const auto pos =
            HUDRenderer::Get().GetWantedPosition();

        if (!pos.Valid)
        {
            *wantedY = gOriginalWantedY;
            return;
        }

        constexpr float WantedHealthYOffset = 12.0f;

        float nativeWantedY =
            static_cast<float>(pos.Y) *
            (480.0f / 1080.0f);

        const auto& playerInfo =
            CWorld::Players[CWorld::PlayerInFocus];

        if (playerInfo.m_nMaxHealth < 101)
        {
            nativeWantedY +=
                WantedHealthYOffset;
        }

        *wantedY =
            nativeWantedY;
    }

    static void HookedWantedDraw(
        SafetyHookContext& ctx)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
            return;

        const auto pos =
            HUDRenderer::Get().GetWantedPosition();

        if (!pos.Valid)
            return;

        float* x =
            reinterpret_cast<float*>(ctx.esp);

        constexpr float NativeWantedRightScale =
            0.9708705f;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float nativeWantedRight =
            screenWidth *
            NativeWantedRightScale;

        const float layoutX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const float delta =
            layoutX -
            nativeWantedRight;

        *x += delta;
    }
}

void WantedRenderHook::Install()
{
    gOriginalWantedY =
        *reinterpret_cast<float*>(
            WantedYAddress);

    gWantedYHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58D9A0),
            HookedWantedY);

    gWantedPositionHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58DFD3),
            HookedWantedDraw);
}

void WantedRenderHook::Remove()
{
    *reinterpret_cast<float*>(
        WantedYAddress) =
        gOriginalWantedY;

    gWantedYHook.reset();
    gWantedPositionHook.reset();
}