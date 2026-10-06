#include <cmath>

#include "../safetyhook/safetyhook.hpp"

#include <CHudColours.h>
#include <extensions/Screen.h>

#include "HealthRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookInline gHealthHook;

    static void __cdecl HookedRenderHealthBar(int player, int x, int y)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            gHealthHook.ccall(player, x, y);

            return;
        }

        const auto pos =
            HUDRenderer::Get().GetHealthPosition();

        if (!pos.Valid)
        {
            gHealthHook.ccall(player, x, y);

            return;
        }

        constexpr float HealthNativeX =
            519.0f;

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float healthX =
            HUDMetrics::ScaleNativeRightX(
                HealthNativeX);

        const float layoutY =
            static_cast<float>(pos.Y) *
            (screenHeight / 1080.0f);

        x = static_cast<int>(
            std::lround(healthX));

        y = static_cast<int>(
            std::lround(layoutY));

        auto& colour =
            HudColour.m_aColours[HUD_COLOUR_RED];

        const unsigned char originalAlpha =
            colour.alpha;

        colour.alpha =
            pos.Alpha;

        gHealthHook.ccall(player, x, y);

        colour.alpha =
            originalAlpha;
    }
}

void HealthRenderHook::Install()
{
    gHealthHook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(0x589270),
            HookedRenderHealthBar);
}

void HealthRenderHook::Remove()
{
    gHealthHook.reset();
}