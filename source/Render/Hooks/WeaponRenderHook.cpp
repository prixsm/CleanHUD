#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include <plugin.h>
#include <CTimer.h>
#include <CTxdStore.h>
#include <CKeyGen.h>
#include <CModelInfo.h>
#include <CSprite2d.h>
#include <CRect.h>
#include <eModelID.h>

#include <safetyhook/safetyhook.hpp>

#include <extensions/Screen.h>

#include "WeaponRenderHook.h"
#include "../../Core/HUDManager.h"
#include "../../Core/HUDSettings.h"
#include "../HUDRenderer.h"

namespace
{
    SafetyHookMid& WeaponHookStorage()
    {
        static SafetyHookMid hook;
        return hook;
    }
    SafetyHookMid& WeaponSpriteAlphaHookStorage()
    {
        static SafetyHookMid hook;
        return hook;
    }
    SafetyHookMid& WeaponFistSpriteHookStorage()
    {
        static SafetyHookMid hook;
        return hook;
    }

    struct NativeWeaponPosition
    {
        float X = 0.0f;
        float Y = 0.0f;
        // Remember the last measured icon size while the icon is hidden.
        // Recheck the size if the screen resolution changes.
        float Width = 0.0f;
        float Height = 0.0f;
        float CapturedScreenWidth = 0.0f;
        float CapturedScreenHeight = 0.0f;
        // Offset from the game's weapon icon position, which other mods can change.
        float OffsetX = 0.0f;
        float OffsetY = 0.0f;
        float AnchorX = 0.0f;
        float AnchorY = 0.0f;
        std::uint8_t Alpha = 0;
        unsigned int Frame = std::numeric_limits<unsigned int>::max();
    };

    // Store only icon measurements, not textures. Look up textures
    // each time so game reloads don't leave invalid references.
    NativeWeaponPosition gNativeWeaponPosition;

    RwTexture* FindIconTexture(int modelId)
    {
        if (modelId <= 0 || modelId >= CModelInfo::ms_modelInfoCount ||
            !CTxdStore::ms_pTxdPool)
        {
            return nullptr;
        }

        CBaseModelInfo* model = CModelInfo::GetModelInfo(modelId);

        if (!model)
            return nullptr;

        // Look up the same model-name + ICON texture that GTA uses.
        const unsigned int iconHash =
            CKeyGen::AppendStringToKey(model->m_nKey, "ICON");

        // Look for WeaponIconsTXD textures before the model's textures,
        // without changing that mod's drawing hooks.
        const int customSlot = CTxdStore::FindTxdSlot("weaponicons");

        if (customSlot >= 0)
        {
            TxdDef* custom = CTxdStore::ms_pTxdPool->GetAt(customSlot);

            if (custom && custom->m_pRwDictionary)
            {
                if (RwTexture* tex = RwTexDictionaryFindHashNamedTexture(
                    custom->m_pRwDictionary, iconHash))
                {
                    return tex;
                }
            }
        }

        if (model->m_nTxdIndex < 0)
            return nullptr;

        TxdDef* native = CTxdStore::ms_pTxdPool->GetAt(model->m_nTxdIndex);

        return native && native->m_pRwDictionary
            ? RwTexDictionaryFindHashNamedTexture(
                native->m_pRwDictionary, iconHash)
            : nullptr;
    }

    void DrawModelIcon(int modelId, float x, float y,
        float width, float height, std::uint8_t alpha)
    {
        RwTexture* texture = FindIconTexture(modelId);

        if (!texture || alpha == 0)
            return;

        // CSprite2d borrows the TXD's texture; do not Delete() it.
        CSprite2d sprite;
        sprite.m_pTexture = texture;
        sprite.Draw(
            CRect(x, y, x + width, y + height),
            CRGBA(255, 255, 255, alpha));
    }

