#include <safetyhook/safetyhook.hpp>
#include <CWorld.h>
#include <extensions/Screen.h>

#include "../../Core/HUDManager.h"
#include "../HUDRenderer.h"
#include "WantedRenderHook.h"

namespace
{
    SafetyHookMid& WantedYHookStorage()
    {
        static SafetyHookMid hook;
        return hook;
    }
    SafetyHookMid& WantedPositionHookStorage()
    {
        static SafetyHookMid hook;
        return hook;
    }

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

        // GTA's wanted-star font uses a 560-unit vertical basis,
        // whereas the old hook converted every stacked position using
        // 480. That compresses Wanted's vertical displacement by 6/7:
        // Money/Clock and the elements below move farther than the stars.
        //
        // Preserve the original top-of-stack placement by keeping the
        // original 480 conversion at the layout anchor. Scale only the
        // distance below the anchor using the wanted font's 560 basis.
        // This makes wanted-to-neighbor gaps independent of how many
        // elements are stacked above the stars.
        const float anchorY =
            HUDRenderer::Get().GetAnchor().Y;

        const float layoutWantedY =
            static_cast<float>(pos.Y);

        float nativeWantedY =
            anchorY * (480.0f / 1080.0f) +
            (layoutWantedY - anchorY) * (560.0f / 1080.0f);

        // Retain the game's existing max-health adjustment for
        // wanted stars; unrelated to stack-relative displacement.
        constexpr float WantedHealthYOffset = 12.0f;

        const auto& playerInfo =
            CWorld::Players[CWorld::PlayerInFocus];

        if (playerInfo.m_nMaxHealth < 101)
            nativeWantedY += WantedHealthYOffset;

        *wantedY = nativeWantedY;
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

    SafetyHookMid& hook = WantedYHookStorage();
    hook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58D9A0),
            HookedWantedY);

    SafetyHookMid& secondHook = WantedPositionHookStorage();
    secondHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58DFD3),
            HookedWantedDraw);
}

void WantedRenderHook::Remove()
{
    *reinterpret_cast<float*>(
        WantedYAddress) =
        gOriginalWantedY;

    WantedYHookStorage().reset();
    WantedPositionHookStorage().reset();
}