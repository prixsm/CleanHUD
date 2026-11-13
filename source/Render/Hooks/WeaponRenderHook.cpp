#include <algorithm>
#include <cmath>
#include <cstdint>
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

    struct NativeWeaponPosition
    {
        int X = 0;
        int Y = 0;
        std::uint8_t Alpha = 0;
        unsigned int Frame = std::numeric_limits<unsigned int>::max();
    };

    // Only POD frame data is cached; RenderWare textures and TXD slots
    // are resolved afresh to survive game/texture reloads.
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

        // Match CHud::DrawWeaponIcon's model-name + ICON lookup key.
        const unsigned int iconHash =
            CKeyGen::AppendStringToKey(model->m_nKey, "ICON");

        // WeaponIconsTXD inserts its own dictionary ahead of the
        // model's texture dictionary. Do the same for extra icons
        // without touching the mod's two redirected call sites.
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

        const float screenHeight = plugin::screen::GetScreenHeight();
        const float scale = screenHeight / 480.0f;
        const float iconWidth = 47.0f * scale;
        const float iconHeight = 58.0f * scale;

        HUDManager& manager = HUDManager::Get();

        // Custom mods may assign a model to unused weapon slot 47,
        // including a skateboard. The model ID is user-configured.
        const int unusedModel = manager.GetUnusedWeaponIconModel();

        if (unusedModel > 0)
            DrawModelIcon(unusedModel,
                static_cast<float>(p.X),
                static_cast<float>(p.Y),
                iconWidth, iconHeight, p.Alpha);

        // The cellphone is a scripted task, so render its model
        // in the weapon position when a phone-use task is active.
        if (manager.ShouldDrawCellphoneIcon())
            DrawModelIcon(MODEL_CELLPHONE,
                static_cast<float>(p.X),
                static_cast<float>(p.Y),
                iconWidth, iconHeight, p.Alpha);

        // Jetpack can coexist with a firearm, but its icon is shown
        // beside the gun only when explicitly requested. Otherwise,
        // restore it in the usual icon slot when the player is unarmed.
        if (manager.ShouldDrawJetpackBesideWeapon())
            DrawModelIcon(MODEL_JETPACK,
                static_cast<float>(p.X) - iconWidth - 8.0f * scale,
                static_cast<float>(p.Y),
                iconWidth, iconHeight, p.Alpha);
        else if (manager.ShouldDrawJetpackAsWeaponIcon())
            DrawModelIcon(MODEL_JETPACK,
                static_cast<float>(p.X),
                static_cast<float>(p.Y),
                iconWidth, iconHeight, p.Alpha);
    }

    void CaptureWeaponPosition(int x, int y, float alpha)
    {
        gNativeWeaponPosition.X = x;
        gNativeWeaponPosition.Y = y;
        gNativeWeaponPosition.Alpha = static_cast<std::uint8_t>(
            std::clamp(static_cast<int>(std::lround(alpha)), 0, 255));
        gNativeWeaponPosition.Frame = CTimer::m_FrameCounter;
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
            // Capture the actual GTA X/Y when Minimal or TAB uses
            // native positions, including widescreen corrections.
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

        const std::uint32_t originalAlpha =
            stack[9];

        const std::uint32_t cleanAlpha =
            pos.Alpha;

        stack[9] =
            originalAlpha *
            cleanAlpha /
            255u;
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

    // Draw after the game's HUD but before the screen-fade stage.
    // No interception of WeaponIconsTXD's calls at
    // 0x58D84B / 0x58E895, or of its TXD loading.
    plugin::Events::drawHudEvent += []()
    {
        DrawExtraWeaponIcons();
    };
}

void WeaponRenderHook::Remove()
{
    WeaponSpriteAlphaHookStorage().reset();
    WeaponHookStorage().reset();
}