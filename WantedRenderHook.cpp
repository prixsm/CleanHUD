#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "WantedRenderHook.h"
#include "HUDManager.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gWantedPositionHook;

    static void HookedWantedDraw(SafetyHookContext& ctx)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
            return;

        const auto pos =
            HUDRenderer::Get().GetWantedPosition();

        if (!pos.Valid)
            return;

        float* x =
            reinterpret_cast<float*>(ctx.esp);

        // Native CHud::DrawWanted() right-edge position:
        // // 1864.07141 / 1920 = 1553.39282 / 1600 = 1242.71423 / 1280.
        constexpr float NativeWantedRightScale = 0.9708705f;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float nativeWantedRight =
            screenWidth * NativeWantedRightScale;

        const float layoutX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const float delta =
            layoutX - nativeWantedRight;

        *x += delta;
    }
}

void WantedRenderHook::Install()
{
    gWantedPositionHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58DFD3),
            HookedWantedDraw);
}

void WantedRenderHook::Remove()
{
    gWantedPositionHook.reset();
}