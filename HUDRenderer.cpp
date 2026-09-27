#include "HUDLayout.h"
#include "HUDManager.h"
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
        switch (item.Type)
        {
        case HUDComponentType::Weapon:

            SetWeaponPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Ammo:

            SetAmmoPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Health:

            SetHealthPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Armour:

            SetArmourPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Breath:

            SetBreathPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Money:

            SetMoneyPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Wanted:

            SetWantedPosition(
                static_cast<int>(item.Position.X),
                static_cast<int>(item.Position.Y));

            break;

        case HUDComponentType::Clock:
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

void HUDRenderer::SetWeaponPosition(int x, int y)
{
    mWeapon = {x, y, true};
}

void HUDRenderer::SetAmmoPosition(int x, int y)
{
    mAmmo = {x, y, true};
}

void HUDRenderer::SetHealthPosition(int x, int y)
{
    mHealth = {x, y, true};
}

void HUDRenderer::SetArmourPosition(int x, int y)
{
    mArmour = {x, y, true};
}

void HUDRenderer::SetBreathPosition(int x, int y)
{
    mBreath = {x, y, true};
}

void HUDRenderer::SetMoneyPosition(int x, int y)
{
    mMoney = {x, y, true};
}

void HUDRenderer::SetWantedPosition(int x, int y)
{
    mWanted = {x, y, true};
}