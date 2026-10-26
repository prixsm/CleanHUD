#include <cstdint>

#include "../safetyhook/safetyhook.hpp"

#include <CFont.h>
#include <extensions/Screen.h>

#include "MoneyRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../HUDMetrics.h"
#include "../HUDRenderer.h"
#include "../../Core/HUDSettings.h"

namespace
{
    SafetyHookMid gMoneyHook;
    SafetyHookMid gMoneyRestoreHook;

    bool gMoneyAlphaModified = false;

    std::uint8_t gOriginalMoneyColourAlpha = 255;
    std::uint8_t gOriginalMoneyDropAlpha = 255;

    static std::uint8_t ScaleAlpha(
        std::uint8_t originalAlpha,
        std::uint8_t cleanHudAlpha)
    {
        return static_cast<std::uint8_t>(
            static_cast<unsigned int>(
                originalAlpha) *
            static_cast<unsigned int>(
                cleanHudAlpha) /
            255u);
    }

    static void HookedMoneyDraw(SafetyHookContext& ctx)
    {
        // Disabled elements must retain their normal game draw call.
        // The original no-position case otherwise skips Money.
        if (!HUDSettings::Get().Money)
            return;

        auto* stack = reinterpret_cast<float*>(ctx.esp);

        const bool vanillaLayout = HUDManager::Get().IsUsingVanillaLayout();

        const auto pos = HUDRenderer::Get().GetMoneyPosition();

        if (!pos.Valid)
        {
            if (!vanillaLayout)
            {
                ctx.eip = 0x58F60C;
            }

            return;
        }

        if (!vanillaLayout)
        {
            const float screenWidth = plugin::screen::GetScreenWidth();

            const float screenHeight = plugin::screen::GetScreenHeight();

            const float layoutX =
                static_cast<float>(pos.X) *
                (screenWidth / 1920.0f);

            const float layoutY =
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f);

            stack[0] = layoutX + HUDMetrics::MoneyRightOffset;

            stack[1] = layoutY;
        }

        gOriginalMoneyColourAlpha = CFont::m_Color->a;

        gOriginalMoneyDropAlpha = CFont::m_FontDropColor->a;

        CFont::m_Color->a = ScaleAlpha(gOriginalMoneyColourAlpha, pos.Alpha);

        CFont::m_FontDropColor->a = ScaleAlpha(gOriginalMoneyDropAlpha, pos.Alpha);

        gMoneyAlphaModified = true;
    }

    static void RestoreMoneyAlpha(SafetyHookContext& ctx)
    {
        if (!gMoneyAlphaModified)
            return;

        CFont::m_Color->a = gOriginalMoneyColourAlpha;

        CFont::m_FontDropColor->a = gOriginalMoneyDropAlpha;

        gMoneyAlphaModified = false;
    }
}

void MoneyRenderHook::Install()
{
    gMoneyHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58F607),
            HookedMoneyDraw);

    gMoneyRestoreHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58F60C),
            RestoreMoneyAlpha);
}

void MoneyRenderHook::Remove()
{
    gMoneyRestoreHook.reset();
    gMoneyHook.reset();

    if (gMoneyAlphaModified)
    {
        CFont::m_Color->a = gOriginalMoneyColourAlpha;

        CFont::m_FontDropColor->a = gOriginalMoneyDropAlpha;

        gMoneyAlphaModified = false;
    }
}