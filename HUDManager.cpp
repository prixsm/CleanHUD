#include <common.h>
#include <CCamera.h>
#include <CClock.h>
#include <CCutsceneMgr.h>
#include <CDarkel.h>
#include <CPad.h>
#include <CPedIntelligence.h>
#include <CPlayerData.h>
#include <CPlayerInfo.h>
#include <CPlayerPed.h>
#include <CStats.h>
#include <CTaskSimpleSwim.h>
#include <CTimer.h>
#include <CWanted.h>
#include <CWeaponInfo.h>
#include <CWorld.h>

#include "HUDManager.h"
#include "HUDMetrics.h"
#include "HUDRenderer.h"

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
        {})
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

    bool IsUnderwaterSwimState(eSwimState state)
    {
        return state == SWIM_DIVE_UNDERWATER ||
            state == SWIM_UNDERWATER_SPRINTING;
    }

    bool IsSurfaceSwimState(eSwimState state)
    {
        return state == SWIM_TREAD ||
            state == SWIM_SPRINT ||
            state == SWIM_SPRINTING ||
            state == SWIM_BACK_TO_SURFACE;
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

    return mPlayer->m_pPlayerData->m_pWanted &&
        mPlayer->m_pPlayerData->m_pWanted->m_nWantedLevel > 0;
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

void HUDManager::Initialize()
{
    if (m_initialized)
        return;

    m_initialized = true;
}

void HUDManager::OnGameLoaded()
{
    mSessionInitialized = false;
    mVanillaLayoutLatched = false;

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
        return;
    }

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

    Initialize();

    CPlayerData* playerData = mPlayer->m_pPlayerData;

    if (tabHeld)
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

    mClock.Update();

    if (tabHeld)
    {
        mHealth.ForceVisible();
        mArmour.ForceVisible();
        mBreath.ForceVisible();
        mWeaponIcon.ForceVisible();
        mAmmo.ForceVisible();
    }

    BuildPlayerHUDLayout();
    HUDRenderer::Get().Build(mLayout);

    if (!tabHeld)
    {
        mVanillaLayoutLatched = false;
    }
}

void HUDManager::ResetSessionState()
{
    HideAll();

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

    /*
     * While the initial game/cutscene/fade sequence is
     * still in progress, health values are setup state,
     * not gameplay changes.
     */
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

    /*
     * Once actual gameplay is available, require health
     * to remain stable before arming change detection.
     */
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

    mHealth.Update();

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

    mArmour.Update();

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

    mMoney.Update();

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

    mWeaponIcon.Update();

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

    mAmmo.Update();

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

    if (HasHealthChanged(currentHealth))
    {
        mHealth.ShowFor(
            mVisibleTime);
    }

    mHealth.Update();
}

void HUDManager::UpdateArmour()
{
    float currentArmour = mPlayer->m_fArmour;

    if (!UpdateArmourInitialization(currentArmour))
        return;

    if (HasArmourChanged(currentArmour))
        mArmour.ShowFor(mVisibleTime);

    mArmour.Update();
}

void HUDManager::UpdateBreath()
{
    if (!mPlayer ||
        !mPlayer->m_pPlayerData)
    {
        mBreath.Hide();
        mBreath.Update();
        return;
    }

    auto* data =
        mPlayer->m_pPlayerData;

    const float maxBreath =
        CStats::GetFatAndMuscleModifier(
            STAT_MOD_AIR_IN_LUNG);

    const bool breathDepleted =
        data->m_fBreath < maxBreath;

    bool underwater =
        false;

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

    if (underwater ||
        breathDepleted)
    {
        mBreath.Show();
    }
    else
    {
        mBreath.Hide();
    }

    mBreath.Update();
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

    mMoney.Update();
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

    mWeaponIcon.Update();
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

    mAmmo.Update();
}

void HUDManager::UpdateWanted()
{
    if (HasWantedLevel())
        mWanted.Show();
    else
        mWanted.Hide();

    mWanted.Update();
}

void HUDManager::BuildPlayerHUDLayout()
{
    constexpr float ClockHeight = 18.0f;
    constexpr float MoneyHeight = 18.0f;
    constexpr float WantedHeight = 18.0f;
    constexpr float AmmoHeight = 16.0f;

    constexpr float MoneyToWantedSpacing = 40.0f;
    constexpr float MoneyToWeaponSpacing = 35.0f;
    constexpr float MoneyToStatusBarSpacing = 28.0f;

    constexpr float WantedToWeaponSpacing = 30.0f;
    constexpr float WantedToStatusBarSpacing = 20.0f;

    constexpr float ClockSpacing = 40.0f;
    constexpr float WeaponSpacing = 4.0f;
    constexpr float AmmoSpacing = 4.0f;
    constexpr float ArmourSpacing = 4.0f;
    constexpr float BreathSpacing = 4.0f;
    constexpr float HealthSpacing = 4.0f;

    constexpr float WeaponAmmoOffsetX = 11.0f;
    constexpr float WeaponAmmoOffsetY = -12.0f;
    constexpr float WeaponToAmmoOffset = 83.0f;

    const float statusBarHeight =
        HUDMetrics::GetStatusBarSpacing();

    const bool ammoDisplayed =
        ShouldDisplayAmmo();

    const bool weaponGroupVisible =
        mWeaponIcon.IsVisible() ||
        (mAmmo.IsVisible() && ammoDisplayed);

    const bool statusBarVisible =
        mArmour.IsVisible() ||
        mBreath.IsVisible() ||
        mHealth.IsVisible();

    float moneySpacing = 0.0f;

    if (mWanted.IsVisible())
    {
        moneySpacing =
            MoneyToWantedSpacing;
    }
    else if (weaponGroupVisible)
    {
        moneySpacing =
            MoneyToWeaponSpacing;
    }
    else if (statusBarVisible)
    {
        moneySpacing =
            MoneyToStatusBarSpacing;
    }

    float wantedSpacing = 0.0f;

    if (weaponGroupVisible)
    {
        wantedSpacing =
            WantedToWeaponSpacing;
    }
    else if (statusBarVisible)
    {
        wantedSpacing =
            WantedToStatusBarSpacing;
    }

    mLayout.Add(
        mClock,
        ClockHeight,
        0.0f,
        ClockSpacing);

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
        WeaponToAmmoOffset - WeaponSpacing,
        WeaponAmmoOffsetY,
        WeaponSpacing,
        WeaponAmmoOffsetX);

    if (ammoDisplayed)
    {
        mLayout.Add(
            mAmmo,
            AmmoHeight,
            WeaponAmmoOffsetY,
            AmmoSpacing,
            WeaponAmmoOffsetX);
    }

    mLayout.Add(
        mArmour,
        statusBarHeight,
        0.0f,
        ArmourSpacing);

    mLayout.Add(
        mBreath,
        statusBarHeight,
        0.0f,
        BreathSpacing);

    mLayout.Add(
        mHealth,
        statusBarHeight,
        0.0f,
        HealthSpacing);

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
}

HUDManager& HUDManager::Get()
{
    static HUDManager instance;
    return instance;
}