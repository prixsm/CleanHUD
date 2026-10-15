#include <algorithm>

#include <common.h>
#include <CCamera.h>
#include <CCutsceneMgr.h>
#include <CDarkel.h>
#include <CPad.h>
#include <CPedIntelligence.h>
#include <CPlayerData.h>
#include <CPlayerInfo.h>
#include <CPlayerPed.h>
#include <CStats.h>
#include <CVehicle.h>
#include <CTaskSimpleSwim.h>
#include <CWanted.h>
#include <CWeaponInfo.h>
#include <CWorld.h>

#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"
#include "HUDSettings.h"

HUDManager::HUDManager() :
    mClock(
        HUDComponentType::Clock,
        {
            HUDPatch::Clock
        }),
    mHealth(
        HUDComponentType::Health,
        {
            HUDPatch::Health
        }),
    mArmour(
        HUDComponentType::Armour,
        {
            HUDPatch::Armour
        }),
    mBreath(
        HUDComponentType::Breath,
        {
            HUDPatch::Breath1,
            HUDPatch::Breath2
        }),
    mMoney(
        HUDComponentType::Money,
        {}),
    mWeaponIcon(
        HUDComponentType::Weapon,
        {
            HUDPatch::Weapon
        }),
    mAmmo(
        HUDComponentType::Ammo,
        {
            HUDPatch::Ammo
        }),
    mWanted(
        HUDComponentType::Wanted,
        {},
        false)
{
}

namespace
{
    static bool IsMeleeWeapon(
        eWeaponType weaponType)
    {
        return weaponType >= WEAPONTYPE_UNARMED &&
            weaponType <= WEAPONTYPE_CANE;
    }

    static bool IsDriveByWeapon(eWeaponType weaponType)
    {
        return weaponType == WEAPONTYPE_MICRO_UZI ||
            weaponType == WEAPONTYPE_TEC9 ||
            weaponType == WEAPONTYPE_MP5;
    }

}

HUDManager::ActiveWeaponState HUDManager::GetCurrentWeaponState() const
{
    ActiveWeaponState state;

    if (!mPlayer)
        return state;

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon)
        return state;

    state.WeaponType = weapon->m_eWeaponType;

    return state;
}

HUDManager::AmmoState HUDManager::GetCurrentAmmoState() const
{
    AmmoState state;

    if (!mPlayer)
        return state;

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon)
        return state;

    state.AmmoInClip = weapon->m_nAmmoInClip;
    state.AmmoTotal = weapon->m_nAmmoTotal;

    return state;
}

bool HUDManager::HasWantedLevel() const
{
    if (!mPlayer || !mPlayer->m_pPlayerData)
        return false;

    const CWanted* wanted =
        mPlayer->m_pPlayerData->m_pWanted;

    if (!wanted)
        return false;

    return wanted->m_nWantedLevel > 0 ||
        wanted->m_nWantedLevelBeforeParole > 0;
}

bool HUDManager::IsInitialHealthTrackingBlocked() const
{
    if (!mPlayer)
        return true;

    CPad* pad =
        mPlayer->GetPadFromPlayer();

    if (!pad)
        return true;

    const bool cutsceneActive =
        CCutsceneMgr::ms_cutsceneProcessing ||
        pad->bPlayerSafeForCutscene;

    const bool controlsDisabled =
        pad->DisablePlayerControls != 0;

    const bool screenNotReady =
        TheCamera.m_bFading ||
        TheCamera.m_fFadeAlpha > 0.0f;

    return
        cutsceneActive ||
        controlsDisabled ||
        screenNotReady;
}

bool HUDManager::IsPlayerHUDInitialized() const
{
    return mSessionInitialized;
}

bool HUDManager::IsUsingVanillaLayout() const
{
    return
        mVanillaLayoutLatched ||
        (GetAsyncKeyState(VK_TAB) & 0x8000);
}

