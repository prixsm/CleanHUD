#include <algorithm>
#include <cmath>

#include <safetyhook/safetyhook.hpp>
#include <extensions/Screen.h>

#include "AmmoRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookInline& AmmoHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    SafetyHookInline& AmmoFontHookStorage()
    {
        static SafetyHookInline hook;
        return hook;
    }

    // Only the text calls made *inside* CHud::DrawAmmo are eligible
    // for the last-stage relocation. Never move other HUD/font text.
    bool gDrawingNativeAmmo = false;

    HUDRenderer::HUDAnchor GetStackAmmoTextPosition(
        const HUDRenderer::HUDRenderPosition& pos)
    {
        const float screenWidth = plugin::screen::GetScreenWidth();
        const float screenHeight = plugin::screen::GetScreenHeight();
        const float weaponWidth = 47.0f * (screenHeight / 480.0f);
        const float layoutRightX =
            static_cast<float>(pos.X) * (screenWidth / 1920.0f);
        const float ammoCenterOffset =
            3.4f * (screenHeight / 480.0f);

        return {
            static_cast<float>(std::lround(
                layoutRightX - weaponWidth * 0.5f -
                ammoCenterOffset)),
            static_cast<float>(std::lround(
                static_cast<float>(pos.Y) *
                (screenHeight / 1080.0f)))
        };
    }

    void DrawOriginalAmmo(int player, int x, int y, float alpha)
    {
        // Scoped to the original routine and restored even if an
        // external patch reenters it. No persistent font state.
        const bool previous = gDrawingNativeAmmo;
        gDrawingNativeAmmo = true;
        AmmoHookStorage().ccall(player, x, y, alpha);
        gDrawingNativeAmmo = previous;
    }

    static void __cdecl HookedAmmoPrintString(
        float x, float y, const char* text)
    {
        // Some expanded-weapon loaders redirect branches *inside*
        // CHud::DrawAmmo. Reapply the same Stack coordinates at the
        // actual CFont::PrintString call, regardless of which branch
        // formatted or positioned the ammunition text.
        if (gDrawingNativeAmmo &&
            !HUDManager::Get().IsUsingVanillaLayout())
        {
            const auto pos = HUDRenderer::Get().GetAmmoPosition();
            if (pos.Valid)
            {
                const auto target = GetStackAmmoTextPosition(pos);
                x = target.X;
                y = target.Y;
            }
        }

        AmmoFontHookStorage().ccall(x, y, text);
    }

    static void __cdecl HookedRenderAmmo(
        int player, int x, int y, float alpha)
    {
        const auto pos = HUDRenderer::Get().GetAmmoPosition();

        if (!pos.Valid)
        {
            DrawOriginalAmmo(player, x, y, alpha);
            return;
        }

        alpha = std::clamp(
            alpha * (static_cast<float>(pos.Alpha) / 255.0f),
            0.0f, 255.0f);

        if (!HUDManager::Get().IsUsingVanillaLayout())
        {
            const auto target = GetStackAmmoTextPosition(pos);
            x = static_cast<int>(target.X);
            y = static_cast<int>(target.Y);
        }

        // Keep the existing CHud::DrawAmmo hook so standard weapon
        // paths stay unchanged, plus the last-stage text correction
        // for special one-count/expanded weapon presentation paths.
        DrawOriginalAmmo(player, x, y, alpha);
    }
}

void AmmoRenderHook::Install()
{
    AmmoHookStorage() = safetyhook::create_inline(
        reinterpret_cast<void*>(0x5893B0),
        reinterpret_cast<void*>(HookedRenderAmmo));

    // CFont::PrintString(float,float,const GxtChar*) in SA 1.0 US.
    // The callback only modifies calls nested within DrawAmmo.
    AmmoFontHookStorage() = safetyhook::create_inline(
        reinterpret_cast<void*>(0x71A700),
        reinterpret_cast<void*>(HookedAmmoPrintString));
}

void AmmoRenderHook::Remove()
{
    AmmoFontHookStorage().reset();
    AmmoHookStorage().reset();
}
