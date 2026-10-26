#include <cstdint>
#include <intrin.h>

#include <safetyhook/safetyhook.hpp>

#include <CFont.h>

#include "ClockRenderHook.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookInline& ClockPrintHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    constexpr std::uintptr_t ClockPrintReturnAddress =
        0x58EC26;

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

    static void __cdecl HookedPrintString(
        float x,
        float y,
        const char* text)
    {
        const auto returnAddress =
            reinterpret_cast<std::uintptr_t>(
                _ReturnAddress());

        if (returnAddress != ClockPrintReturnAddress)
        {
            ClockPrintHookStorage().ccall(
                x,
                y,
                text);

            return;
        }

        const auto pos =
            HUDRenderer::Get().GetClockPosition();

        if (!pos.Valid ||
            pos.Alpha == 255)
        {
            ClockPrintHookStorage().ccall(
                x,
                y,
                text);

            return;
        }

        const auto originalColour =
            *CFont::m_Color;

        const auto originalDropColour =
            *CFont::m_FontDropColor;

        CFont::m_Color->a =
            ScaleAlpha(
                originalColour.a,
                pos.Alpha);

        CFont::m_FontDropColor->a =
            ScaleAlpha(
                originalDropColour.a,
                pos.Alpha);

        ClockPrintHookStorage().ccall(
            x,
            y,
            text);

        *CFont::m_Color =
            originalColour;

        *CFont::m_FontDropColor =
            originalDropColour;
    }
}

void ClockRenderHook::Install()
{
    SafetyHookInline& hook = ClockPrintHookStorage();
    hook =
        safetyhook::create_inline(
            reinterpret_cast<void*>(
                0x71A700),
            reinterpret_cast<void*>(
                HookedPrintString));
}

void ClockRenderHook::Remove()
{
    ClockPrintHookStorage().reset();
}