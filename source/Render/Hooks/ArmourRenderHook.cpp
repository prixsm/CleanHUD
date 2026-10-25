#include <cmath>

#include "../safetyhook/safetyhook.hpp"

#include <CHudColours.h>
#include <extensions/Screen.h>

#include "ArmourRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookInline gArmourHook;

    static void __cdecl HookedRenderArmourBar(int player, int x, int y)
    {
        const auto pos = HUDRenderer::Get().GetArmourPosition();

        if (!pos.Valid)
        {
            gArmourHook.ccall(player, x, y);

            return;
        }

        if (!HUDManager::Get().IsUsingVanillaLayout())
        {
            constexpr float ArmourNativeX = 559.0f;

            const float screenHeight =
                plugin::screen::GetScreenHeight();

            const float armourX =
                HUDMetrics::ScaleNativeRightX(
                    ArmourNativeX +
                    HUDMetrics::
                    ArmourBreathNativeXOffset);

            const float layoutY =
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f);

            x = static_cast<int>(
                std::lround(armourX));

            y = static_cast<int>(
                std::lround(layoutY));
        }

        auto& colour = HudColour.m_aColours[HUD_COLOUR_WHITE];

        const unsigned char originalAlpha = colour.alpha;

        colour.alpha =
            static_cast<unsigned char>(
                static_cast<unsigned int>(
                    originalAlpha) *
                static_cast<unsigned int>(
                    pos.Alpha) /
                255u);

        gArmourHook.ccall(player, x, y);

        colour.alpha = originalAlpha;
    }
}

void ArmourRenderHook::Install()
{
    gArmourHook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(0x5890A0),
            HookedRenderArmourBar);
}

void ArmourRenderHook::Remove()
{
    gArmourHook.reset();
}