bool HUDManager::ShouldDisplayAmmo() const
{
    if (!mPlayer)
        return false;

    const CWeapon* weapon =
        mPlayer->GetWeapon();

    if (!weapon)
        return false;

    constexpr unsigned int MaxClip = 9999;

    const eWeaponType weaponType =
        weapon->m_eWeaponType;

    const CWeaponInfo* weaponInfo =
        CWeaponInfo::GetWeaponInfo(
            weaponType);

    if (!weaponInfo)
        return false;

    if (IsMeleeWeapon(weaponType))
        return false;

    if (weapon->m_nAmmoTotal -
        weapon->m_nAmmoInClip >= MaxClip)
    {
        return false;
    }

    if (CDarkel::FrenzyOnGoing())
        return false;

    switch (weaponType)
    {
    case WEAPONTYPE_DETONATOR:
    case WEAPONTYPE_PARACHUTE:
        return false;

    default:
        break;
    }

    if (weaponInfo->m_nWeaponFire ==
        WEAPON_FIRE_USE)
    {
        return false;
    }

    return true;
}

bool HUDManager::ShouldDisplayArmour() const
{
    return mPlayer &&
        mPlayer->m_fArmour > 1.0f;
}

void HUDManager::RestoreVanillaElements()
{
    const auto& settings = HUDSettings::Get();

    // Construction initially disables all patched draws.
    // Restore user-disabled components immediately instead
    // of waiting for health initialization or the first
    // update frame.
    if (!settings.Clock)
        mClock.Update(true);
    if (!settings.Health)
        mHealth.Update(true);
    if (!settings.Armour)
        mArmour.Update(true);
    if (!settings.Breath)
        mBreath.Update(true);
    if (!settings.Weapon)
        mWeaponIcon.Update(true);
    if (!settings.Ammo)
        mAmmo.Update(true);
}

void HUDManager::OnGameLoaded()
{
    mSessionInitialized = false;
    mVanillaLayoutLatched = false;
    mWasWeaponAiming = false;

    mPlayer = nullptr;
    mLastPlayer = nullptr;

    mNeedsReset = true;
}

void HUDManager::Update()
{
    mPlayer = FindPlayerPed();

    if (!mPlayer)
    {
        mSessionInitialized = false;
        mLastPlayer = nullptr;
        mWasWeaponAiming = false;
        return;
    }

    const auto& settings = HUDSettings::Get();

    mVisibleTime = settings.VisibilityMs;

    const bool tabHeld =
        GetAsyncKeyState(VK_TAB) & 0x8000;

    if (tabHeld)
    {
        mVanillaLayoutLatched = true;
    }

    const auto origin =
        HUDRenderer::Get().GetAnchor();

    mLayout.SetAnchor(
        origin.X,
        origin.Y);
    mLayout.Reset();

    if (tabHeld || !settings.Clock)
    {
        mClock.Show();
    }
    else
    {
        mClock.Hide();
    }

    if (mNeedsReset)
    {
        ResetSessionState();
        mNeedsReset = false;
    }

    if (mPlayer != mLastPlayer)
    {
        mLastPlayer = mPlayer;
        mSessionInitialized = false;
        mNeedsReset = true;
        return;
    }

    bool isDead = (mPlayer->m_fHealth <= 0.0f);

    if (mWasDead && !isDead)
    {
        mSessionInitialized = false;
        mNeedsReset = true;
        mWasDead = false;
        return;
    }

    mWasDead = isDead;

    UpdateHealth();
    UpdateArmour();
    UpdateBreath();
    UpdateMoney();
    UpdateWeaponIcon();
    UpdateAmmo();
    UpdateWanted();

    const bool weaponAiming = IsWeaponAiming();

    if (mWasWeaponAiming && !weaponAiming)
    {
        mWeaponIcon.Hide();
        mAmmo.Hide();
    }

    mWasWeaponAiming = weaponAiming;

    mClock.Update(!settings.Clock);

    // Disabled CleanHUD elements retain vanilla draw calls.
    // HUDLayout::Add excludes them from CleanHUD's stack.
    mHealth.Update(tabHeld || !settings.Health);
    mArmour.Update(tabHeld || !settings.Armour);
    mBreath.Update(tabHeld || !settings.Breath);
    mMoney.Update(tabHeld || !settings.Money);
    mWeaponIcon.Update(
        tabHeld || weaponAiming || !settings.Weapon);
    mAmmo.Update(
        tabHeld || !settings.Ammo || weaponAiming);

    mWanted.Update(!settings.Wanted);

    if (HasEmptyVehicleAmmo())
    {
        if (settings.Weapon)
            mWeaponIcon.Reset();

        if (settings.Ammo)
            mAmmo.Reset();
    }

    // Prepare one-way stack displacement from what was already
    // visible in the preceding frame. Order must match layout.
    mLayout.PrepareStack({
        &mClock, &mMoney, &mWanted, &mWeaponIcon,
        &mAmmo, &mArmour, &mBreath, &mHealth
    });

    BuildPlayerHUDLayout();
    HUDRenderer::Get().Build(mLayout);

    if (!tabHeld)
    {
        const bool tabFadeOutActive =
            mClock.IsFadingOut() ||
            mMoney.IsFadingOut() ||
            mHealth.IsFadingOut() ||
            mArmour.IsFadingOut() ||
            mBreath.IsFadingOut() ||
            mWeaponIcon.IsFadingOut() ||
            mAmmo.IsFadingOut();

        if (!tabFadeOutActive)
        {
            mVanillaLayoutLatched = false;
        }
    }
}

