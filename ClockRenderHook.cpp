#include <cstdint>

#include "../safetyhook/safetyhook.hpp"

#include <CFont.h>

#include "ClockRenderHook.h"
#include "HUDRenderer.h"

namespace
{
    SafetyHookMid gClockAlphaHook;
    SafetyHookMid gClockRestoreHook;

    bool gClockAlphaModified = false;

    std::uint8_t gOriginalColourAlpha = 255;
    std::uint8_t gOriginalDropAlpha = 255;

    static std::uint8_t ScaleAlpha(
        std::uint8_t original,
        std::uint8_t fade)
    {
        return static_cast<std::uint8_t>(
            static_cast<unsigned int>(
                original) *
            static_cast<unsigned int>(
                fade) /
            255u);
    }

    static void ApplyClockAlpha(SafetyHookContext&)
    {
        const auto pos = HUDRenderer::Get().GetClockPosition();

        if (!pos.Valid)
            return;

        gOriginalColourAlpha = CFont::m_Color->a;

        gOriginalDropAlpha = CFont::m_FontDropColor->a;

        CFont::m_Color->a = ScaleAlpha(gOriginalColourAlpha, pos.Alpha);

        CFont::m_FontDropColor->a = ScaleAlpha(gOriginalDropAlpha, pos.Alpha);

        gClockAlphaModified = true;
    }

    static void RestoreClockAlpha(SafetyHookContext&)
    {
        if (!gClockAlphaModified)
            return;

        CFont::m_Color->a = gOriginalColourAlpha;

        CFont::m_FontDropColor->a = gOriginalDropAlpha;

        gClockAlphaModified = false;
    }
}

void ClockRenderHook::Install()
{
    gClockAlphaHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58EC1F),
            ApplyClockAlpha);

    gClockRestoreHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58EC26),
            RestoreClockAlpha);
}

void ClockRenderHook::Remove()
{
    gClockRestoreHook.reset();
    gClockAlphaHook.reset();

    if (gClockAlphaModified)
    {
        CFont::m_Color->a = gOriginalColourAlpha;

        CFont::m_FontDropColor->a = gOriginalDropAlpha;

        gClockAlphaModified = false;
    }
}