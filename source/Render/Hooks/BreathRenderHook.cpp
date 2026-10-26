#include <cmath>

#include <safetyhook/safetyhook.hpp>

#include <CHudColours.h>
#include <extensions/Screen.h>

#include "BreathRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../HUDMetrics.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookInline& BreathHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    static void __cdecl HookedRenderBreathBar(int player, int x, int y)
    {
        const auto pos = HUDRenderer::Get().GetBreathPosition();

        if (!pos.Valid)
        {
            BreathHookStorage().ccall(player, x, y);

            return;
        }

        if (!HUDManager::Get().IsUsingVanillaLayout())
        {
            constexpr float BreathNativeX = 559.0f;

            const float screenHeight =
                plugin::screen::GetScreenHeight();

            const float breathX =
                HUDMetrics::ScaleNativeRightX(
                    BreathNativeX +
                    HUDMetrics::
                    ArmourBreathNativeXOffset);

            const float layoutY =
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f);

            x = static_cast<int>(
                std::lround(breathX));

            y = static_cast<int>(
                std::lround(layoutY));
        }

        auto& colour = HudColour.m_aColours[HUD_COLOUR_BLUELIGHT];

        const unsigned char originalAlpha = colour.alpha;

        colour.alpha =
            static_cast<unsigned char>(
                static_cast<unsigned int>(
                    originalAlpha) *
                static_cast<unsigned int>(
                    pos.Alpha) /
                255u);

        BreathHookStorage().ccall(player, x, y);

        colour.alpha =
            originalAlpha;
    }
}

void BreathRenderHook::Install()
{
    SafetyHookInline& hook = BreathHookStorage();
    hook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(0x589190),
            HookedRenderBreathBar);
}

void BreathRenderHook::Remove()
{
    BreathHookStorage().reset();
}