void HUDManager::ResetSessionState()
{
    HideAll();
    RestoreVanillaElements();
    mLayout.ResetHistory();
    mWasWeaponAiming = false;

    mPreviousHealth = mLastHealth;
    mHealthStableFrames = 0;
    mWaitingForStableHealth = true;

    mPreviousArmour = mLastArmour;
    mArmourStableFrames = 0;
    mWaitingForStableArmour = true;

    mPreviousMoney = mLastMoney;
    mMoneyStableFrames = 0;
    mWaitingForStableMoney = true;

    mWeaponStableFrames = 0;
    mWaitingForStableWeapon = true;

    mAmmoStableFrames = 0;
    mWaitingForStableAmmo = true;

    SyncTrackedValues();
}

void HUDManager::SyncTrackedValues()
{
    mLastHealth = mPlayer->m_fHealth;
    mLastArmour = mPlayer->m_fArmour;
    mLastMoney = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;

    mWasDead = (mPlayer->m_fHealth <= 0.0f);
    mLastWeaponState = GetCurrentWeaponState();
    mLastAmmoState = GetCurrentAmmoState();
}

bool HUDManager::UpdateHealthInitialization(float currentHealth)
{
    if (!mWaitingForStableHealth)
        return true;

    if (IsInitialHealthTrackingBlocked())
    {
        mSessionInitialized = false;

        mHealthStableFrames = 0;

        mLastHealth =
            currentHealth;

        mPreviousHealth =
            currentHealth;

        mHealth.Hide();
        mHealth.Update();

        return false;
    }

    if (currentHealth == mPreviousHealth)
    {
        mHealthStableFrames++;

        if (mHealthStableFrames >= 2)
        {
            mWaitingForStableHealth = false;
            mSessionInitialized = true;
        }
    }
    else
    {
        mHealthStableFrames = 0;
    }

    mLastHealth =
        currentHealth;

    mPreviousHealth =
        currentHealth;

    return false;
}