    void DrawExtraWeaponIcons()
    {
        if (gNativeWeaponPosition.Frame != CTimer::m_FrameCounter ||
            gNativeWeaponPosition.Alpha == 0 ||
            !HUDManager::Get().IsPlayerHUDInitialized())
        {
            return;
        }

        const auto& p = gNativeWeaponPosition;
        const float screenWidth = plugin::screen::GetScreenWidth();
        const float screenHeight = plugin::screen::GetScreenHeight();

        // Reuse the last icon size when GTA skips a frame of drawing.
        // Measure again after a resolution change.
        const bool haveMeasuredScale =
            p.Width > 0.0f && p.Height > 0.0f &&
            p.CapturedScreenWidth == screenWidth &&
            p.CapturedScreenHeight == screenHeight;
        const float iconWidth = haveMeasuredScale
            ? p.Width : 47.0f * (screenHeight / 480.0f);
        const float iconHeight = haveMeasuredScale
            ? p.Height : 58.0f * (screenHeight / 480.0f);
        const float iconX = p.AnchorX +
            (haveMeasuredScale ? p.OffsetX : 0.0f);
        const float iconY = p.AnchorY +
            (haveMeasuredScale ? p.OffsetY : 0.0f);

        HUDManager& manager = HUDManager::Get();

        const int customModel = manager.GetCustomWeaponIconModel();
        if (customModel > 0 && !manager.ShouldDrawCellphoneIcon() &&
            !manager.ShouldReplaceWeaponWithJetpack() &&
            !manager.ShouldReplaceWeaponWithSkateboard())
            DrawModelIcon(customModel, iconX, iconY,
                iconWidth, iconHeight, p.Alpha);

        if (manager.ShouldDrawCellphoneIcon())
            DrawModelIcon(MODEL_CELLPHONE, iconX, iconY,
                iconWidth, iconHeight, manager.GetCellphoneIconAlpha());

        const auto& settings = HUDSettings::Get();
        constexpr float SideBySideShiftX = 12.0f;
        const float offsetScaleX = screenWidth / 1920.0f;
        const float offsetScaleY = screenHeight / 1080.0f;

        // Jetpack replaces fists or knuckles but sits beside other weapons.
        // The side-by-side position includes a 12-unit right shift.
        const float jetpackX = manager.ShouldReplaceWeaponWithJetpack()
            ? iconX + settings.JetpackIconOffsetX * offsetScaleX
            : iconX - iconWidth -
                8.0f * (iconWidth / 47.0f) +
                (SideBySideShiftX + settings.JetpackIconOffsetX) *
                    offsetScaleX;
        const float jetpackY = iconY +
            settings.JetpackIconOffsetY * offsetScaleY;

        // Keep the phone in the weapon slot and place jetpack beside it.
        if (manager.ShouldDrawJetpackIcon())
            DrawModelIcon(MODEL_JETPACK, jetpackX, jetpackY,
                iconWidth, iconHeight, manager.GetJetpackIconAlpha());

        if (manager.ShouldDrawSkateboardIcon())
        {
            // The riding skateboard icon fades separately and uses the game's
            // weapon icon size. With another weapon, show it to the left.
            const float sideGap =
                8.0f * (iconWidth / 47.0f);
            const float sideShift = 12.0f * offsetScaleX;
            const float skateX = manager.ShouldReplaceWeaponWithSkateboard()
                ? iconX + settings.SkateboardIconOffsetX * offsetScaleX
                : iconX - iconWidth - sideGap + sideShift +
                    settings.SkateboardIconOffsetX * offsetScaleX -
                    (manager.ShouldDrawJetpackIcon()
                        ? iconWidth + sideGap : 0.0f);
            const float skateY = iconY +
                settings.SkateboardIconOffsetY * offsetScaleY;

            DrawModelIcon(manager.GetSkateboardIconModel(),
                skateX, skateY, iconWidth, iconHeight,
                manager.GetSkateboardIconAlpha());
        }
    }

    void CaptureWeaponPosition(int x, int y, float alpha)
    {
        gNativeWeaponPosition.AnchorX = static_cast<float>(x);
        gNativeWeaponPosition.AnchorY = static_cast<float>(y);
        gNativeWeaponPosition.X = static_cast<float>(x);
        gNativeWeaponPosition.Y = static_cast<float>(y);
        gNativeWeaponPosition.Alpha = static_cast<std::uint8_t>(
            std::clamp(static_cast<int>(std::lround(alpha)), 0, 255));
        gNativeWeaponPosition.Frame = CTimer::m_FrameCounter;
    }

    void CaptureWeaponBounds(float left, float top,
        float width, float height)
    {
        if (gNativeWeaponPosition.Frame != CTimer::m_FrameCounter ||
            !std::isfinite(left) || !std::isfinite(top) ||
            !std::isfinite(width) || !std::isfinite(height) ||
            width <= 0.0f || height <= 0.0f)
        {
            return;
        }

        gNativeWeaponPosition.X = left;
        gNativeWeaponPosition.Y = top;
        gNativeWeaponPosition.Width = width;
        gNativeWeaponPosition.Height = height;
        gNativeWeaponPosition.CapturedScreenWidth =
            plugin::screen::GetScreenWidth();
        gNativeWeaponPosition.CapturedScreenHeight =
            plugin::screen::GetScreenHeight();
        gNativeWeaponPosition.OffsetX =
            left - gNativeWeaponPosition.AnchorX;
        gNativeWeaponPosition.OffsetY =
            top - gNativeWeaponPosition.AnchorY;
    }

    constexpr std::uintptr_t WeaponIconStart = 0x58D7D0;

    constexpr std::uintptr_t WeaponIconEnd = 0x58D9A0;

