#include <cmath>
#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "ArmourRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookInline gArmourHook;

    static void __cdecl HookedRenderArmourBar(
        int player,
        int x,
        int y)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            gArmourHook.ccall(player, x, y);
            return;
        }

        const auto pos =
            HUDRenderer::Get().GetArmourPosition();

        if (!pos.Valid)
        {
            gArmourHook.ccall(player, x, y);
            return;
        }

        constexpr float ArmourNativeX = 559.0f;
        constexpr float ArmourNativeY = 92.0f;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float armourX =
            HUDMetrics::ScaleNativeRightX(
                ArmourNativeX);

        x = static_cast<int>(
            std::lround(armourX));

        y = static_cast<int>(std::lround(
            ArmourNativeY *
            (screenHeight / 1080.0f)));

        gArmourHook.ccall(player, x, y);
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