bool HUDManager::UpdateArmourInitialization(float currentArmour)
{
    if (!mWaitingForStableArmour)
        return true;

    if (currentArmour == mPreviousArmour)
    {
        mArmourStableFrames++;

        if (mArmourStableFrames >= 2)
        {
            mWaitingForStableArmour = false;
        }
    }
    else
    {
        mArmourStableFrames = 0;
    }

    mLastArmour = currentArmour;
    mPreviousArmour = currentArmour;

    return false;
}

bool HUDManager::UpdateMoneyInitialization(int currentMoney)
{
    if (!mWaitingForStableMoney)
        return true;

    if (currentMoney == mPreviousMoney)
    {
        mMoneyStableFrames++;

        if (mMoneyStableFrames >= 2)
            mWaitingForStableMoney = false;
    }
    else
    {
        mMoneyStableFrames = 0;
    }

    mLastMoney = currentMoney;
    mPreviousMoney = currentMoney;

    return false;
}

bool HUDManager::UpdateWeaponInitialization(const ActiveWeaponState& current)
{
    if (!mWaitingForStableWeapon)
        return true;

    if (current.WeaponType == mLastWeaponState.WeaponType)
    {
        mWeaponStableFrames++;

        if (mWeaponStableFrames >= 2)
            mWaitingForStableWeapon = false;
    }
    else
    {
        mWeaponStableFrames = 0;

        mLastWeaponState = current;
    }

    return false;
}

bool HUDManager::UpdateAmmoInitialization(const AmmoState& current)
{
    if (!mWaitingForStableAmmo)
        return true;

    if (current == mLastAmmoState)
    {
        mAmmoStableFrames++;

        if (mAmmoStableFrames >= 2)
            mWaitingForStableAmmo = false;
    }
    else
    {
        mAmmoStableFrames = 0;
        mLastAmmoState = current;
    }

    return false;
}

bool HUDManager::HasHealthChanged(float currentHealth)
{
    if (fabs(currentHealth - mLastHealth) <= 0.01f)
        return false;

    mLastHealth = currentHealth;
    return true;
}

bool HUDManager::HasArmourChanged(float currentArmour)
{
    if (fabs(currentArmour - mLastArmour) <= 0.01f)
        return false;

    mLastArmour = currentArmour;

    return true;
}

bool HUDManager::HasMoneyChanged(int currentMoney)
{
    if (currentMoney == mLastMoney)
        return false;

    mLastMoney = currentMoney;

    return true;
}

bool HUDManager::HasWeaponChanged(const ActiveWeaponState& current)
{
    if (current.WeaponType == mLastWeaponState.WeaponType)
        return false;

    mLastWeaponState = current;

    return true;
}

bool HUDManager::HasAmmoChanged(const AmmoState& current)
{
    if (current == mLastAmmoState)
        return false;

    mLastAmmoState = current;

    return true;
}

void HUDManager::UpdateHealth()
{
    const float currentHealth =
        mPlayer->m_fHealth;

    if (!UpdateHealthInitialization(
        currentHealth))
    {
        return;
    }

    const bool healthChanged =
        HasHealthChanged(currentHealth);

    const bool criticallyLow =
        static_cast<int>(currentHealth) < 10;

    if (criticallyLow)
    {
        mHealth.Show();
        return;
    }

    if (healthChanged)
    {
        mHealth.ShowFor(
            mVisibleTime);
    }
}

void HUDManager::UpdateArmour()
{
    float currentArmour = mPlayer->m_fArmour;

    if (!UpdateArmourInitialization(currentArmour))
        return;

    if (HasArmourChanged(currentArmour))
        mArmour.ShowFor(mVisibleTime);
}

