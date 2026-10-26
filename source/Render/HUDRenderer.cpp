#include "../Core/HUDComponent.h"
#include "../Layout/HUDLayout.h"
#include "../Core/HUDManager.h"
#include "HUDRenderer.h"

HUDRenderer& HUDRenderer::Get()
{
    static HUDRenderer renderer;

    static bool initialized = false;

    if (!initialized)
    {
        renderer.SetAnchor(
            1865.0f,
            50.0f);

        initialized = true;
    }

    return renderer;
}

bool HUDRenderer::ShouldOverridePosition() const
{
    return !HUDManager::Get().IsUsingVanillaLayout();
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetClockPosition() const
{
    return mClock;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetWeaponPosition() const
{
    return mWeapon;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetAmmoPosition() const
{
    return mAmmo;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetHealthPosition() const
{
    return mHealth;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetArmourPosition() const
{
    return mArmour;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetBreathPosition() const
{
    return mBreath;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetMoneyPosition() const
{
    return mMoney;
}

HUDRenderer::HUDRenderPosition HUDRenderer::GetWantedPosition() const
{
    return mWanted;
}

void HUDRenderer::Reset()
{
    mClock = {};
    mWeapon = {};
    mAmmo = {};
    mHealth = {};
    mArmour = {};
    mBreath = {};
    mMoney = {};
    mWanted = {};
}

void HUDRenderer::Build(const HUDLayout& layout)
{
    Reset();

    for (const auto& item : layout.GetItems())
    {
        const uint8_t alpha =
            item.Component
            ? item.Component->GetAlpha()
            : 255;

        switch (item.Type)
        {
        case HUDComponentType::Clock:

            SetClockPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Weapon:

            SetWeaponPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Ammo:

            SetAmmoPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Health:

            SetHealthPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Armour:

            SetArmourPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Breath:

            SetBreathPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Money:

            SetMoneyPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        case HUDComponentType::Wanted:

            SetWantedPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y),
                alpha);

            break;

        default:
            break;
        }
    }
}

HUDRenderer::HUDAnchor HUDRenderer::GetAnchor() const
{
    return mAnchor;
}

void HUDRenderer::SetAnchor(float x, float y)
{
    mAnchor = {x, y};
}

void HUDRenderer::SetClockPosition(int x, int y, uint8_t alpha)
{
    mClock = {x, y, alpha, true};
}

void HUDRenderer::SetWeaponPosition(int x, int y, uint8_t alpha)
{
    mWeapon = {x, y, alpha, true};
}

void HUDRenderer::SetAmmoPosition(int x, int y, uint8_t alpha)
{
    mAmmo = {x, y, alpha, true};
}

void HUDRenderer::SetHealthPosition(int x, int y, uint8_t alpha)
{
    mHealth = {x, y, alpha, true};
}

void HUDRenderer::SetArmourPosition(int x, int y, uint8_t alpha)
{
    mArmour = {x, y, alpha, true};
}

void HUDRenderer::SetBreathPosition(int x, int y, uint8_t alpha)
{
    mBreath = {x, y, alpha, true};
}

void HUDRenderer::SetMoneyPosition(int x, int y, uint8_t alpha)
{
    mMoney = {x, y, alpha, true};
}

void HUDRenderer::SetWantedPosition(int x, int y, uint8_t alpha)
{
    mWanted = {x, y, alpha, true};
}