    static void HookedWeaponPosition(SafetyHookContext& ctx)
    {
        const auto pos = HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
            return;

        auto* alpha =
            reinterpret_cast<float*>(
                ctx.esp + 4);

        *alpha *=
            static_cast<float>(
                pos.Alpha) /
            255.0f;

        if (HUDManager::Get().IsUsingVanillaLayout())
        {
            // Keep the game's positions in Minimal mode or during HUD Preview.
            CaptureWeaponPosition(
                static_cast<int>(ctx.eax),
                static_cast<int>(*reinterpret_cast<std::uint32_t*>(ctx.esp)),
                *alpha);
            return;
        }

        const float screenWidth = plugin::screen::GetScreenWidth();

        const float screenHeight = plugin::screen::GetScreenHeight();

        constexpr float WeaponNativeWidth = 47.0f;

        const float weaponWidth =
            WeaponNativeWidth *
            (screenHeight / 480.0f);

        const float layoutRightX =
            static_cast<float>(pos.X) *
            (screenWidth / 1920.0f);

        const int layoutX =
            static_cast<int>(
                std::lround(
                    layoutRightX -
                    weaponWidth));

        const int layoutY =
            static_cast<int>(
                std::lround(
                    static_cast<float>(pos.Y) *
                    (screenHeight / 1080.0f)));

        ctx.eax =
            static_cast<std::uintptr_t>(
                layoutX);

        auto* stack =
            reinterpret_cast<
            std::uint32_t*>(
                ctx.esp);

        stack[0] =
            static_cast<std::uint32_t>(
                layoutY);

        CaptureWeaponPosition(layoutX, layoutY, *alpha);
    }

    static void HookedWeaponFistSprite(SafetyHookContext& ctx)
    {
        // GTA draws fists and weapons in different ways, so handle both.
        auto* stack = reinterpret_cast<std::uintptr_t*>(ctx.esp);
        if (stack[0] < WeaponIconStart || stack[0] >= WeaponIconEnd)
            return;

        const auto* rect = reinterpret_cast<const CRect*>(stack[1]);
        if (rect)
            CaptureWeaponBounds(rect->left, rect->top,
                rect->right - rect->left, rect->bottom - rect->top);

        if (HUDManager::Get().ShouldSuppressNativeFist())
        {
            // Hide only the game's fist drawing; don't change the game's saved colors.
            static const CRGBA transparent(255, 255, 255, 0);
            stack[2] = reinterpret_cast<std::uintptr_t>(&transparent);
        }
    }

    static void HookedWeaponSpriteAlpha(SafetyHookContext& ctx)
    {
        auto* stack =
            reinterpret_cast<std::uint32_t*>(
                ctx.esp);

        const std::uintptr_t returnAddress =
            stack[0];

        if (returnAddress < WeaponIconStart ||
            returnAddress >= WeaponIconEnd)
        {
            return;
        }

        const auto pos = HUDRenderer::Get().GetWeaponPosition();

        if (!pos.Valid)
            return;

        // Measure the game's weapon icon before changing its transparency.
        const auto readFloat = [stack](int index)
        {
            float value = 0.0f;
            std::memcpy(&value, &stack[index], sizeof(value));
            return value;
        };
        const float halfWidth = readFloat(4);
        const float halfHeight = readFloat(5);
        CaptureWeaponBounds(
            readFloat(1) - halfWidth,
            readFloat(2) - halfHeight,
            2.0f * halfWidth, 2.0f * halfHeight);

        const std::uint32_t originalAlpha =
            stack[9];

        const std::uint32_t cleanAlpha =
            pos.Alpha;

        // Phone hides knuckles; jetpack never replaces a real weapon icon.
        stack[9] = HUDManager::Get().ShouldSuppressNativeWeaponSprite()
            ? 0u : originalAlpha * cleanAlpha / 255u;
    }
}

void WeaponRenderHook::Install()
{
    SafetyHookMid& hook = WeaponHookStorage();
    hook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x58F93E),
            HookedWeaponPosition);

    SafetyHookMid& secondHook = WeaponSpriteAlphaHookStorage();
    secondHook =
        safetyhook::create_mid(
            reinterpret_cast<void*>(
                0x70D000),
            HookedWeaponSpriteAlpha);

    // Handle the fist icon's separate drawing path so its size
    // still matches WidescreenFix and MixSets.
    SafetyHookMid& fistHook = WeaponFistSpriteHookStorage();
    fistHook = safetyhook::create_mid(
        reinterpret_cast<void*>(0x728350),
        HookedWeaponFistSprite);

    // Draw additional icons after the game's HUD, before screen fades.
    // Leave WeaponIconsTXD's hooks and texture loading untouched.
    plugin::Events::drawHudEvent += []()
    {
        DrawExtraWeaponIcons();
    };
}

void WeaponRenderHook::Remove()
{
    WeaponFistSpriteHookStorage().reset();
    WeaponSpriteAlphaHookStorage().reset();
    WeaponHookStorage().reset();
}