void HUDManager::UpdateBreath()
{
    if (!mPlayer ||
        !mPlayer->m_pPlayerData)
    {
        mBreath.Hide();
        return;
    }

    auto* data =
        mPlayer->m_pPlayerData;

    const float maxBreath =
        CStats::GetFatAndMuscleModifier(
            STAT_MOD_AIR_IN_LUNG);

    const bool breathDepleted =
        data->m_fBreath < maxBreath;

    bool underwater = false;

    if (mPlayer->m_pIntelligence)
    {
        if (auto* swim =
            mPlayer->m_pIntelligence->GetTaskSwim())
        {
            underwater =
                swim->m_nSwimState ==
                SWIM_DIVE_UNDERWATER ||
                swim->m_nSwimState ==
                SWIM_UNDERWATER_SPRINTING;
        }
    }

    if (underwater || breathDepleted)
    {
        mBreath.Show();
    }
    else
    {
        mBreath.Hide();
    }
}

void HUDManager::UpdateMoney()
{
    const auto& playerInfo = CWorld::Players[CWorld::PlayerInFocus];

    int currentMoney = playerInfo.m_nMoney;
    int displayedMoney = playerInfo.m_nDisplayMoney;

    if (!UpdateMoneyInitialization(currentMoney))
        return;

    if (HasMoneyChanged(currentMoney))
    {
        mMoney.ShowFor(mVisibleTime);
    }

    if (displayedMoney != currentMoney)
    {
        mMoney.ShowFor(mVisibleTime);
    }
}

void HUDManager::UpdateWeaponIcon()
{
    ActiveWeaponState current = GetCurrentWeaponState();

    if (!UpdateWeaponInitialization(current))
        return;

    if (HasWeaponChanged(current))
    {
        mWeaponIcon.ShowFor(mVisibleTime);
        mAmmo.ShowFor(mVisibleTime);
    }
}

void HUDManager::UpdateAmmo()
{
    AmmoState current = GetCurrentAmmoState();

    if (!UpdateAmmoInitialization(current))
        return;

    if (HasAmmoChanged(current))
    {
        mAmmo.ShowFor(mVisibleTime);
        mWeaponIcon.ShowFor(mVisibleTime);
    }
}

bool HUDManager::IsWeaponAiming() const
{
    if (mWaitingForStableWeapon ||
        mWaitingForStableAmmo ||
        !ShouldDisplayAmmo())
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    if (!weapon || weapon->m_nAmmoTotal == 0)
        return false;

    CPad* pad = mPlayer->GetPadFromPlayer();

    if (!pad || pad->DisablePlayerControls)
        return false;

    if (!mPlayer->bInVehicle)
        return HUDSettings::Get().ShowOnFootAim &&
            pad->GetTarget();

    const CVehicle* vehicle = mPlayer->m_pVehicle;

    if (!vehicle ||
        vehicle->m_nVehicleSubClass == VEHICLE_BMX)
    {
        return false;
    }

    const auto& settings = HUDSettings::Get();

    const bool driveByAim =
        settings.ShowOnVehicleLook &&
        (pad->GetLookLeft() || pad->GetLookRight());

    // Firing is always an explicit display trigger.
    const bool driveByFire =
        IsDriveByWeapon(weapon->m_eWeaponType) &&
        pad->GetCarGunFired() != 0;

    return driveByAim || driveByFire;
}

bool HUDManager::HasEmptyVehicleAmmo() const
{
    if (!mPlayer || !mPlayer->bInVehicle ||
        !ShouldDisplayAmmo())
    {
        return false;
    }

    const CWeapon* weapon = mPlayer->GetWeapon();

    return weapon &&
        weapon->m_nAmmoTotal == 0 &&
        weapon->m_nAmmoInClip == 0;
}

void HUDManager::UpdateWanted()
{
    if (HasWantedLevel())
        mWanted.Show();
    else
        mWanted.Hide();
}

