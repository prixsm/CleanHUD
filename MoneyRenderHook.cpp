#include "../safetyhook/safetyhook.hpp"
#include <extensions/Screen.h>
#include "MoneyRenderHook.h"
#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gMoneyHook;

    static void HookedMoneyDraw(SafetyHookContext& ctx)
    {
        auto* stack =
            reinterpret_cast<float*>(ctx.esp);

        if (HUDManager::Get().IsUsingVanillaLayout())
            return;

        const auto pos =
            HUDRenderer::Get().GetMoneyPosition();

        if (!pos.Valid)
        {
            ctx.eip = 0x58F60C;
            return;
        }

        const float screenWidth =
            plugin::screen::GetScreenWidth();

        const float screenHeight =
            plugin::screen::GetScreenHeight();

        const float layoutX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const float layoutY =
            static_cast<float>(pos.Y) *
            (screenHeight / 1080.0f);

        stack[0] =
            layoutX + HUDMetrics::MoneyRightOffset;

        stack[1] =
            layoutY;
    }
}

void MoneyRenderHook::Install()
{
    gMoneyHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(0x58F607),
            HookedMoneyDraw);
}

void MoneyRenderHook::Remove()
{
    gMoneyHook.reset();
}