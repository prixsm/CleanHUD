#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include <cmath>
#include <cstdint>

#include "AmmoRenderHook.h"
#include "HUDManager.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gAmmoPositionHook;

    static void HookedAmmoDraw(SafetyHookContext& ctx)
    {
        if (HUDManager::Get().IsUsingVanillaLayout())
            return;

        const auto pos =
            HUDRenderer::Get().GetAmmoPosition();

        if (!pos.Valid)
            return;

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const int layoutX =
            static_cast<int>(std::lround(
                static_cast<float>(pos.X) *
                (screenWidth / 1920.0f)));

        const int layoutY =
            static_cast<int>(std::lround(
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f)));

        auto* stack =
            reinterpret_cast<std::uint32_t*>(ctx.esp);

        stack[1] = static_cast<std::uint32_t>(layoutX);
        stack[2] = static_cast<std::uint32_t>(layoutY);
    }
}

void AmmoRenderHook::Install()
{
    gAmmoPositionHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58FA25),
            HookedAmmoDraw);
}

void AmmoRenderHook::Remove()
{
    gAmmoPositionHook.reset();
}