void HUDManager::BuildPlayerHUDLayout()
{
    constexpr float ClockHeight = 18.0f;
    constexpr float MoneyHeight = 18.0f;
    constexpr float WantedHeight = 18.0f;
    constexpr float AmmoHeight = 16.0f;

    constexpr float WeaponAmmoOffsetX = 12.0f;
    constexpr float WeaponAmmoOffsetY = -12.0f;
    constexpr float WeaponToAmmoOffset = 83.0f;

    const auto& settings = HUDSettings::Get();
    const float statusBarHeight =
        HUDMetrics::GetStatusBarSpacing();

    const bool ammoDisplayed =
        settings.Ammo && ShouldDisplayAmmo();

    const bool armourDisplayed =
        settings.Armour && ShouldDisplayArmour();

    const bool wantedDisplayed =
        settings.Wanted && mWanted.IsVisible();

    const bool weaponDisplayed =
        (settings.Weapon && mWeaponIcon.IsVisible()) ||
        (ammoDisplayed && mAmmo.IsVisible());

    const bool statusDisplayed =
        (armourDisplayed && mArmour.IsVisible()) ||
        (settings.Breath && mBreath.IsVisible()) ||
        (settings.Health && mHealth.IsVisible());

    // Set each gap to its final size as soon as its next
    // component is present. A newly shown item below the
    // current stack should fade at that final position.
    float moneySpacing = 0.0f;

    if (wantedDisplayed)
    {
        moneySpacing = settings.MoneyToWantedSpacing;
    }
    else if (weaponDisplayed)
    {
        moneySpacing = settings.MoneyToWeaponSpacing;
    }
    else if (statusDisplayed)
    {
        moneySpacing = settings.MoneyToStatusBarSpacing;
    }

    float wantedSpacing = 0.0f;

    if (wantedDisplayed)
    {
        if (weaponDisplayed)
            wantedSpacing = settings.WantedToWeaponSpacing;
        else if (statusDisplayed)
            wantedSpacing = settings.WantedToStatusBarSpacing;
    }

    mLayout.Add(
        mClock,
        ClockHeight,
        0.0f,
        settings.ClockSpacing);

    mLayout.Add(
        mMoney,
        MoneyHeight,
        0.0f,
        moneySpacing);

    mLayout.Add(
        mWanted,
        WantedHeight,
        0.0f,
        wantedSpacing);

    mLayout.Add(
        mWeaponIcon,
        WeaponToAmmoOffset - settings.WeaponSpacing,
        WeaponAmmoOffsetY,
        ammoDisplayed
        ? settings.WeaponSpacing
        : settings.WeaponToStatusBarSpacingNoAmmo,
        WeaponAmmoOffsetX);

    if (ammoDisplayed)
    {
        // When Weapon is inserted above an existing stack,
        // keep Ammo anchored directly beneath its icon
        // while elements already below move downward.
        const float weaponReservation =
            settings.Weapon
            ? mLayout.GetStackProgress(HUDComponentType::Weapon)
            : 1.0f;

        const float ammoPositionCorrection =
            WeaponToAmmoOffset *
            (1.0f - weaponReservation);

        mLayout.Add(
            mAmmo,
            AmmoHeight,
            WeaponAmmoOffsetY + ammoPositionCorrection,
            settings.AmmoSpacing,
            WeaponAmmoOffsetX);
    }

    if (armourDisplayed)
    {
        mLayout.Add(
            mArmour,
            statusBarHeight,
            0.0f,
            settings.ArmourSpacing);
    }

    mLayout.Add(
        mBreath,
        statusBarHeight,
        0.0f,
        settings.BreathSpacing);

    mLayout.Add(
        mHealth,
        statusBarHeight,
        0.0f,
        settings.HealthSpacing);

    // TODO:
    // Status bars currently use vertical stacking.
    // Future "Compact Status Bars" mode will arrange
    // Armour, Breath and Health horizontally.
}

void HUDManager::HideAll()
{
    mClock.Reset();
    mHealth.Reset();
    mArmour.Reset();
    mBreath.Reset();
    mMoney.Reset();
    mWeaponIcon.Reset();
    mAmmo.Reset();
    mWanted.Reset();
}

HUDManager& HUDManager::Get()
{
    static HUDManager instance;
    return instance;
}