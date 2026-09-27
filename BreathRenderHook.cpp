#include <cmath>
#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "BreathRenderHook.h"
#include "HUDManager.h"
#include "HUDRenderer.h"
#include "HUDMetrics.h"

namespace
{
    SafetyHookInline gBreathHook;

    static void __cdecl HookedRenderBreathBar(
        int player,
        int x,
        int y)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            gBreathHook.ccall(player, x, y);
            return;
        }

        const auto pos =
            HUDRenderer::Get().GetBreathPosition();

        if (!pos.Valid)
        {
            gBreathHook.ccall(player, x, y);
            return;
        }

        constexpr float BreathNativeX = 559.0f;
        constexpr float BreathNativeY = 119.0f;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float breathX =
            HUDMetrics::ScaleNativeRightX(
                BreathNativeX);

        const float breathY =
            BreathNativeY *
            (screenHeight / 1080.0f);

        x = static_cast<int>(
            std::lround(breathX));

        y = static_cast<int>(
            std::lround(breathY));

        gBreathHook.ccall(player, x, y);
    }
}

void BreathRenderHook::Install()
{
    gBreathHook = safetyhook::create_inline(
        reinterpret_cast<void*>(0x589190),
        reinterpret_cast<void*>(HookedRenderBreathBar)
    );
}

void BreathRenderHook::Remove()
{
    gBreathHook.reset();
}