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
        const auto pos = HUDRenderer::Get().GetHealthPosition();

        if (!pos.Valid)
        {
            gHealthHook.ccall(player, x, y);

            return;
        }

        if (!HUDManager::Get().IsUsingVanillaLayout())
        {
            constexpr float HealthNativeX = 519.0f;

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
        }

        auto& colour = HudColour.m_aColours[HUD_COLOUR_RED];

        const unsigned char originalAlpha = colour.alpha;

        colour.alpha =
            static_cast<unsigned char>(
                static_cast<unsigned int>(
                    originalAlpha) *
                static_cast<unsigned int>(
                    pos.Alpha) /
                255u);

        gHealthHook.ccall(player, x, y);

        colour.alpha = originalAlpha;
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